#include "retexture.h"

#define VERSION "1.00"

/*
	Convert a texture sparse OBJ file to one with more efficient texture packing.
	Assumes the texture sparce OBJ file is the product of subdividion and/or splitting.
	Stages
		1. Read the OBJ file, most important thing is UV coordinates per face
		2. Read the material data and load texture image files
		3. Mask out the parts of the texture not used
		4. Create a polygon around the islands of texture pixels actually used
		5. Pack the textures into a single new texture image
		6. Adjust UV coordinates dependent on the island polygon they reside in
		7. Write new OBJ file, same as input except for modified UV values and new material names
	10 Jul 2020: First version
	25 Jul 2020: Completed texture packing, stage 4
	09 Aug 2020: Fixed multiple identical texture issue 
	14 Aug 2020: Replaced pointinpolygon with the fast winding solution
*/

// Command line parameters
PARAMS params;

// All the vertices, global bounding box
XYZ *vertices = NULL;
long nvertices = 0;
XYZ themin = {1e32,1e32,1e32},themax = {-1e32,-1e32,-1e32},themid;

// All the UV coordinates and bounding box
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

// Islands, defined as the used portions (by the faces) of the texture maps
ISLAND *islands = NULL;
int nislands = 0;

// The final output texture image, islands will be packing into this
BITMAP4 *newtex = NULL;
int newwidth = 8192,newheight = 8192;

int main(int argc,char **argv)
{
   int i;
   char objname[256];
   FILE *fptr;
	double starttime;

	Init();

   // Deal with command line
   if (argc < 2) 
      GiveUsage(argv[0]);
   for (i=1;i<argc;i++) {
      if (strcmp(argv[i],"-v") == 0)
         params.verbose = TRUE;
      if (strcmp(argv[i],"-d") == 0) { // Also enables verbose
         params.debug = TRUE;
			params.verbose = TRUE;
		}
   }
   strcpy(objname,argv[argc-1]);

   // Parse the OBJ file and read the textures
	starttime = GetRunTime();
   if ((fptr = fopen(objname,"r")) == NULL) {
      fprintf(stderr,"Failed to open input OBJ file \"%s\"\n",objname);
      exit(-1);
   }
	fprintf(stderr,"Processing file \"%s\"\n",objname);
   if (!ReadObj(fptr)) {
      fclose(fptr);
      exit(-1);
   }
   fclose(fptr);
	ReadTextures();
   if (params.verbose) 
		ReportTime(starttime,"Time to read model");

	// Create a mask of used portions of the texture image
	// Create a polygon around each island of used texture
	// Sort islands by decreasing area
	starttime = GetRunTime();
	MaskTextures();
	if (params.debug)
		SaveMasks();
	FindIslands();
	if (params.debug)
		SaveIslands();
	if (params.debug) { // Remask if in debug mode, texture images are destroyed in FindIslands()
		ReadTextures();
		MaskTextures();
	}
	qsort((void *)islands,nislands,sizeof(ISLAND),islandcompare);
	if (params.verbose) { // Summary of islands
		for (i=0;i<nislands;i++) {
			fprintf(stderr,"   Island %5d has area %7d and bounds (%5d,%5d) to (%5d,%5d), boundary length: %5d\n",
				i,islands[i].area,
				islands[i].themin.h,islands[i].themin.v,islands[i].themax.h,islands[i].themax.v,
				islands[i].npoly);
			if (i > 20) {
				fprintf(stderr,"   The remaining %d islands are smaller\n",nislands-20);
				break;
			}
		}
	}
   if (params.verbose)
      ReportTime(starttime,"Time to create island boundaries");

	// Pack the textures into a new texture
	starttime = GetRunTime();
	PackIslands(objname);
   if (params.verbose) {
      ReportTime(starttime,"Time to pack islands");
		fprintf(stderr,"Islands packed into a texture image of size %d x %d\n",newwidth,newheight);
	}

	// Write output OBJ
	// At the same time update UVs for each UV
	starttime = GetRunTime();
	WriteOBJFile(argv[0],objname);
   if (params.verbose)
      ReportTime(starttime,"Time to remap UV and save OBJ");
	fprintf(stderr,"\n");

   exit(0);
}

void GiveUsage(char *s)
{
   fprintf(stderr,"Usage: %s [options] objfilename\n",s);
	fprintf(stderr,"Version: %s\n",VERSION);
   fprintf(stderr,"Options\n");
   fprintf(stderr,"   -v     verbose mode, default: disabled\n");
	fprintf(stderr,"   -d     debug mode, save images at various stages, default: disabled\n");

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
	int i,found,currmatid=0;
   long linecount = 0;
   long ram;
   char aline[1024],ignore[256],matname[256];
	float u,v;
   XYZ p;
	FACE aface;

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
         nvertices++;
         MinMaxXYZ(p,&themin,&themax);
         continue;
      }

      // Texture coordinate line
      if (aline[0] == 'v' && aline[1] == 't' && aline[2] == ' ') {
         if (sscanf(aline+3,"%f %f",&u,&v) != 2) {
            fprintf(stderr,"Error reading uv coordinate line %ld\n",linecount);
            fprintf(stderr,"Line: \"%s\"\n",aline);
            break;
         }
			while (u < 0) u += 1;
			while (v < 0) v += 1;
			while (u > 1) u -= 1;
			while (v > 1) v -= 1;
			if (u == 1) u -= EPS;
         if (v == 1) v -= EPS;
         if ((uv = realloc(uv,(nuv+1)*sizeof(UV))) == NULL) {
				fprintf(stderr,"Ran out of memory allocating memory for UV coordinates\n");
				return(FALSE);
			}
         uv[nuv].u = u;
			uv[nuv].v = v;
			uv[nuv].materialid = currmatid;
         nuv++;
			if (u < alluvmin.x) alluvmin.x = u;
         if (u > alluvmax.x) alluvmax.x = u;
         if (v < alluvmin.y) alluvmin.y = v;
         if (v > alluvmax.y) alluvmax.y = v;
         continue;
      }

      // Normals, ignore
      if (aline[0] == 'v' && aline[1] == 'n' && aline[2] == ' ') {
         continue;
      }

      // Count the faces, optionally load into memory
      if (aline[0] == 'f' && aline[1] == ' ') {
         if (!ParseFace(aline,&aface)) {
            fprintf(stderr,"Face parsing error occured on line %ld\n",linecount);
            return(FALSE);
         }
			if ((faces = realloc(faces,(nfaces+1)*sizeof(FACE))) == NULL) {
				fprintf(stderr,"Ran out of memory allocating memory for faces\n");
				return(FALSE);
			}
			CopyFace(&(aface),&(faces[nfaces]));
			faces[nfaces].materialid = currmatid;
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
			sscanf(aline,"%s %s",ignore,matname);
			// Do we already have a material of this name?
			found = FALSE;
			for (i=0;i<nmaterials;i++) {
				if (strcmp(materials[i].name,matname) == 0) {
					found = TRUE;
					currmatid = i;
					break;
				}
			}
			if (!found) {
				materials = realloc(materials,(nmaterials+1)*sizeof(MATERIAL));
            currmatid = nmaterials;
				if (params.verbose)
					fprintf(stderr,"Found new material \"%s\", id: %d\n",matname,currmatid);
				strcpy(materials[nmaterials].name,matname);
				strcpy(materials[nmaterials].texture,"");
				if (!ReadMaterial(materialfile,matname,nmaterials)) 
					fprintf(stderr,"Failed to find texture file for material \"%s\"\n",matname);
				nmaterials++;
			}
     	   continue;
     	}
   }

	// Was it a useful OBJ file?
	if (nfaces <= 0) {
		fprintf(stderr,"Failed to find any faces in the OBJ file\n");
		return(FALSE);
	}

   if (params.verbose) {
      fprintf(stderr,"Found %ld vertices, ",nvertices);
      fprintf(stderr,"%ld uv coordinates, ",nuv);
      fprintf(stderr,"and %ld faces\n",nfaces);
		ram = (nvertices*sizeof(XYZ)+nuv*sizeof(UV))/1024/1024;
      if (ram < 1024)
         ; //fprintf(stderr,"Require %ld MB of memory for vertices and uv\n",ram);
      else
         fprintf(stderr,"Require %ld GB of memory for vertices and uv\n",ram/1024);
		ram = nfaces*sizeof(FACE)/1024/1024;
	   if (ram < 1024)
	      ; //fprintf(stderr,"Require %ld MB of memory for faces\n",ram);
	   else
	      fprintf(stderr,"Require %ld GB of memory for faces\n",ram/1024);
		fprintf(stderr,"Found %d materials\n",nmaterials);
      fprintf(stderr,"Bounding box\n");
      fprintf(stderr,"   %g <= x <= %g\n",themin.x,themax.x);
      fprintf(stderr,"   %g <= y <= %g\n",themin.y,themax.y);
      fprintf(stderr,"   %g <= z <= %g\n",themin.z,themax.z);
		fprintf(stderr,"UV range\n");
		fprintf(stderr,"   %.3f <= u <= %.3f\n",alluvmin.x,alluvmax.x);
      fprintf(stderr,"   %.3f <= v <= %.3f\n",alluvmin.y,alluvmax.y);
   }

   return(TRUE);
}

/*
	Read the texture images referenced by the materials
	Only support jpeg at this stage
	If a material doesn't use a texture, make one up, for consistentcy later on.
	Limitations: Only supports materials with textures, plain colours will fail
	Update the newwidth and newheight planned for the island packing.
*/
int ReadTextures(void)
{
	int i,j;
	FILE *fptr;
	int width,height,depth;
	int maxtsize = 0;
	BITMAP4 missing = {255,128,128,0};

	// For each material
	for (i=0;i<nmaterials;i++) {
		if (params.verbose)
			fprintf(stderr,"Reading material %d with texture \"%s\"\n",i,materials[i].texture);

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

		maxtsize = MAX(maxtsize,height);
		maxtsize = MAX(maxtsize,width);
	}

	newwidth = maxtsize * 1.5;
	newheight = newwidth;

	return(TRUE);
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
						sscanf(aline,"%s %s",ignore,materials[materialid].texture);
						if (params.verbose)
							fprintf(stderr,"Found texture \"%s\" for material \"%s\"\n",
								materials[materialid].texture,material);
						break;
					}
				}
				break;
			}
		}
	}
	if (strlen(materials[materialid].texture) < 2)
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
      fprintf(stderr,"Encountered an unexpectedly formatted face\n");
      return(FALSE);
   }

   return(TRUE);
}

/*
	Default value of the parameters
*/
void Init(void)
{
	params.verbose = FALSE;
	params.debug = FALSE;
}

/*
	Illustrate the parts of the textures actually used
	If the alpha channel is 255 then it is not used by triangles, 0 if used
	At the end of this, any part of the texture unused is also overwritten as black
*/
void MaskTextures(void)
{
	int i,j,n,matid;
	POINT t[3];
	POINT uvmin,uvmax;
	int index;
	long sum = 0;
	double ratio;
	//BITMAP4 black = {0,0,0,255};

	// Set alpha channel initially to 255 on all textures
	for (n=0;n<nmaterials;n++) {
		for (i=0;i<materials[n].width*materials[n].height;i++)
			materials[n].image[i].a = 255;
	}

	// Mark out used portions and set alpha to 0
	// This is done by considering the uv triangle for each triangular face
	for (n=0;n<nfaces;n++) {
		matid = faces[n].materialid;

		// Convert the triangle in 0...1 UV coordinastes to pixel space
		// Assume out of range is meant to wrap, see wrapping on UV reading code
		for (i=0;i<3;i++) {
			t[i].h = materials[matid].width * uv[faces[n].uvid[i]].u;
			t[i].v = materials[matid].height * uv[faces[n].uvid[i]].v;
		}

		// Determine bounding box of uv triangle
		uvmin = t[0];
		uvmax = t[0];
		for (i=1;i<3;i++) {
			uvmin.h = MIN(uvmin.h,t[i].h);
         uvmin.v = MIN(uvmin.v,t[i].v);
         uvmax.h = MAX(uvmax.h,t[i].h);
         uvmax.v = MAX(uvmax.v,t[i].v);
		}

		// Extend it slightly
		uvmin.h -= 1;
		uvmin.v -= 1;
		uvmax.h += 1;
		uvmax.v += 1;
		PointClip(materials[matid].width,materials[matid].height,&uvmin);
      PointClip(materials[matid].width,materials[matid].height,&uvmax);

		// Adjust the mask of the rectangle that is used by this triangle, set alpha to 0
		for (i=uvmin.h;i<uvmax.h;i++) {
			for (j=uvmin.v;j<uvmax.v;j++) {
				index = j * materials[matid].width + i;
				materials[matid].image[index].a = 0;
			}
		}
	}

   // Erase all unused portions, calculate ratio of island to total area
   for (n=0;n<nmaterials;n++) {
      for (i=0;i<materials[n].width*materials[n].height;i++) {
         if (materials[n].image[i].a > 0) 
				//materials[n].image[i] = black;
				materials[n].image[i].a = 255;
			else
				sum++;
		}
   }
	ratio = sum;
	ratio /= (materials[0].width*materials[0].height);
	ratio /= nmaterials; // Assumes they are all the same size
	ratio *= 100;
	if (params.verbose)
		fprintf(stderr,"Total island area: %ld pixels. %0.2lf%% of total area\n",sum,ratio);
	if (ratio > 20)
		fprintf(stderr,"*** Detected a rather high ratio of island area to total area\n");
}

/*
	At this stage we have all the regions (islands) actually used masked.
	Now form the polygon, in pixel coordinates, around each island.
	Reset each point to alpha = 255
	Improvement: simplfying the boundary vectors will speed up packing!
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
		fprintf(stderr,"Creating islands. ");
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

				// Nothing to do if it is a masked pixel
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

            //if (params.verbose)
            //   fprintf(stderr,"   Creating new island %4d with %5d boundary vertices and area of %7d\n",
				//		nislands,islands[nislands].npoly,islands[nislands].area);
				nislands++;
			}
		}
	} // nth material
	if (params.verbose)
		fprintf(stderr,"Created %d islands\n",nislands);

	free(plist);

	// Calculate the island bounding boxes
	for (i=0;i<nislands;i++) {
		for (j=0;j<islands[i].npoly;j++) {
      	islands[i].themin.h = MIN(islands[i].themin.h,islands[i].poly[j].h);
      	islands[i].themax.h = MAX(islands[i].themax.h,islands[i].poly[j].h);
      	islands[i].themin.v = MIN(islands[i].themin.v,islands[i].poly[j].v);
      	islands[i].themax.v = MAX(islands[i].themax.v,islands[i].poly[j].v);
		}
	}

	// Combined vectors where possible for efficiency
	if (params.verbose)
		fprintf(stderr,"Merging vectors. ");
	for (n=0;n<nislands;n++) {
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
	Pack the textures into a destination texture
	Do so into a single large texture image, this will be trimmed later
	Islands are already sorted from the largest to the smallest, packed in this order
*/
void PackIslands(char *objname)
{
	int i,j,n,nmat,index1,index2;
	float dist;
	BITMAP4 black = {0,0,0,255}; // Note alpha is 255
	BITMAP4 *croppedtex = NULL;
	POINT p,newp,newmax = {0,0};
	FILE *fptr = NULL;
	char texname[256];

	// New texture, cleared to black
	newtex = Create_Bitmap(newwidth,newheight);     // See globals for initial size to pack into
	Erase_Bitmap(newtex,newwidth,newheight,black);

	for (n=0;n<nislands;n++) {
		nmat = islands[n].matid;
		if (n < 20)
			fprintf(stderr,"   Packing island %5d, area: %d\n",n,islands[n].area);
		else if (n == 20)
			fprintf(stderr,"   Packing remaining islands ");
		else
			fprintf(stderr,".");

		// Find an available position for the current island
		if (!FindBestPosition(islands[n],newtex,newwidth,newheight,&p)) {
			fprintf(stderr,"Failed to find a location to place island %d !\n",n);
			continue;
		}

		// Update the offset
		islands[n].offset.h = islands[n].themin.h - p.h;
      islands[n].offset.v = islands[n].themin.v - p.v;

		// Copy island from material texture into the destination texture
		for (p.h=islands[n].themin.h;p.h<islands[n].themax.h;p.h++) {
			for (p.v=islands[n].themin.v;p.v<islands[n].themax.v;p.v++) {

            // Calculate new position and calculate the top right most position
            newp.h = p.h - islands[n].offset.h;
            newp.v = p.v - islands[n].offset.v;
            if (newp.h < 0 || newp.v < 0 || newp.h >= newwidth || newp.v >= newheight) {
               fprintf(stderr,"Unexpected out of range pixel during packing pixel copy (%d,%d)\n",newp.h,newp.v);
               continue;
            }

				// Indices on source and destination images
				index1 = p.v * materials[nmat].width + p.h;
				index2 = newp.v * newwidth + newp.h;

            // Set clear flag if not inside polygon, slow for large islands
            if (!PointInIsland(n,p,&dist)) {
					if (dist >= 2) {
						newtex[index2] = black;
						continue;
					}
            }

				// Maximum top right corner needed so far
				newmax.h = MAX(newmax.h,newp.h);
				newmax.v = MAX(newmax.v,newp.v);

				// Copy pixel
				newtex[index2] = materials[nmat].image[index1];
				newtex[index2].a = 0;
			}
		}
	} // nth island
	fprintf(stderr,"\n");

	// Save the whole packed image in debug mode
   if (params.debug)
      SaveIslands2(newtex,newwidth,newheight);

	// Trim the texture to the least required, make factor of 2 in size
	// Copy the bottom left corner to a new cropped texture of required size
	// Irrespective of the relative sizes of the texture maps, no scaling is performed
	// That will left to a later process.
	newmax.h = (newmax.h/2) * 2;
   newmax.v = (newmax.v/2) * 2;
   newmax.h += 4;
   newmax.v += 4;
	if (params.verbose)
		fprintf(stderr,"Final texture has dimensions %d x %d\n",newmax.h,newmax.v);
	croppedtex = Create_Bitmap(newmax.h,newmax.v);
	for (i=0;i<newmax.h;i++) {
		for (j=0;j<newmax.v;j++) {
			index1 = j * newwidth + i;
			index2 = j * newmax.h + i;
			croppedtex[index2] = newtex[index1];
		}	
	}

	// Recreate newtex (global), copy the cropped texture back to newtex
	Destroy_Bitmap(newtex);
	newwidth = newmax.h;
	newheight = newmax.v;
	newtex = Create_Bitmap(newwidth,newheight);
	for (i=0;i<newwidth;i++) {
      for (j=0;j<newheight;j++) {
         index1 = j * newwidth + i;
         newtex[index1] = croppedtex[index1];
      }
   }
	Destroy_Bitmap(croppedtex);

	// Save the new cropped texture, this will be the one references by the new obj (mtl) file
	strcpy(texname,objname);
	StripExtension(texname);
	strcat(texname,".jpg");
	fptr = fopen(texname,"w");
	JPEG_Write(fptr,newtex,newwidth,newheight,100);
	fclose(fptr);
}

/*
	Determine if point p is inside island n, calculate distance if not inside
	No trig functions, based upon winding count method here
		http://geomalgorithms.com/a03-_inclusion.html
*/
int PointInIsland(int n,POINT p,float *dist)
{
   if (p.h < islands[n].themin.h || p.v < islands[n].themin.v ||
       p.h > islands[n].themax.h || p.v > islands[n].themax.v) {
		*dist = ClosestPoint(islands[n].poly,islands[n].npoly,p);
		return(FALSE);
	}
	if (InsidePolygon(p,islands[n].poly,islands[n].npoly)) {
		*dist = 0;
		return(TRUE);
	} else {
      *dist = ClosestPoint(islands[n].poly,islands[n].npoly,p);
      return(FALSE);
   }
}

/*
	Find the best position in the image for an island
	This code scans the destination image (square) in a square out from the bottom left corner
*/
int FindBestPosition(ISLAND island,BITMAP4 *image,int width,int height,POINT *pbest)
{
	int dh,dv,n,border=2;
	POINT p;
	
	dh = island.themax.h - island.themin.h;
	dv = island.themax.v - island.themin.v;

	for (n=border;n<width;n++) {
		p.h = n;
		for (p.v=border;p.v<=n;p.v++) {
			if (!TestRectAlpha(p,dh,dv,image,width,height))
				continue;
			*pbest = p;
			return(TRUE);
		}
		p.v = n;
		for (p.h=border;p.h<n;p.h++) {
         if (!TestRectAlpha(p,dh,dv,image,width,height))
            continue;
         *pbest = p;
         return(TRUE);
      }
	}

	// After all that, didn't find a free location
	// If this happens then something is seriously wrong, one option is that this
	// code is not being applied to a situation where there is a sparce texture.
	return(FALSE);
}

/*
	Return true if the rectangle is in empty space
	Do the various tests based upon their likelihood to return false early
	Rectangle has corners (p.h,p.v) and (p.h+dh,p.v+dv)
*/
int TestRectAlpha(POINT p,int dh,int dv,BITMAP4 *image,int width,int height)
{
	int idh,idv;
	POINT q;

	// Test center
	q.h = p.h + dh/2;
	q.v = p.v + dh/2;
   if (AlphaZero(q,image,width,height))
      return(FALSE);

   // Test corners 
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

	// Test a grid
	idv = dh / 100;
	idh = dv / 100;
	if (idv < 10)
		idv = dh / 10;
	if (idh < 10)
		idh = dh / 10;
   if (idv < 1)
      idv = 1;
   if (idh < 1)
      idh = 1;
	for (q.v=p.v;q.v<p.v+dv;q.v+=idv) {
		for (q.h=p.h;q.h<p.h+dh;q.h+=idh) {
   	   if (AlphaZero(q,image,width,height))
	         return(FALSE);
		}
	}

	return(TRUE);
}

/*
	Write the final OBJ file
	Remap all uv coordinate to correctly point to the new texture
*/
int WriteOBJFile(char *s,char *srcname)
{
	int i,j,k,isl,nmat,index;
	int distmin,dist2,dh,dv;
	float dist;
	BITMAP4 errorcolour = {255,0,255,0};
	FILE *fobj = NULL,*fmtl = NULL;
	POINT old;
	UV new;
	char basename[256],mtlname[256],objname[256];

	strcpy(basename,srcname);
	StripExtension(basename);

	// Write the material file first
	sprintf(mtlname,"%s_r.mtl",basename);
   fmtl = fopen(mtlname,"w");
	fprintf(fmtl,"# Material definition created by %s\n",s);
	fprintf(fmtl,"newmtl packed\n");
	fprintf(fmtl,"Ka 0.2 0.2 0.2\n");
	fprintf(fmtl,"Kd 1.0 1.0 1.0\n");
	fprintf(fmtl,"Ks 1.0 1.0 1.0\n");
	fprintf(fmtl,"Tr 1.0\n");
	fprintf(fmtl,"illum 2\n");
	fprintf(fmtl,"Ns 0.0\n");
	fprintf(fmtl,"map_Kd %s.jpg\n",basename);
	fclose(fmtl);

   sprintf(objname,"%s_r.obj",basename);
	fobj = fopen(objname,"w");
	fprintf(fobj,"#\n");
   fprintf(fobj,"# Source OBJ file: %s\n",srcname);
   fprintf(fobj,"#    Consists of %ld vertices\n",nvertices);
   fprintf(fobj,"#    Consists of %ld uv coordinates\n",nuv);
   fprintf(fobj,"#    Consists of %ld faces\n",nfaces);
   fprintf(fobj,"#    Global box X bounds: %f <= x <= %f, length: %f\n",
      themin.x,themax.x,themax.x-themin.x);
   fprintf(fobj,"#    Global box Y bounds: %f <= y <= %f, length: %f\n",
      themin.y,themax.y,themax.y-themin.y);
   fprintf(fobj,"#    Global box Z bounds: %f <= z <= %f, length: %f\n",
      themin.z,themax.z,themax.z-themin.z);
   fprintf(fobj,"#    Global box center: %f %f %f\n",
      (themin.x+themax.x)/2,
      (themin.y+themax.y)/2,
      (themin.z+themax.z)/2);
	fprintf(fobj,"# Found %d texture islands\n",nislands);
	fprintf(fobj,"# Final packed texture size of %d x %d\n",newwidth,newheight);
	fprintf(fobj,"#\n");
	fprintf(fobj,"mtllib %s\n",mtlname);
	fprintf(fobj,"usemtl packed\n");

	// Vertices
	fprintf(fobj,"# %ld vertices\n",nvertices);
	for (i=0;i<nvertices;i++) 
		fprintf(fobj,"v %lf %lf %lf\n",vertices[i].x,vertices[i].y,vertices[i].z);

	// Texture coordinates, remapped
	fprintf(fobj,"# %ld uv coordinates\n",nuv);
	for (i=0;i<nuv;i++) {
		nmat = uv[i].materialid;

		// Find which island the UV coordinte is in
		// First test bounding box (fast), then test polygon itself (slow)
		old.h = uv[i].u * materials[nmat].width;
		old.v = uv[i].v * materials[nmat].height;
		isl = -1;
		for (j=0;j<nislands;j++) {
			if (PointInIsland(j,old,&dist)) {
				isl = j;
				break;
			}
		}
	
		// This should not happen, in the past was an edge effect
		// Mark the texture, use closest point on edge
		if (isl < 0) {
			fprintf(stderr,"No island found containing UV coordinate %6d, (%4d,%4d)\n",i,old.h,old.v);
			index = old.v * materials[nmat].width + old.h;
			materials[nmat].image[index] = errorcolour;
			distmin = 32000;
			for (j=0;j<nislands;j++) {
				for (k=0;k<islands[j].npoly;k++) {
					dh = islands[j].poly[k].h - old.h;
               dv = islands[j].poly[k].v - old.v;
					dist2 = dh * dh + dv * dv;
					if (dist2 < distmin) {
						distmin = dist2;
						isl = j;
					}
					if (distmin == 0)
						break;
				}
            if (distmin == 0)
               break;
			}
		}
		new.u = (old.h - islands[isl].offset.h) / (double)newwidth;
		new.v = (old.v - islands[isl].offset.v) / (double)newheight;
		fprintf(fobj,"vt %lf %lf\n",new.u,new.v);
	}

	// Faces
	fprintf(fobj,"# %ld faces\n",nfaces);
	for (i=0;i<nfaces;i++) 
		fprintf(fobj,"f %d/%d %d/%d %d/%d\n",
			faces[i].xyzid[0]+1,faces[i].uvid[0]+1,
         faces[i].xyzid[1]+1,faces[i].uvid[1]+1,
         faces[i].xyzid[2]+1,faces[i].uvid[2]+1);

	fclose(fobj);

	return(TRUE);
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
   BITMAP4 yellow = {255,255,0,0},red = {255,0,0,0}; //,pink = {255,0,255,0};
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

      /* Island bounding box
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
		*/
   }

   // Save textures
   for (n=0;n<nmaterials;n++) {
      sprintf(fname,"newborder.tga");
      fptr = fopen(fname,"w");
      Write_Bitmap(fptr,image,width,height,13);
      fclose(fptr);
   }
}


