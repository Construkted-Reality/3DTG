#include "gridsplit.h"

#define VERSION "1.00"

/*
   Take a textured OBJ file and split into octtree pieces given a certain depth
	Based upon origin "split.c"
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

// All the faces
FACE *faces = NULL;
long nfaces = 0;

// Materials
char materialfile[256] = "missing.mtl";
MATERIALNAME *materials = NULL;
int nmaterials = 0;

// Bounding boxes
BBOX *bboxes = NULL;
int nbboxes = 0;

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
      if (strcmp(argv[i],"-n") == 0) {
         params.depth = atoi(argv[i+1]);
			if (params.depth < 1) {
				fprintf(stderr,"Splitting depth must be greater than 0, resetting to 1\n");
				params.depth = 1;
			}
         if (params.depth > 3) {
            fprintf(stderr,"Splitting depth greater than 3 not yet supported, resetting to 3\n");
            params.depth = 3;
         }
		}
      if (strcmp(argv[i],"-t") == 0)
         params.trisplit = !params.trisplit;
   }
	params.depth = 1 << params.depth;
	if (params.verbose)
		fprintf(stderr,"Splitting the model %d times along each axis\n",params.depth);
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

   // Create the bounding boxes based upon vertex counts
	starttime = GetRunTime();
   GenerateBBoxes(); 
   stoptime = GetRunTime();
   if (params.verbose)
      fprintf(stderr,"Time to create boxes: %.1lf seconds\n",stoptime-starttime);

   // Read the faces and allocate to correct subvolume
	starttime = GetRunTime();
   ProcessFaces(objname);
   stoptime = GetRunTime();
   if (params.verbose)
      fprintf(stderr,"Time process geometry: %.1lf seconds\n",stoptime-starttime);

   // Create an obj file showing the bounding boxes, debugging purposes only
   if (params.verbose) 
		SaveBBoxes();

   exit(0);
}

void GiveUsage(char *s)
{
   fprintf(stderr,"Usage: %s [options] objfilename\n",s);
	fprintf(stderr,"Version: %s\n",VERSION);
   fprintf(stderr,"Options\n");
   fprintf(stderr,"   -v     verbose mode, default: disabled\n");
	fprintf(stderr,"   -t     toggle whether to split triangles at boundary, default: on\n");
   fprintf(stderr,"   -n n   power of 2 depth to split along each axis, default: %d\n",params.depth);
	fprintf(stderr,"   -d n   issue decimate script for cells with less than this number of triangles\n");

   exit(0);
}

/*
   Read all vertices and uv coordinates from OBJ file
   Currently ignore vertex colours and normals
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
         if ((uv = realloc(uv,(nuv+1)*sizeof(UV))) == NULL) {
				fprintf(stderr,"Ran out of memory allocating memory for UV coordinates\n");
				return(FALSE);
			}
         uv[nuv] = t;
			uv[nuv].flag = -1;
         nuv++;
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
			materials = realloc(materials,(nmaterials+1)*sizeof(MATERIALNAME));
			sscanf(aline,"%s %s",ignore,materials[nmaterials].name);
			if (params.verbose)
				fprintf(stderr,"Found material \"%s\"\n",materials[nmaterials].name);
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
		fprintf(stderr,"Found %d materials\n",nmaterials-1);
      fprintf(stderr,"Bounding box\n");
      fprintf(stderr,"   %g <= x <= %g\n",themin.x,themax.x);
      fprintf(stderr,"   %g <= y <= %g\n",themin.y,themax.y);
      fprintf(stderr,"   %g <= z <= %g\n",themin.z,themax.z);
   }

   // Tweek to get around issue of face vertices lying exactly on the bounding box
   themin.x -= EPS;
   themin.y -= EPS;
   themin.z -= EPS;
   themax.x += EPS;
   themax.y += EPS;
   themax.z += EPS;
   themid.x = (themin.x + themax.x)/2;
   themid.y = (themin.y + themax.y)/2;
   themid.z = (themin.z + themax.z)/2;

   return(TRUE);
}

/*
   Process faces and deal with materials
	Determines if any part of the face is in the current bounding box?
	Optionally split the face
*/
int ProcessFaces(char *fname)
{
   int d,i,j,n,k,ix,iy,iz;
   int xyzcount,uvcount;
	int currentmaterial = -1;
   char objname[256];
   FILE *fobj,*fptr;
   XYZ p;
   UV u;
   FACE *aface = NULL;
	int nf = 0;
	int maxface = 50; // Actually I think 16 is the maximum possible

	aface = malloc(maxface*sizeof(FACE));

	fptr = fopen("decimatescript","w");
	fprintf(fptr,"# Decimate each grid cell\n");

   // Process each box in turn
   for (n=0;n<nbboxes;n++) {
		currentmaterial = -1;
      if (params.verbose)
         fprintf(stderr,"Processing faces for box %4d of %4d",n+1,nbboxes);

		// Number of contained faces
      bboxes[n].facecount = 0;

      // Create the nth output obj file
      FormFileName(objname,fname,bboxes[n].ix,bboxes[n].iy,bboxes[n].iz,params.depth);
		strcat(objname,".obj");
      if ((fobj = fopen(objname,"w")) == NULL) {
         fprintf(stderr,"Failed to open output file \"%s\"\n",objname);
         return(FALSE);
      }
      fprintf(fobj,"#\n");
      fprintf(fobj,"# Source OBJ file: %s\n",fname);
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
      fprintf(fobj,"# Section %d of %d sections, index (%d,%d,%d)\n",
			n+1,nbboxes,bboxes[n].ix,bboxes[n].iy,bboxes[n].iz);
      fprintf(fobj,"#    Consists of %ld vertices\n",bboxes[n].vertexcount);
      fprintf(fobj,"# Splitting box X bounds: %lf <= x <= %lf, length: %lf\n",
         bboxes[n].themin.x,bboxes[n].themax.x,bboxes[n].themax.x-bboxes[n].themin.x);
      fprintf(fobj,"# Splitting box Y bounds: %lf <= y <= %lf, length: %lf\n",
         bboxes[n].themin.y,bboxes[n].themax.y,bboxes[n].themax.y-bboxes[n].themin.y);
      fprintf(fobj,"# Splitting box Z bounds: %lf <= z <= %lf, length: %lf\n",
         bboxes[n].themin.z,bboxes[n].themax.z,bboxes[n].themax.z-bboxes[n].themin.z);
      fprintf(fobj,"# Splitting box center: %lf %lf %lf\n",
         (bboxes[n].themin.x+bboxes[n].themax.x)/2,
         (bboxes[n].themin.y+bboxes[n].themax.y)/2,
         (bboxes[n].themin.z+bboxes[n].themax.z)/2);
      fprintf(fobj,"#\n");

		fprintf(fobj,"mtllib %s\n",materialfile);

      // flags are used to keep a sequential count of the added vertex ids
      // -1 in the vertex and uv flag means vertex is new, not yet saved
      for (i=0;i<nvertices;i++)
         vertices[i].flag = -1;
      xyzcount = 0;
      for (i=0;i<nuv;i++)
         uv[i].flag = -1;
      uvcount = 0;

		for (i=0;i<nfaces;i++) {

			nf = 1;
			CopyFace(&(faces[i]),&(aface[0]));

			// Write the material when it changes
			if (aface[0].materialid != currentmaterial) {
				fprintf(fobj,"usemtl %s\n",materials[aface[0].materialid].name);
				currentmaterial = aface[0].materialid;
			}
				
         // Is this face within the bounding box
         if (!FaceInBox(aface[0],bboxes[n]))
				continue;

			// Split the face if there are 1 or 2 vertices inside the bounding box
			if (params.trisplit) {
				nf = SplitFaceX(bboxes[n],aface,nf);
            nf = SplitFaceY(bboxes[n],aface,nf);
            nf = SplitFaceZ(bboxes[n],aface,nf);
				if (nf > 16)
					fprintf(stderr,"Split to %d faces, not ever expecting more than 16\n",nf);
			}

			for (k=0;k<nf;k++) { // The k'th split face

				// Filter fragments that can arise from corners
				if (params.trisplit) {
      	   	if (!FaceInBox(aface[k],bboxes[n]))
						continue;
				}

         	for (j=0;j<3;j++) {

         	   // This vertex has not yet been saved
         	   // Allocate it the next number in the sequence
         	   // Save the vertex to the output OBJ file
         	   // Remember the number the next time it is encountered
         	   if (vertices[aface[k].xyzid[j]].flag < 0) {
         	      vertices[aface[k].xyzid[j]].flag = xyzcount;
         	      xyzcount++;
         	      p = vertices[aface[k].xyzid[j]];
         	      fprintf(fobj,"v %lf %lf %lf\n",p.x,p.y,p.z);
         	   }
         	   aface[k].xyzid[j] = vertices[aface[k].xyzid[j]].flag;
	
	            // Treat the uv coordinates the same as vertices above
	            if (uv[aface[k].uvid[j]].flag < 0) {
	               uv[aface[k].uvid[j]].flag = uvcount;
	               uvcount++;
	               u = uv[aface[k].uvid[j]];
	               fprintf(fobj,"vt %lf %lf\n",u.u,u.v);
	            }
	            aface[k].uvid[j] = uv[aface[k].uvid[j]].flag;
	
	         } // j
	
	         // Finally save the face definition using the new numbers
	         fprintf(fobj,"f %d/%d %d/%d %d/%d\n",
	            aface[k].xyzid[0]+1,aface[k].uvid[0]+1,
	            aface[k].xyzid[1]+1,aface[k].uvid[1]+1,
	            aface[k].xyzid[2]+1,aface[k].uvid[2]+1);

   	      bboxes[n].facecount++;
			} // k of nf split faces
		}
	
      // Report on final number of faces
      fprintf(fobj,"#\n");
      fprintf(fobj,"# Total faces: %ld\n",bboxes[n].facecount);
      fprintf(fobj,"#\n");
      fclose(fobj);

	   if (params.verbose)
   	   fprintf(stderr,". Contains %ld faces\n",bboxes[n].facecount);

		WriteDecimateLine(fptr,objname);
   } // n

	// Write merge and tessellation command to script
	fprintf(fptr,"\n");
	d = params.depth;
	do {
		fprintf(fptr,"# depth = %d\n",d);
   	for (ix=0;ix<d;ix+=2) {
  		   for (iy=0;iy<d;iy+=2) {
   	  	   for (iz=0;iz<d;iz+=2) {
					n = ix * d * d + iy * d + iz;
					fprintf(fptr,"%s -i ",MESHLABPATH);
					for (i=0;i<2;i++) {
						for (j=0;j<2;j++) {
							for (k=0;k<2;k++) {
								FormFileName(objname,fname,ix+i,iy+j,iz+k,d);
								if (ObjExists(objname)) { // To handle empty grid cells that MeshLab will fail on
									fprintf(fptr,"%s_d.obj ",objname);
								} else if (d != params.depth) {
									fprintf(fptr,"%s_d.obj ",objname);
								} else {
									fprintf(stderr,"Didn't find \"%s\", probably empty\n",objname);
								}
							}
						}
					}
					FormFileName(objname,fname,ix/2,iy/2,iz/2,d/2);
					strcat(objname,".obj");
					fprintf(fptr," -o %s -m wt -s flatten.mlx\n",objname);
					WriteDecimateLine(fptr,objname);
				}
			}
		}
		d /= 2;
		fprintf(fptr,"\n");
	} while (d > 1);
	fclose(fptr);

   return(TRUE);
}

/*
	Issue decimate script
*/
void WriteDecimateLine(FILE *fptr,char *s)
{
	int i;
	char fname[256];

	// Form output file name
	strcpy(fname,s);
	for (i=strlen(fname)-1;i>0;i--) {
		if (fname[i] == '.') {
			fname[i] = '\0';
			break;
		}
	}
	strcat(fname,"_d.obj");

	fprintf(fptr,"%s -i %s -o %s -m wt -s decimate.mlx\n",MESHLABPATH,s,fname);
}

/*
	Splits a series of faces between xmin and xmax of the bounding box
	I admit this and the Y and Z axis versions is some of the crappiest code, but efficient.
*/
int SplitFaceX(BBOX box,FACE *face,int n)
{
	int i,k;
	int nf;
	int fp = 0;
	double mu1,mu2;
	XYZ p[3],p1,p2;
	UV uv1,uv2;

	nf = n;

	// One vertex on the interior
	for (i=0;i<nf;i++) {
		for (k=0;k<3;k++) {
			p[0] = vertices[face[i].xyzid[k]];
			p[1] = vertices[face[i].xyzid[(k+1)%3]];
			p[2] = vertices[face[i].xyzid[(k+2)%3]];
			if (p[0].x > box.themin.x && p[1].x < box.themin.x && p[2].x < box.themin.x) {
				p1 = SplitXEdge(box.themin.x,p[0],p[1],&mu1);
   		   p2 = SplitXEdge(box.themin.x,p[0],p[2],&mu2);
   			uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   			uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   			uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   			uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
				AddXYZUV(p1,p2,uv1,uv2);
				//face[i].xyzid[k] = face[i].xyzid[k];
				face[i].xyzid[(k+1)%3] = nvertices-2;
   		   face[i].xyzid[(k+2)%3] = nvertices-1;
				//face[i].uvid[k] = face[i].uvid[k];
   		   face[i].uvid[(k+1)%3] = nuv-2;
   		   face[i].uvid[(k+2)%3] = nuv-1;
				break;
			}
		}
	}

	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	   p[1] = vertices[face[i].xyzid[(k+1)%3]];
   	   p[2] = vertices[face[i].xyzid[(k+2)%3]];
   	   if (p[0].x < box.themax.x && p[1].x > box.themax.x && p[2].x > box.themax.x) {
   	      p1 = SplitXEdge(box.themax.x,p[0],p[1],&mu1);
   	      p2 = SplitXEdge(box.themax.x,p[0],p[2],&mu2);
   	      uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   	      uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   	      uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   	      uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
   	      AddXYZUV(p1,p2,uv1,uv2);
   	      //face[i].xyzid[k] = face[i].xyzid[k];
   	      face[i].xyzid[(k+1)%3] = nvertices-2;
   	      face[i].xyzid[(k+2)%3] = nvertices-1;
   	      //face[i].uvid[k] = face[i].uvid[k];
   	      face[i].uvid[(k+1)%3] = nuv-2;
   	      face[i].uvid[(k+2)%3] = nuv-1;
   	      break;
   	   }
		}
   }


	// Two vertices on the interior
	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	   p[1] = vertices[face[i].xyzid[(k+1)%3]];
   	   p[2] = vertices[face[i].xyzid[(k+2)%3]];
   	   if (p[0].x < box.themin.x && p[1].x > box.themin.x && p[2].x > box.themin.x) {
   	      p1 = SplitXEdge(box.themin.x,p[0],p[1],&mu1);
   	      p2 = SplitXEdge(box.themin.x,p[0],p[2],&mu2);
   	      uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   	      uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   	      uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   	      uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
   	      AddXYZUV(p1,p2,uv1,uv2);
				face[nf].xyzid[k] = nvertices-2;
   	      face[nf].xyzid[(k+1)%3] = face[i].xyzid[(k+1)%3];
   	      face[nf].xyzid[(k+2)%3] = face[i].xyzid[(k+2)%3];
				face[nf].uvid[k] = nuv-2;
   	      face[nf].uvid[(k+1)%3] = face[i].uvid[(k+1)%3];
   	      face[nf].uvid[(k+2)%3] = face[i].uvid[(k+2)%3];
				face[nf].materialid = face[i].materialid;
				nf++;
	
				face[i].xyzid[k] = nvertices-2;
	         face[i].xyzid[(k+1)%3] = face[i].xyzid[(k+2)%3];
	         face[i].xyzid[(k+2)%3] = nvertices-1;
				face[i].uvid[k] = nuv-2;
	         face[i].uvid[(k+1)%3] = face[i].uvid[(k+2)%3];
	         face[i].uvid[(k+2)%3] = nuv-1;
				//face[i].materialid = face[i].materialid;
				break;
	      }
	   }
	}

	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	  	p[1] = vertices[face[i].xyzid[(k+1)%3]];
     		p[2] = vertices[face[i].xyzid[(k+2)%3]];
      	if (p[0].x > box.themax.x && p[1].x < box.themax.x && p[2].x < box.themax.x) {
	         p1 = SplitXEdge(box.themax.x,p[0],p[1],&mu1);
  	      	p2 = SplitXEdge(box.themax.x,p[0],p[2],&mu2);
         	uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
         	uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
         	uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
         	uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
         	AddXYZUV(p1,p2,uv1,uv2);
         	face[nf].xyzid[k] = nvertices-2;
         	face[nf].xyzid[(k+1)%3] = face[i].xyzid[(k+1)%3];
         	face[nf].xyzid[(k+2)%3] = face[i].xyzid[(k+2)%3];
         	face[nf].uvid[k] = nuv-2;
         	face[nf].uvid[(k+1)%3] = face[i].uvid[(k+1)%3];
         	face[nf].uvid[(k+2)%3] = face[i].uvid[(k+2)%3];
         	face[nf].materialid = face[i].materialid;
         	nf++;
	
	         face[i].xyzid[k] = nvertices-2;
	         face[i].xyzid[(k+1)%3] = face[fp].xyzid[(k+2)%3];
	         face[i].xyzid[(k+2)%3] = nvertices-1;
	         face[i].uvid[k] = nuv-2;
	         face[i].uvid[(k+1)%3] = face[fp].uvid[(k+2)%3];
	         face[i].uvid[(k+2)%3] = nuv-1;
	         //face[i].materialid = face[fp].materialid;
	         break;
	      }
		}
   } 

	return(nf);
}

int SplitFaceY(BBOX box,FACE *face,int n) 
{
	int i,k;
	int nf;
	int fp = 0;
	double mu1,mu2;
	XYZ p[3],p1,p2;
	UV uv1,uv2;

	nf = n;

	// One vertex on the interior
	for (i=0;i<nf;i++) {
		for (k=0;k<3;k++) {
			p[0] = vertices[face[i].xyzid[k]];
			p[1] = vertices[face[i].xyzid[(k+1)%3]];
			p[2] = vertices[face[i].xyzid[(k+2)%3]];
			if (p[0].y > box.themin.y && p[1].y < box.themin.y && p[2].y < box.themin.y) {
				p1 = SplitYEdge(box.themin.y,p[0],p[1],&mu1);
   		   p2 = SplitYEdge(box.themin.y,p[0],p[2],&mu2);
   			uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   			uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   			uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   			uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
				AddXYZUV(p1,p2,uv1,uv2);
				//face[i].xyzid[k] = face[i].xyzid[k];
				face[i].xyzid[(k+1)%3] = nvertices-2;
   		   face[i].xyzid[(k+2)%3] = nvertices-1;
				//face[i].uvid[k] = face[i].uvid[k];
   		   face[i].uvid[(k+1)%3] = nuv-2;
   		   face[i].uvid[(k+2)%3] = nuv-1;
				break;
			}
		}
	}

	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	   p[1] = vertices[face[i].xyzid[(k+1)%3]];
   	   p[2] = vertices[face[i].xyzid[(k+2)%3]];
   	   if (p[0].y < box.themax.y && p[1].y > box.themax.y && p[2].y > box.themax.y) {
   	      p1 = SplitYEdge(box.themax.y,p[0],p[1],&mu1);
   	      p2 = SplitYEdge(box.themax.y,p[0],p[2],&mu2);
   	      uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   	      uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   	      uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   	      uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
   	      AddXYZUV(p1,p2,uv1,uv2);
   	      //face[i].xyzid[k] = face[i].xyzid[k];
   	      face[i].xyzid[(k+1)%3] = nvertices-2;
   	      face[i].xyzid[(k+2)%3] = nvertices-1;
   	      //face[i].uvid[k] = face[i].uvid[k];
   	      face[i].uvid[(k+1)%3] = nuv-2;
   	      face[i].uvid[(k+2)%3] = nuv-1;
   	      break;
   	   }
		}
   }


	// Two vertices on the interior
	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	   p[1] = vertices[face[i].xyzid[(k+1)%3]];
   	   p[2] = vertices[face[i].xyzid[(k+2)%3]];
   	   if (p[0].y < box.themin.y && p[1].y > box.themin.y && p[2].y > box.themin.y) {
   	      p1 = SplitYEdge(box.themin.y,p[0],p[1],&mu1);
   	      p2 = SplitYEdge(box.themin.y,p[0],p[2],&mu2);
   	      uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   	      uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   	      uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   	      uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
   	      AddXYZUV(p1,p2,uv1,uv2);
				face[nf].xyzid[k] = nvertices-2;
   	      face[nf].xyzid[(k+1)%3] = face[i].xyzid[(k+1)%3];
   	      face[nf].xyzid[(k+2)%3] = face[i].xyzid[(k+2)%3];
				face[nf].uvid[k] = nuv-2;
   	      face[nf].uvid[(k+1)%3] = face[i].uvid[(k+1)%3];
   	      face[nf].uvid[(k+2)%3] = face[i].uvid[(k+2)%3];
				face[nf].materialid = face[i].materialid;
				nf++;
	
				face[i].xyzid[k] = nvertices-2;
	         face[i].xyzid[(k+1)%3] = face[i].xyzid[(k+2)%3];
	         face[i].xyzid[(k+2)%3] = nvertices-1;
				face[i].uvid[k] = nuv-2;
	         face[i].uvid[(k+1)%3] = face[i].uvid[(k+2)%3];
	         face[i].uvid[(k+2)%3] = nuv-1;
				//face[i].materialid = face[i].materialid;
				break;
	      }
	   }
	}

	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	  	p[1] = vertices[face[i].xyzid[(k+1)%3]];
     		p[2] = vertices[face[i].xyzid[(k+2)%3]];
      	if (p[0].y > box.themax.y && p[1].y < box.themax.y && p[2].y < box.themax.y) {
	         p1 = SplitYEdge(box.themax.y,p[0],p[1],&mu1);
  	      	p2 = SplitYEdge(box.themax.y,p[0],p[2],&mu2);
         	uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
         	uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
         	uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
         	uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
         	AddXYZUV(p1,p2,uv1,uv2);
         	face[nf].xyzid[k] = nvertices-2;
         	face[nf].xyzid[(k+1)%3] = face[i].xyzid[(k+1)%3];
         	face[nf].xyzid[(k+2)%3] = face[i].xyzid[(k+2)%3];
         	face[nf].uvid[k] = nuv-2;
         	face[nf].uvid[(k+1)%3] = face[i].uvid[(k+1)%3];
         	face[nf].uvid[(k+2)%3] = face[i].uvid[(k+2)%3];
         	face[nf].materialid = face[i].materialid;
         	nf++;
	
	         face[i].xyzid[k] = nvertices-2;
	         face[i].xyzid[(k+1)%3] = face[fp].xyzid[(k+2)%3];
	         face[i].xyzid[(k+2)%3] = nvertices-1;
	         face[i].uvid[k] = nuv-2;
	         face[i].uvid[(k+1)%3] = face[fp].uvid[(k+2)%3];
	         face[i].uvid[(k+2)%3] = nuv-1;
	         //face[i].materialid = face[fp].materialid;
	         break;
	      }
		}
   } 

	return(nf);
}

int SplitFaceZ(BBOX box,FACE *face,int n)
{
	int i,k;
	int nf;
	int fp = 0;
	double mu1,mu2;
	XYZ p[3],p1,p2;
	UV uv1,uv2;

	nf = n;

	// One vertex on the interior
	for (i=0;i<nf;i++) {
		for (k=0;k<3;k++) {
			p[0] = vertices[face[i].xyzid[k]];
			p[1] = vertices[face[i].xyzid[(k+1)%3]];
			p[2] = vertices[face[i].xyzid[(k+2)%3]];
			if (p[0].z > box.themin.z && p[1].z < box.themin.z && p[2].z < box.themin.z) {
				p1 = SplitZEdge(box.themin.z,p[0],p[1],&mu1);
   		   p2 = SplitZEdge(box.themin.z,p[0],p[2],&mu2);
   			uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   			uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   			uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   			uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
				AddXYZUV(p1,p2,uv1,uv2);
				//face[i].xyzid[k] = face[i].xyzid[k];
				face[i].xyzid[(k+1)%3] = nvertices-2;
   		   face[i].xyzid[(k+2)%3] = nvertices-1;
				//face[i].uvid[k] = face[i].uvid[k];
   		   face[i].uvid[(k+1)%3] = nuv-2;
   		   face[i].uvid[(k+2)%3] = nuv-1;
				break;
			}
		}
	}

	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	   p[1] = vertices[face[i].xyzid[(k+1)%3]];
   	   p[2] = vertices[face[i].xyzid[(k+2)%3]];
   	   if (p[0].z < box.themax.z && p[1].z > box.themax.z && p[2].z > box.themax.z) {
   	      p1 = SplitZEdge(box.themax.z,p[0],p[1],&mu1);
   	      p2 = SplitZEdge(box.themax.z,p[0],p[2],&mu2);
   	      uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   	      uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   	      uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   	      uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
   	      AddXYZUV(p1,p2,uv1,uv2);
   	      //face[i].xyzid[k] = face[i].xyzid[k];
   	      face[i].xyzid[(k+1)%3] = nvertices-2;
   	      face[i].xyzid[(k+2)%3] = nvertices-1;
   	      //face[i].uvid[k] = face[i].uvid[k];
   	      face[i].uvid[(k+1)%3] = nuv-2;
   	      face[i].uvid[(k+2)%3] = nuv-1;
   	      break;
   	   }
		}
   }


	// Two vertices on the interior
	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	   p[1] = vertices[face[i].xyzid[(k+1)%3]];
   	   p[2] = vertices[face[i].xyzid[(k+2)%3]];
   	   if (p[0].z < box.themin.z && p[1].z > box.themin.z && p[2].z > box.themin.z) {
   	      p1 = SplitZEdge(box.themin.z,p[0],p[1],&mu1);
   	      p2 = SplitZEdge(box.themin.z,p[0],p[2],&mu2);
   	      uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
   	      uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
   	      uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
   	      uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
   	      AddXYZUV(p1,p2,uv1,uv2);
				face[nf].xyzid[k] = nvertices-2;
   	      face[nf].xyzid[(k+1)%3] = face[i].xyzid[(k+1)%3];
   	      face[nf].xyzid[(k+2)%3] = face[i].xyzid[(k+2)%3];
				face[nf].uvid[k] = nuv-2;
   	      face[nf].uvid[(k+1)%3] = face[i].uvid[(k+1)%3];
   	      face[nf].uvid[(k+2)%3] = face[i].uvid[(k+2)%3];
				face[nf].materialid = face[i].materialid;
				nf++;
	
				face[i].xyzid[k] = nvertices-2;
	         face[i].xyzid[(k+1)%3] = face[i].xyzid[(k+2)%3];
	         face[i].xyzid[(k+2)%3] = nvertices-1;
				face[i].uvid[k] = nuv-2;
	         face[i].uvid[(k+1)%3] = face[i].uvid[(k+2)%3];
	         face[i].uvid[(k+2)%3] = nuv-1;
				//face[i].materialid = face[i].materialid;
				break;
	      }
	   }
	}

	for (i=0;i<nf;i++) {
   	for (k=0;k<3;k++) {
   	   p[0] = vertices[face[i].xyzid[k]];
   	  	p[1] = vertices[face[i].xyzid[(k+1)%3]];
     		p[2] = vertices[face[i].xyzid[(k+2)%3]];
      	if (p[0].z > box.themax.z && p[1].z < box.themax.z && p[2].z < box.themax.z) {
	         p1 = SplitZEdge(box.themax.z,p[0],p[1],&mu1);
  	      	p2 = SplitZEdge(box.themax.z,p[0],p[2],&mu2);
         	uv1.u = uv[face[i].uvid[k]].u + mu1 * (uv[face[i].uvid[(k+1)%3]].u - uv[face[i].uvid[k]].u);
         	uv1.v = uv[face[i].uvid[k]].v + mu1 * (uv[face[i].uvid[(k+1)%3]].v - uv[face[i].uvid[k]].v);
         	uv2.u = uv[face[i].uvid[k]].u + mu2 * (uv[face[i].uvid[(k+2)%3]].u - uv[face[i].uvid[k]].u);
         	uv2.v = uv[face[i].uvid[k]].v + mu2 * (uv[face[i].uvid[(k+2)%3]].v - uv[face[i].uvid[k]].v);
         	AddXYZUV(p1,p2,uv1,uv2);
         	face[nf].xyzid[k] = nvertices-2;
         	face[nf].xyzid[(k+1)%3] = face[i].xyzid[(k+1)%3];
         	face[nf].xyzid[(k+2)%3] = face[i].xyzid[(k+2)%3];
         	face[nf].uvid[k] = nuv-2;
         	face[nf].uvid[(k+1)%3] = face[i].uvid[(k+1)%3];
         	face[nf].uvid[(k+2)%3] = face[i].uvid[(k+2)%3];
         	face[nf].materialid = face[i].materialid;
         	nf++;
	
	         face[i].xyzid[k] = nvertices-2;
	         face[i].xyzid[(k+1)%3] = face[fp].xyzid[(k+2)%3];
	         face[i].xyzid[(k+2)%3] = nvertices-1;
	         face[i].uvid[k] = nuv-2;
	         face[i].uvid[(k+1)%3] = face[fp].uvid[(k+2)%3];
	         face[i].uvid[(k+2)%3] = nuv-1;
	         //face[i].materialid = face[fp].materialid;
	         break;
	      }
		}
   } 

	return(nf);
}

XYZ SplitXEdge(double x,XYZ p1,XYZ p2,double *mu) 
{
	XYZ p;

	if (fabs(p2.x-p1.x) < EPS) {
		*mu = 0;
		fprintf(stderr,"Warning: No x variation in edge, should not happen!\n");
	} else {
		*mu = (x - p1.x) / (p2.x - p1.x);
	}
	p.x = x;
	p.y = p1.y + (*mu) * (p2.y - p1.y);
	p.z = p1.z + (*mu) * (p2.z - p1.z);
	
	return(p);
}

XYZ SplitYEdge(double y,XYZ p1,XYZ p2,double *mu)
{
   XYZ p;

   if (fabs(p2.y-p1.y) < EPS) {
      *mu = 0;
		fprintf(stderr,"Warning: No y variation in edge, should not happen!\n");
   } else {
      *mu = (y - p1.y) / (p2.y - p1.y);
	}
   p.x = p1.x + (*mu) * (p2.x - p1.x);
   p.y = y;
   p.z = p1.z + (*mu) * (p2.z - p1.z);
  
   return(p);
}

XYZ SplitZEdge(double z,XYZ p1,XYZ p2,double *mu)
{
   XYZ p;

   if (fabs(p2.z-p1.z) < EPS) {
      *mu = 0;
		fprintf(stderr,"Warning: No z variation in edge, should not happen!\n");
   } else {
      *mu = (z - p1.z) / (p2.z - p1.z);
	}
   p.x = p1.x + (*mu) * (p2.x - p1.x);
   p.y = p1.y + (*mu) * (p2.y - p1.y);
   p.z = z;
  
   return(p);
}

void AddXYZUV(XYZ p1,XYZ p2,UV uv1,UV uv2)
{
   vertices = realloc(vertices,(nvertices+2)*sizeof(XYZ));
   vertices[nvertices].x = p1.x;
   vertices[nvertices].y = p1.y;
   vertices[nvertices].z = p1.z;
   vertices[nvertices].flag = -1;
   vertices[nvertices+1].x = p2.x;
   vertices[nvertices+1].y = p2.y;
   vertices[nvertices+1].z = p2.z;
   vertices[nvertices+1].flag = -1;
   nvertices += 2;

   uv = realloc(uv,(nuv+2)*sizeof(UV));
   uv[nuv].u = uv1.u;
   uv[nuv].v = uv1.v;
   uv[nuv].flag = -1;
   uv[nuv+1].u = uv2.u;
   uv[nuv+1].v = uv2.v;
   uv[nuv+1].flag = -1;
   nuv += 2;
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
	Split the model into params.depth boxes along each axis
*/
void GenerateBBoxes(void)
{
   int ix,iy,iz,i,n;

   if (params.verbose)
      fprintf(stderr,"Computing the boxes\n");

	for (ix=0;ix<params.depth;ix++) {
		for (iy=0;iy<params.depth;iy++) {
			for (iz=0;iz<params.depth;iz++) {

      		bboxes = realloc(bboxes,(nbboxes+1)*sizeof(BBOX));
      		bboxes[nbboxes].themin.x = themin.x + ix * (themax.x - themin.x) / (double)params.depth;
            bboxes[nbboxes].themin.y = themin.y + iy * (themax.y - themin.y) / (double)params.depth;
            bboxes[nbboxes].themin.z = themin.z + iz * (themax.z - themin.z) / (double)params.depth;
      		bboxes[nbboxes].themax.x = themin.x + (ix+1) * (themax.x - themin.x) / (double)params.depth;
            bboxes[nbboxes].themax.y = themin.y + (iy+1) * (themax.y - themin.y) / (double)params.depth;
            bboxes[nbboxes].themax.z = themin.z + (iz+1) * (themax.z - themin.z) / (double)params.depth;
      		bboxes[nbboxes].vertexcount = 0;
      		bboxes[nbboxes].facecount = 0;
				bboxes[nbboxes].ix = ix;
            bboxes[nbboxes].iy = iy;
            bboxes[nbboxes].iz = iz;
			
				// Count the number of vertices in the box
		      bboxes[nbboxes].vertexcount = 0;
   		   for (i=0;i<nvertices;i++) {
      		   if (VertexInBox(vertices[i],bboxes[nbboxes]) > 0) 
           			bboxes[nbboxes].vertexcount++;
      		}

				// Don't create empty boxes
				if (bboxes[nbboxes].vertexcount <= 0)
					continue;

      		nbboxes++;
			}
		}
	}

   if (params.verbose) {
      for (n=0;n<nbboxes;n++) {
         fprintf(stderr,"   Box %3d (%03d,%03d,%03d) has %10ld vertices\n",
				n+1,bboxes[n].ix,bboxes[n].iy,bboxes[n].iz,bboxes[n].vertexcount);
		}
	}

   return;
}

/*
   Return true if any part of a face is within a box
	Two tests
	- is any vertex in the box
	- is a corner of the box in the triangle
*/
int FaceInBox(FACE f,BBOX box)
{
	int i;
	XYZ p[3];

	for (i=0;i<3;i++)
		p[i] = vertices[f.xyzid[i]];

   // First most common test, are all vertices outside
   if (p[0].x < box.themin.x && p[1].x < box.themin.x && p[2].x < box.themin.x)
      return(FALSE);
   if (p[0].x > box.themax.x && p[1].x > box.themax.x && p[2].x > box.themax.x)
      return(FALSE);
   if (p[0].y < box.themin.y && p[1].y < box.themin.y && p[2].y < box.themin.y)
      return(FALSE);
   if (p[0].y > box.themax.y && p[1].y > box.themax.y && p[2].y > box.themax.y)
      return(FALSE);
   if (p[0].z < box.themin.z && p[1].z < box.themin.z && p[2].z < box.themin.z)
      return(FALSE);
   if (p[0].z > box.themax.z && p[1].z > box.themax.z && p[2].z > box.themax.z)
      return(FALSE);

	/* Next test, is any single vertex on the inside?
	for (i=0;i<3;i++) {
		if (VertexInBox(p[i],box))
			return(TRUE);
	}*/

   return(TRUE);
}

/*
   Update bounds
*/
void MinMaxXYZ(XYZ p,XYZ *min,XYZ *max)
{
   min->x = MIN(min->x,p.x); 
   min->y = MIN(min->y,p.y); 
   min->z = MIN(min->z,p.z);
   max->x = MAX(max->x,p.x); 
   max->y = MAX(max->y,p.y); 
   max->z = MAX(max->z,p.z);
}


/*
   Create the output file name for the obj files
*/
void FormFileName(char *outname,char *inname,int ix,int iy,int iz,int depth)
{
/*
   strcpy(tmp,inname);
   for (i=strlen(tmp)-1;i>0;i--) {
      if (tmp[i] == '.') {
         tmp[i] = '\0';
         break;
      }
   }
   sprintf(outname,"%s_%03d_%03d_%03d",tmp,ix,iy,iz);
*/
	if (depth == 1) 
		sprintf(outname,"%1d_%1d_%1d",ix,iy,iz);
	else if (depth == 2)
		sprintf(outname,"%02d_%02d_%02d",ix,iy,iz);
	else if (depth == 4)
		sprintf(outname,"%03d_%03d_%03d",ix,iy,iz);
	else if (depth == 4)
		sprintf(outname,"%04d_%04d_%04d",ix,iy,iz);
	else
		sprintf(outname,"%05d_%05d_%05d",ix,iy,iz);
}

void Init(void)
{
	params.verbose = FALSE;
	params.trisplit = TRUE;   // Whether to split triangles at boundaries
	params.depth = 2;         // The power of two depth to split along each axis

   // Create a default material
	// Not really necessary but makes it easier to handle a bad OBJ without one
   materials = realloc(materials,sizeof(MATERIALNAME));
   sprintf(materials[0].name,"unknown");
   nmaterials = 1;
}

void SaveBBoxes(void)
{
	int i;
	double r;
	XYZ p = {0,0,0};
	FILE *fobj,*fmtl;
	COLOUR colour = {0,0,0};

   ObjVertex(NULL,p,p);
   AddColour(NULL,colour);
   fobj = fopen("splittingboxes.obj","w");
   fprintf(fobj,"mtllib splittingboxes.mtl\n");
   fmtl = fopen("splittingboxes.mtl","w");
   for (i=0;i<nbboxes;i++) {
      colour.r = i / (nbboxes-1.0);
      colour.g = 0;
      colour.b = 1 - colour.r;
      r = VectorLength(bboxes[i].themin,bboxes[i].themax)/400.0;
      ObjBBox(fobj,fmtl,bboxes[i].themin,bboxes[i].themax,r,16,colour);
   }
   fclose(fobj);
   fclose(fmtl);
}

#include "objlib.c"

