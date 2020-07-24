#include "retexture.h"

#define VERSION "1.00"

/*
	Read an OBJ file along with texture materials
	Rearrange texture images and uv coordinates to make more efficient packing
	Stages
		- Read the OBJ file, most important thing is UV coordinates per face
		- Read the material data and load texture files
		- Mask out the parts of the texture not used
		- Create a polygon around the islands of texture pixels actually used
		- Pack the textures into new texture images, reuse the rest of the material information
		- Adjust UV coordinates dependent on the island polygon they reside in
		- Write new OBJ file, same as input except for modified UV values and new material names
*/

// Command line parameters
PARAMS params;

// All the vertices, global bounding box
XYZ *vertices = NULL;
long nvertices = 0;
XYZ themin = {1e32,1e32,1e32},themax = {-1e32,-1e32,-1e32},themid;

// All the UV coordinates
UV *uv = NULL;
long nuv = 0;
XY alluvmin = {1e32,1e32}, alluvmax = {-1e32,-1e32};

// All the faces
FACE *faces = NULL;
long nfaces = 0;

// Materials
char materialfile[256] = "missing.mtl";
MATERIAL *materials = NULL;
int nmaterials = 0;

// Island
ISLAND *islands = NULL;
int nislands = 0;

int main(int argc,char **argv)
{
   int i;
   char objname[256];
   FILE *fptr;
	double starttime,stoptime;

	Init();

   // Deal with command line
   if (argc < 2) 
      GiveUsage(argv[0]);
   for (i=1;i<argc;i++) {
      if (strcmp(argv[i],"-v") == 0)
         params.verbose = TRUE;
   }
   strcpy(objname,argv[argc-1]);

   // Parse the OBJ file
	starttime = GetRunTime();
   if ((fptr = fopen(objname,"r")) == NULL) {
      fprintf(stderr,"Failed to open input OBJ file \"%s\"\n",objname);
      exit(-1);
   }
   if (!ReadObj(fptr)) {
      fclose(fptr);
      exit(-1);
   }
   fclose(fptr);
	stoptime = GetRunTime();
	if (params.verbose)
		fprintf(stderr,"Time to read OBJ file: %.1lf seconds\n",stoptime-starttime);

	// Read texture files
	ReadTextures();

	// Create a mask of used portions of the texture image
	MaskTextures();
	if (params.verbose)
		SaveMasks();

	// Create a polygon around each island of used texture
	FindIslands();
	if (params.verbose)
		SaveIslands();

	// Remask, destroyed in FindIslands()
	ReadTextures();
	MaskTextures();

	// Sort by decreasing area
	qsort((void *)islands,nislands,sizeof(ISLAND),islandcompare);
	if (params.verbose) 
		for (i=0;i<nislands;i++)
			fprintf(stderr,"   Island %d has area %d and bounds (%d,%d) to (%d,%d)\n",
				i,islands[i].area,islands[i].themin.h,islands[i].themin.v,islands[i].themax.h,islands[i].themax.v);

	// Pack the textures into a new texture
	PackIslands();

	// Update UVs after packing

   exit(0);
}

/*
	Compare function for qsort
*/
int islandcompare(const void *p1,const void *p2)
{
	ISLAND *island1,*island2;

	island1 = (ISLAND *)p1;
   island2 = (ISLAND *)p2;

	if (island1->area > island2->area)
		return(-1);
	else if (island1->area < island2->area)
		return(1);
	else
		return(0);
}

void GiveUsage(char *s)
{
   fprintf(stderr,"Usage: %s [options] objfilename\n",s);
	fprintf(stderr,"Version: %s\n",VERSION);
   fprintf(stderr,"Options\n");
   fprintf(stderr,"   -v     verbose mode, default: disabled\n");

   exit(0);
}

/*
   Read all vertices and uv coordinates from OBJ file
   Currently ignore vertex colours and normals
	Primarily we need UV coordinates and materials but read faces so we know
	which UV triples are used by triangles. We read vertices because we will
	eventually need to save a new OBJ file.
	Not sure if UV coordinates outside the 0 to 1 range are supposed to wrap?
*/
int ReadObj(FILE *fptr)
{
   long linecount = 0;
   long ram;
   char aline[1024],ignore[256];
   UV t;
   XYZ p;
	FACE aface;

   if (params.verbose)
      fprintf(stderr,"Parsing OBJ file\n");

   // Read a line at a time
	// Only interested in vertex and texture coordinate lines
   while (fgets(aline,1023,fptr) != NULL) {
      linecount++;
      if (params.verbose)
			LineStatus(linecount);

      // Vertex line
      if (aline[0] == 'v' && aline[1] == ' ') {
         if (sscanf(aline+2,"%f %f %f",&p.x,&p.y,&p.z) != 3) {
            fprintf(stderr,"Error reading vertex line %ld\n",linecount);
            fprintf(stderr,"Line: \"%s\"\n",aline);
            break;
         }
         if ((vertices = realloc(vertices,(nvertices+1)*sizeof(XYZ))) == NULL) {
				fprintf(stderr,"Ran out of memory allocating memory for vertices\n");
				return(FALSE);
			}
         vertices[nvertices] = p;
			vertices[nvertices].flag = -1;
         nvertices++;
         MinMaxXYZ(p,&themin,&themax);
         continue;
      }

      // Texture coordinate line
      if (aline[0] == 'v' && aline[1] == 't' && aline[2] == ' ') {
         if (sscanf(aline+3,"%f %f",&t.u,&t.v) != 2) {
            fprintf(stderr,"Error reading uv coordinate line %ld\n",linecount);
            fprintf(stderr,"Line: \"%s\"\n",aline);
            break;
         }
			while (t.u < 0) t.u += 1;
			while (t.v < 0) t.v += 1;
			while (t.u > 1) t.u -= 1;
			while (t.v > 1) t.v -= 1;
			if (t.u == 1) t.u -= EPS;
         if (t.v == 1) t.v -= EPS;
         if ((uv = realloc(uv,(nuv+1)*sizeof(UV))) == NULL) {
				fprintf(stderr,"Ran out of memory allocating memory for UV coordinates\n");
				return(FALSE);
			}
         uv[nuv] = t;
			uv[nuv].flag = -1;
         nuv++;
			if (t.u < alluvmin.x) alluvmin.x = t.u;
         if (t.u > alluvmax.x) alluvmax.x = t.u;
         if (t.v < alluvmin.y) alluvmin.y = t.v;
         if (t.v > alluvmax.y) alluvmax.y = t.v;
         continue;
      }

      // Normals, ignore
      if (aline[0] == 'v' && aline[1] == 'n' && aline[2] == ' ') {
         continue;
      }

      // Count the faces, optionally load into memory
      if (aline[0] == 'f' && aline[1] == ' ') {
         if (!ParseFace(aline,&aface)) {
            fprintf(stderr,"Face parsing error occured on line %ld of tmp file\n",linecount);
            return(FALSE);
         }
			if ((faces = realloc(faces,(nfaces+1)*sizeof(FACE))) == NULL) {
				fprintf(stderr,"Ran out of memory allocating memory for faces\n");
				return(FALSE);
			}
			CopyFace(&(aface),&(faces[nfaces]));
			faces[nfaces].materialid = nmaterials-1;
			nfaces++;
			continue;
		}

     	// Materials
     	if (strstr(aline,"mtllib ") != NULL) {
     	   CleanString(aline);
     	   sscanf(aline,"%s %s",ignore,materialfile);
			if (params.verbose)
				fprintf(stderr,"Materials file is \"%s\"\n",materialfile);
     	   continue;
     	}
     	if (strstr(aline,"usemtl ") != NULL) {
     	   CleanString(aline);
			materials = realloc(materials,(nmaterials+1)*sizeof(MATERIAL));
			sscanf(aline,"%s %s",ignore,materials[nmaterials].name);
			if (params.verbose)
				fprintf(stderr,"Found material \"%s\"\n",materials[nmaterials].name);
			strcpy(materials[nmaterials].texture,"");
			if (!ReadMaterial(materialfile,materials[nmaterials].name,nmaterials)) 
				fprintf(stderr,"Failed to find texture file for material \"%s\"\n",materials[nmaterials].name);
			nmaterials++;
     	   continue;
     	}
   }

	// Was it a useful OBJ file?
	if (nvertices <= 0) {
		fprintf(stderr,"Failed to find any vertices\n");
		return(FALSE);
	}

   if (params.verbose) {
      fprintf(stderr,"Found %ld vertices\n",nvertices);
      fprintf(stderr,"Found %ld uv coordinates\n",nuv);
      fprintf(stderr,"Found %ld faces\n",nfaces);
      fprintf(stderr,"Storage per vertex is %ld bytes\n",sizeof(XYZ)+sizeof(UV));
      ram = (nvertices*sizeof(XYZ)+nuv*sizeof(UV))/1024/1024;
      if (ram < 1024)
         fprintf(stderr,"Require %ld MB of memory for vertices and uv\n",ram);
      else
         fprintf(stderr,"Require %ld GB of memory for vertices and uv\n",ram/1024);
		fprintf(stderr,"Storage per face is %ld bytes\n",sizeof(FACE));
		ram = nfaces*sizeof(FACE)/1024/1024;
	   if (ram < 1024)
	      fprintf(stderr,"Require %ld MB of memory for faces\n",ram);
	   else
	      fprintf(stderr,"Require %ld GB of memory for faces\n",ram/1024);
		fprintf(stderr,"Found %d materials\n",nmaterials);
      fprintf(stderr,"Bounding box\n");
      fprintf(stderr,"   %g <= x <= %g\n",themin.x,themax.x);
      fprintf(stderr,"   %g <= y <= %g\n",themin.y,themax.y);
      fprintf(stderr,"   %g <= z <= %g\n",themin.z,themax.z);
		fprintf(stderr,"UV range\n");
		fprintf(stderr,"   %g <= u <= %g\n",alluvmin.x,alluvmax.x);
      fprintf(stderr,"   %g <= v <= %g\n",alluvmin.y,alluvmax.y);
   }

   return(TRUE);
}

/*
	Read the texture images referenced by the materials
	Only support jpeg at this stage
	If a material doesn't use a texture, make one up, for consistentcy later on.
*/
int ReadTextures(void)
{
	int i,j;
	FILE *fptr;
	int width,height,depth;
	BITMAP4 missing = {255,128,128,0};

	if (params.verbose) {
		for (i=0;i<nmaterials;i++)
			fprintf(stderr,"Material %d is texture \"%s\"\n",i,materials[i].texture);
	}

	for (i=0;i<nmaterials;i++) {
		if (params.verbose)
			fprintf(stderr,"Reading texture \"%s\"\n",materials[i].texture);

		// Create a dummy texture image if not supplied
		if (strlen(materials[i].texture) < 2) {
         materials[i].width = 512;
         materials[i].height = 512;
         materials[i].image = Create_Bitmap(materials[i].width,materials[i].height);
         for (j=0;j<materials[i].width*materials[i].height;j++)
            materials[i].image[j] = missing;
			continue;
		}	

		// Attempt to open the image file
		if ((fptr = fopen(materials[i].texture,"r")) == NULL) {
			fprintf(stderr,"Failed to open image \"%s\" for material %d\n",materials[i].texture,i);
			return(FALSE);
		}
		if (!JPEG_Info(fptr,&width,&height,&depth)) {
         fprintf(stderr,"Failed to read image \"%s\" for material %d\n",materials[i].texture,i);
         return(FALSE);
      }
		materials[i].width = width;
		materials[i].height = height;
		materials[i].image = Create_Bitmap(materials[i].width,materials[i].height);
		if (JPEG_Read(fptr,materials[i].image,&width,&height) != 0) {
			fprintf(stderr,"Failed to read image \"%s\" for material %d (%dx%d)\n",
				materials[i].texture,i,width,height);
			return(FALSE);
		}
		fclose(fptr);
	}

	return(FALSE);
}

/*
	Parse the MTL file to extract the name of the texture image for each material
	Lots could go wrong here ... assume just one "map_Kd" reference per material
*/
int ReadMaterial(char *fname,char *material,int materialid)
{
	FILE *fptr;
	char aline[1024],ignore[256];

	if ((fptr = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"Failed to open materials file \"%s\"\n",fname);
		return(FALSE);
	}

	while (fgets(aline,1023,fptr) != NULL) {
		if (strstr(aline,"newmtl ") != NULL) {
			if (strstr(aline,material) != NULL) {
				while (fgets(aline,1023,fptr) != NULL) {
					if (strstr(aline,"newmtl ") != NULL) 
						break;
					if (strstr(aline,"map_Kd ") != NULL) {
						sscanf(aline,"%s %s",ignore,materials[nmaterials].texture);
						if (params.verbose)
							fprintf(stderr,"Found texture \"%s\" for material \"%s\"\n",
								materials[nmaterials].texture,material);
						break;
					}
				}
				break;
			}
		}
	}
	if (strlen(materials[nmaterials].texture) < 2)
		fprintf(stderr,"No texture image associated with material \"%s\"\n",material);
	fclose(fptr);

	return(TRUE);
}

/*
   Extract the vertex and uv indices from a face line
   Only handles 3 vertex faces
   Deals with vertex / texture / normal triples or just vertex / texture pairs
   Fails if there are no texture coordinates
	Note: OBJ file indices start from 1, not 0
*/
int ParseFace(char *s,FACE *f)
{
   int i,m,a[9];

   for (i=0;i<strlen(s);i++)
      if (s[i] == '/')
         s[i] = ' ';
   m = sscanf(s+2,"%d %d %d %d %d %d %d %d %d\n",
       &a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6],&a[7],&a[8]);
   if (m == 3) {
      fprintf(stderr,"Encountered a face with no texture coordinates\n");
      return(FALSE);
   } else if (m == 6) {
      f->xyzid[0] = a[0]-1;
      f->xyzid[1] = a[2]-1;
      f->xyzid[2] = a[4]-1;
      f->uvid[0] = a[1]-1;
      f->uvid[1] = a[3]-1;
      f->uvid[2] = a[5]-1;
   } else if (m == 9) {
      f->xyzid[0] = a[0]-1;
      f->xyzid[1] = a[3]-1;
      f->xyzid[2] = a[6]-1;
      f->uvid[0] = a[1]-1;
      f->uvid[1] = a[4]-1;
      f->uvid[2] = a[7]-1;
   } else {
      fprintf(stderr,"Encountered an unexpected formatted face\n");
      return(FALSE);
   }

   return(TRUE);
}

/*
	Copy the contents of a face from one face to another
*/
void CopyFace(FACE *source,FACE *destination)
{
	int i;

   for (i=0;i<3;i++) {
      destination->xyzid[i] = source->xyzid[i];
      destination->uvid[i] = source->uvid[i];
   }
	destination->materialid = source->materialid;
}

/*
	Default value of the parameters
*/
void Init(void)
{
	params.verbose = FALSE;
}

/*
   Line progress reports, designed to give feed back but not too
	much for large files.
*/
void LineStatus(long n)
{
   if (n < 1000000) {
      if ((n % 100000) == 0)
         fprintf(stderr,"   Processing line %ld\n",n);
   } else if (n < 10000000) {
      if ((n % 1000000) == 0)
         fprintf(stderr,"   Processing line %ld\n",n);
   } else {
      if ((n % 10000000) == 0)
         fprintf(stderr,"   Processing line %ld\n",n);
   }
}

/*
	Illustrate the parts of the textures actually used
	If the alpha channel is 255 then it is not used in a texture, 0 if used
	Save modified textures, initially just for debugging.
	At the end of this, any part of the texture unused is black
	Also alpha channel will be 0 for used portions, 255 for unused portions
*/
void MaskTextures(void)
{
	int i,j,n,matid;
	UV t[3];
	UV uvmin,uvmax;
	int index;
	BITMAP4 black = {0,0,0,255};

	// Set alpha channel to 255 on all textures
	for (n=0;n<nmaterials;n++) {
		for (i=0;i<materials[n].width*materials[n].height;i++)
			materials[n].image[i].a = 255;
	}

	// Mark out used portions and set alpha to 0
	// This is done by considering the uv triangle for each triangular face
	for (n=0;n<nfaces;n++) {
		matid = faces[n].materialid;

		// The triangle in UV pixel space
		// Assume out of range is meant to wrap, see wrapping on UV reading code
		for (i=0;i<3;i++) {
			t[i].u = materials[matid].width * uv[faces[n].uvid[i]].u;
			t[i].v = materials[matid].height * uv[faces[n].uvid[i]].v;
		}

		// Determine bounding box of uv triangle
		uvmin = t[0];
		uvmax = t[0];
		for (i=1;i<3;i++) {
			uvmin.u = MIN(uvmin.u,t[i].u);
         uvmin.v = MIN(uvmin.v,t[i].v);
         uvmax.u = MAX(uvmax.u,t[i].u);
         uvmax.v = MAX(uvmax.v,t[i].v);
		}

		// Extend it by one pixel
		if (uvmin.u > 0) uvmin.u -= 1;
      if (uvmin.v > 0) uvmin.v -= 1;
      if (uvmin.u < materials[matid].width-1) uvmin.u += 1;
      if (uvmin.v < materials[matid].height-1) uvmin.v += 1;

		// Mask the rectangle that is used as a texture, set alpha to 0
		for (i=uvmin.u;i<uvmax.u;i++) {
			for (j=uvmin.v;j<uvmax.v;j++) {
				index = j * materials[matid].width + i;
				materials[matid].image[index].a = 0;
			}
		}
	}

   // Erase all unused portions
   for (n=0;n<nmaterials;n++) {
      for (i=0;i<materials[n].width*materials[n].height;i++) {
         if (materials[n].image[i].a > 0)
				materials[n].image[i] = black;
		}
   }
}

/*
	Collect all the points in an island
	Reset each point to alpha = 255
*/
void FindIslands(void)
{
	int i,j,n,index,npoly,indexL,indexR;
	POINT seed,p,currp,dir,dir1,dir2,left,right;
	POINT large = {32000,32000},small = {-32000,-32000},zero = {0,0};
   POINT *plist = NULL;
   int nplist = 0;
	int merged = 0;

	// Create islands for each material
	if (params.verbose)
		fprintf(stderr,"Creating islands\n");
	for (n=0;n<nmaterials;n++) {
		nplist = 0;

   	// Require boundaries to be masked off, not part of islands
   	for (i=0;i<materials[n].width;i++) {
   	   materials[n].image[i].a = 255;
   	   materials[n].image[(materials[n].height-1)*materials[n].width+i].a = 255;
   	}
   	for (i=0;i<materials[n].height;i++) {
   	   materials[n].image[i*materials[n].width].a = 255;
   	   materials[n].image[i*materials[n].width+materials[n].width-1].a = 255;
   	}

		// Consider every pixel in the texture
		for (seed.v=1;seed.v<materials[n].height-1;seed.v++) {
			for (seed.h=1;seed.h<materials[n].width-1;seed.h++) {

				// Nothing to do to masked pixels
				index = seed.v * materials[n].width + seed.h;
				if (materials[n].image[index].a > 0)
					continue;

				// This point is on the left edge of an island pixel
				// because we are scanning bottom to top, left to right
				dir.h = 0;
				dir.v = 1;
				currp = seed;

				// Create island starting with this seed point
            islands = realloc(islands,(nislands+1)*sizeof(ISLAND));
            islands[nislands].matid = n;
				islands[nislands].poly = malloc(2*sizeof(POINT));
				islands[nislands].poly[0] = seed;
            islands[nislands].poly[1].h = seed.h + dir.h;
            islands[nislands].poly[1].v = seed.v + dir.v;
				currp = islands[nislands].poly[1];
				islands[nislands].npoly = 2;
				islands[nislands].area = 0;
				islands[nislands].themin = large;
            islands[nislands].themax = small;
				islands[nislands].offset = zero;

				npoly = islands[nislands].npoly;
				currp = islands[nislands].poly[npoly-1];
				dir.h = currp.h - islands[nislands].poly[npoly-2].h;
				dir.v = currp.v - islands[nislands].poly[npoly-2].v;

				// Trace around the island
				for (;;) {

					// Determine the index of the left and right pixel
					if (dir.h == 0 && dir.v == 1) {
						left.h = currp.h - 1;
						left.v = currp.v;
						right = currp;
					} else if (dir.h == 0 && dir.v == -1) {
                  left.h = currp.h;
                  left.v = currp.v - 1;
						right.h = currp.h - 1;
						right.v = currp.v - 1;
					} else if (dir.h == 1 && dir.v == 0) {
                  left = currp;
                  right.h = currp.h;
						right.v = currp.v - 1;
					} else if (dir.h == -1 && dir.v == 0) {
                  left.h = currp.h - 1;
                  left.v = currp.v - 1;
						right.h = currp.h - 1;
						right.v = currp.v;
					}
					indexL = left.v * materials[n].width + left.h;
					indexR = right.v * materials[n].width + right.h;
					npoly = islands[nislands].npoly;
					islands[nislands].poly = realloc(islands[nislands].poly,(npoly+1)*sizeof(POINT));

					// Case of left cell on outside and right cell on inside, move forward
					if (materials[n].image[indexL].a > 0 && materials[n].image[indexR].a == 0) {
						islands[nislands].poly[npoly].h = currp.h + dir.h;
						islands[nislands].poly[npoly].v = currp.v + dir.v;
					}

					// Case of left and right on outside, move to the right, rotate by 90 clockwise
					else if (materials[n].image[indexL].a > 0 && materials[n].image[indexR].a > 0) {
                  islands[nislands].poly[npoly].h = currp.h + dir.v;
                  islands[nislands].poly[npoly].v = currp.v - dir.h;
					}

               // Case of left and right on inside, move to the left, rotate by -90 clockwise
               else if (materials[n].image[indexL].a == 0 && materials[n].image[indexR].a == 0) {
                  islands[nislands].poly[npoly].h = currp.h - dir.v;
                  islands[nislands].poly[npoly].v = currp.v + dir.h;
               }

               // Case of left on inside and right on outside, move to the left, rotate by -90 clockwise
					// Ambiguous case
               else if (materials[n].image[indexL].a == 0 && materials[n].image[indexR].a > 0) {
                  islands[nislands].poly[npoly].h = currp.h - dir.v;
                  islands[nislands].poly[npoly].v = currp.v + dir.h;
               }

					else {
						fprintf(stderr,"Should not get here, something bad and/or strange has occured!\n");
					}
				
					currp = islands[nislands].poly[npoly];
               islands[nislands].npoly++;
            	dir.h = currp.h - islands[nislands].poly[npoly-1].h;
            	dir.v = currp.v - islands[nislands].poly[npoly-1].v;
					
					if (currp.h == islands[nislands].poly[0].h && currp.v == islands[nislands].poly[0].v)
						break;
				}

				// Clear all points within the island
            plist = realloc(plist,(nplist+1)*sizeof(POINT));
            plist[nplist] = seed;
            nplist++;
            while (nplist > 0) {
               index = plist[nplist-1].v * materials[n].width + plist[nplist-1].h;
               if (materials[n].image[index].a == 0) {
                  p = plist[nplist-1];
                  if (p.h > 0 && p.h < materials[n].width-1) {
                     plist = realloc(plist,(nplist+2)*sizeof(POINT));
                     plist[nplist-1].h = p.h - 1;
                     plist[nplist-1].v = p.v;
                     plist[nplist].h = p.h + 1;
                     plist[nplist].v = p.v;
                     nplist += 2;
                  }
                  if (p.v > 0 && p.v < materials[n].height-1) {
                     plist = realloc(plist,(nplist+2)*sizeof(POINT));
                     plist[nplist-1].h = p.h;
                     plist[nplist-1].v = p.v - 1;
                     plist[nplist].h = p.h;
                     plist[nplist].v = p.v + 1;
                     nplist += 2;
                  }
               	materials[n].image[index].a = 255;
						islands[nislands].area++;
					}
               nplist--;
            }

            if (params.verbose)
               fprintf(stderr,"   Creating new island %d with %d vertices and area of %d\n",
						nislands,islands[nislands].npoly,islands[nislands].area);
				nislands++;
			}
		}
	} // nth material

	free(plist);

	// Update the island bounding boxes
	for (i=0;i<nislands;i++) {
		for (j=0;j<islands[i].npoly;j++) {
      	islands[i].themin.h = MIN(islands[i].themin.h,islands[i].poly[j].h);
      	islands[i].themax.h = MAX(islands[i].themax.h,islands[i].poly[j].h);
      	islands[i].themin.v = MIN(islands[i].themin.v,islands[i].poly[j].v);
      	islands[i].themax.v = MAX(islands[i].themax.v,islands[i].poly[j].v);
		}
	}

	if (params.verbose) 
		fprintf(stderr,"Created %d islands\n",nislands);

	// Combined vectors where possible for efficiency
	for (n=0;n<nislands;n++) {

		// Smooth out single pixel corners
      for (i=2;i<islands[n].npoly;i++) {
         dir1.h = islands[n].poly[i].h - islands[n].poly[i-1].h;
         dir1.v = islands[n].poly[i].v - islands[n].poly[i-1].v;
         dir2.h = islands[n].poly[i-1].h - islands[n].poly[i-2].h;
         dir2.v = islands[n].poly[i-1].v - islands[n].poly[i-2].v;
         if (dir1.h == 1 && dir1.v == 0 && dir2.h == 0 && dir2.v == 1) {
            for (j=i-1;j<islands[n].npoly-1;j++)
               islands[n].poly[j] = islands[n].poly[j+1];
            islands[n].npoly--;
            merged++;
            i--;
				continue;
         }
         if (dir1.h == -1 && dir1.v == 0 && dir2.h == 0 && dir2.v == -1) {
            for (j=i-1;j<islands[n].npoly-1;j++)
               islands[n].poly[j] = islands[n].poly[j+1];
            islands[n].npoly--;
            merged++;
            i--;
				continue;
         }
         if (dir1.h == 0 && dir1.v == 1 && dir2.h == -1 && dir2.v == 0) {
            for (j=i-1;j<islands[n].npoly-1;j++)
               islands[n].poly[j] = islands[n].poly[j+1];
            islands[n].npoly--;
            merged++;
            i--;
            continue;
         }
         if (dir1.h == 0 && dir1.v == -1 && dir2.h == 1 && dir2.v == 0) {
            for (j=i-1;j<islands[n].npoly-1;j++)
               islands[n].poly[j] = islands[n].poly[j+1];
            islands[n].npoly--;
            merged++;
            i--;
            continue;
         }
      }
		
		// Combine straight lines
		for (i=2;i<islands[n].npoly;i++) {
			dir1 = CalcDir(islands[n].poly[i-1],islands[n].poly[i]);
			dir2 = CalcDir(islands[n].poly[i-2],islands[n].poly[i-1]);
			if (dir1.h == dir2.h && dir1.v == dir2.v) {
				for (j=i-1;j<islands[n].npoly-1;j++) 
					islands[n].poly[j] = islands[n].poly[j+1];
				islands[n].npoly--;
				merged++;
				i--;
			}
		}
	}
	if (params.verbose)
		fprintf(stderr,"Merged %d vectors\n",merged);
}

/*
	Pack the textures
	Do so into a 8K x 8K texture
	Islands are packed from the largest to the smallest
*/
void PackIslands(void)
{
	int n,nmat,newh,newv,index1,index2;
	BITMAP4 *newtex = NULL,black = {0,0,0,255}; // Note alpha is 255
	int width = 8192,height = 8192;
	POINT p;
	FILE *fptr = NULL;

	// New texture, cleared to black
	newtex = Create_Bitmap(width,height);
	Erase_Bitmap(newtex,width,height,black);

	for (n=0;n<nislands;n++) {
		nmat = islands[n].matid;

		// Find the best position
		// Update offset
		// Copy pixels
		if (!FindBestPosition(islands[n],newtex,width,height,&p)) {
			fprintf(stderr,"Failed to find a location to place island %d\n",n);
			continue;
		}
		islands[n].offset.h = islands[n].themin.h - p.h;
      islands[n].offset.v = islands[n].themin.v - p.v;

      if (params.verbose)
         fprintf(stderr,"Packing island %d, at (%d,%d)\n",n,p.h,p.v);

		// Copy island into destination texture
		for (p.h=islands[n].themin.h;p.h<islands[n].themax.h;p.h++) {
			for (p.v=islands[n].themin.v;p.v<islands[n].themax.v;p.v++) {

            // Set clear flag if not inside polygon, slow
            if (!InsidePolygon(islands[n].poly,islands[n].npoly,p)) {
               newtex[index2] = black;
					continue;
            }

				// Calculate new position
				newh = p.h - islands[n].offset.h;
				newv = p.v - islands[n].offset.v;
				if (newh < 0 || newv < 0 || newh >= width || newv >= height) {
					fprintf(stderr,"Out of range pixel during packing pixel copy (%d,%d)\n",newh,newv);
					continue;
				}

				// Copy pixel
				index1 = p.v * materials[nmat].width + p.h;
				index2 = newv * width + newh;
				newtex[index2] = materials[nmat].image[index1];
				newtex[index2].a = 0;
			}
		}

	}

	// Save the new texture
	fptr = fopen("newtexture.tga","w");
	Write_Bitmap(fptr,newtex,width,height,13);
	fclose(fptr);

	if (params.verbose) 
		SaveIslands2(newtex,width,height);
}

/*
	Find the best poisition in the image for an island
	To start just test 4 corners
*/
int FindBestPosition(ISLAND island,BITMAP4 *image,int width,int height,POINT *pbest)
{
	int dh,dv,n;
	POINT p;
	
	dh = island.themax.h - island.themin.h;
	dv = island.themax.v - island.themin.v;

	for (n=0;n<width;n++) {
		p.h = n;
		for (p.v=0;p.v<=n;p.v++) {
			if (!TestRectAlpha(p,dh,dv,image,width,height))
				continue;
			*pbest = p;
			return(TRUE);
		}
		p.v = n;
		for (p.h=0;p.h<n;p.h++) {
         if (!TestRectAlpha(p,dh,dv,image,width,height))
            continue;
         *pbest = p;
         return(TRUE);
      }
	}

	/* Initial case of bottom to top, left to right mode
   for (p.v=0;p.v<height;p.v++) { 
      for (p.h=0;p.h<width;p.h++) {
            
         if (!TestRectAlpha(p,dh,dv,image,width,height))
            continue;
            
         *pbest = p;
         return(TRUE);
      }
   }
	*/

	// After all that, didn't find a free location
	return(FALSE);
}

/*
	Return true if the rectangle is in empty space
	Rectangle has corners (p.h,p.v) and (p.h+dh,p.v+dv)
*/
int TestRectAlpha(POINT p,int dh,int dv,BITMAP4 *image,int width,int height)
{
	POINT q;

   // Test corners first
   q.h = p.h;
   q.v = p.v;
   if (AlphaZero(q,image,width,height))
      return(FALSE);
   q.h = p.h + dh;
   q.v = p.v;
   if (AlphaZero(q,image,width,height))
      return(FALSE);
   q.h = p.h + dh;
   q.v = p.v + dv;
   if (AlphaZero(q,image,width,height))
      return(FALSE);
   q.h = p.h;
   q.v = p.v + dv;
   if (AlphaZero(q,image,width,height))
      return(FALSE);

	// Test along all edges
   q.v = p.v;
   for (q.h=p.h;q.h<p.h+dh;q.h++) 
      if (AlphaZero(q,image,width,height)) 
         return(FALSE);

   q.v = p.v + dv;
   for (q.h=p.h;q.h<p.h+dh;q.h++) 
      if (AlphaZero(q,image,width,height)) 
         return(FALSE);

   q.h = p.h;
   for (q.v=p.v;q.v<p.v+dv;q.v++) 
      if (AlphaZero(q,image,width,height)) 
         return(FALSE);

   q.h = p.h + dh;
   for (q.v=p.v;q.v<p.v+dv;q.v++) 
      if (AlphaZero(q,image,width,height)) 
         return(FALSE);

	return(TRUE);
}

/*
	Return true if the alpha value at p in the image is zero
	Return true also if the image is sampled out of bounds
*/
int AlphaZero(POINT p,BITMAP4 *image,int width,int height)
{
	int index;

	if (p.h < 0 || p.v < 0 || p.h >= width || p.v >= height)
		return(TRUE);
	index = p.v * width + p.h;
	if (image[index].a == 0)
		return(TRUE);
	else
		return(FALSE);
}

// ------------------ The following are rough and ready debugging tools -------------------

/*
	Part of debugging only
	Mask is on the alpha channel
*/
void SaveMasks(void)
{
	int n;
	FILE *fptr = NULL;
	char fname[256];

   for (n=0;n<nmaterials;n++) {
      sprintf(fname,"mask_%d.tga",n);
      fptr = fopen(fname,"w");
      Write_Bitmap(fptr,materials[n].image,materials[n].width,materials[n].height,13);
      fclose(fptr);
   }
}

/*
	Part of debugging only
	Note that this damages the material textures, need to reload afterwards and compute masks.
*/
void SaveIslands(void)
{
	int n,i,j,nmat,npoly;
	FILE *fptr = NULL;
	char fname[256];
   BITMAP4 c,blue = {0,0,255,0},cyan = {0,255,255,0},green = {0,255,0,0};
   BITMAP4 yellow = {255,255,0,0},red = {255,0,0,0},pink = {255,0,255,0};
	double dx,dy,d;

   for (n=0;n<nislands;n++) {
      npoly = islands[n].npoly;
      nmat = islands[n].matid;

		// Island border
      for (i=0;i<islands[n].npoly;i++) {
         j = (i+1)%npoly;
         dx = islands[n].poly[i].h - islands[n].poly[j].h;
         dy = islands[n].poly[i].v - islands[n].poly[j].v;
         d = sqrt(dx*dx+dy*dy);
         c = blue; // Single line
         if (d > 1) // 1 diagonal + 2 straight
            c = cyan;
         if (d > 2) // 2 diagonal or 3 straight
            c = green;
         if (d > 3) // 3 diagonals or 4 straight
            c = yellow;
         if (d > 5)
            c = red;
         Draw_Line(materials[nmat].image,materials[nmat].width,materials[nmat].height,
            islands[n].poly[i].h,islands[n].poly[i].v,
            islands[n].poly[j].h,islands[n].poly[j].v,c);
      }

		// Island bounding box
		Draw_Line(materials[nmat].image,materials[nmat].width,materials[nmat].height,
			islands[n].themin.h,islands[n].themin.v,islands[n].themax.h,islands[n].themin.v,pink);
      Draw_Line(materials[nmat].image,materials[nmat].width,materials[nmat].height,
         islands[n].themax.h,islands[n].themin.v,islands[n].themax.h,islands[n].themax.v,pink);
      Draw_Line(materials[nmat].image,materials[nmat].width,materials[nmat].height,
         islands[n].themax.h,islands[n].themax.v,islands[n].themin.h,islands[n].themax.v,pink);
      Draw_Line(materials[nmat].image,materials[nmat].width,materials[nmat].height,
         islands[n].themin.h,islands[n].themax.v,islands[n].themin.h,islands[n].themin.v,pink);
   }
  
   // Save textures
   for (n=0;n<nmaterials;n++) {
      sprintf(fname,"border_%d.tga",n);
      fptr = fopen(fname,"w");
      Write_Bitmap(fptr,materials[n].image,materials[n].width,materials[n].height,13);
      fclose(fptr);
   }
}

/*
   Part of debugging only
	Intended to put borders around the final new texture
*/
void SaveIslands2(BITMAP4 *image,int width,int height)
{
   int n,i,j,nmat,npoly;
   FILE *fptr = NULL;
   char fname[256];
   BITMAP4 c,blue = {0,0,255,0},cyan = {0,255,255,0},green = {0,255,0,0};
   BITMAP4 yellow = {255,255,0,0},red = {255,0,0,0},pink = {255,0,255,0};
   double dx,dy,d;

   for (n=0;n<nislands;n++) {
      npoly = islands[n].npoly;
      nmat = islands[n].matid;

      // Island border
      for (i=0;i<islands[n].npoly;i++) {
         j = (i+1)%npoly;
         dx = islands[n].poly[i].h - islands[n].poly[j].h;
         dy = islands[n].poly[i].v - islands[n].poly[j].v;
         d = sqrt(dx*dx+dy*dy);
         c = blue; // Single line
         if (d > 1) // 1 diagonal + 2 straight
            c = cyan;
         if (d > 2) // 2 diagonal or 3 straight
            c = green;
         if (d > 3) // 3 diagonals or 4 straight
            c = yellow;
         if (d > 5)
            c = red;
/*
         Draw_Line(image,width,height,
            islands[n].poly[i].h-islands[n].offset.h,islands[n].poly[i].v-islands[n].offset.v,
            islands[n].poly[j].h-islands[n].offset.h,islands[n].poly[j].v-islands[n].offset.v,c);
*/
      }

      // Island bounding box
      Draw_Line(image,width,height,
         islands[n].themin.h-islands[n].offset.h,islands[n].themin.v-islands[n].offset.v,
			islands[n].themax.h-islands[n].offset.h,islands[n].themin.v-islands[n].offset.v,pink);
      Draw_Line(image,width,height,
         islands[n].themax.h-islands[n].offset.h,islands[n].themin.v-islands[n].offset.v,
			islands[n].themax.h-islands[n].offset.h,islands[n].themax.v-islands[n].offset.v,pink);
      Draw_Line(image,width,height,
         islands[n].themax.h-islands[n].offset.h,islands[n].themax.v-islands[n].offset.v,
			islands[n].themin.h-islands[n].offset.h,islands[n].themax.v-islands[n].offset.v,pink);
      Draw_Line(image,width,height,
         islands[n].themin.h-islands[n].offset.h,islands[n].themax.v-islands[n].offset.v,
			islands[n].themin.h-islands[n].offset.h,islands[n].themin.v-islands[n].offset.v,pink);
   }

   // Save textures
   for (n=0;n<nmaterials;n++) {
      sprintf(fname,"newborder.tga");
      fptr = fopen(fname,"w");
      Write_Bitmap(fptr,image,width,height,13);
      fclose(fptr);
   }
}

