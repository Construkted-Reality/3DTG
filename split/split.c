#include "split.h"

#define VERSION "1.07"

/*
   Take a textured OBJ file and split into pieces each with a maximum number of triangles.
   Overview
   - Subdivide the bounding box until each box contains less than a maximum number of vertices
   - Parse obj files faces, saving each to a OBJ file depending on which box it is inside
   Initial version: vertices in RAM, obj files parsed during analysis of faces
	 3 June 2020: Use the center of mass for the subdivision rather than midpoint
    4 June 2020: switched to double precision for vertices and uv coordinates
    7 June 2020: Add option to extend the splitting box volumns, -g
   12 June 2020: Added -ram option to read faces into memory
                 General code cleanup
   13 June 2020: Added version number and some progress reporting. Made -ram the default.
   17 June 2002: Added timing and investigate vertex reading time.
	08 July 2020: Added option for triangle spitting (-t)
                 Removed disk based solution
   09 July 2020: Fixed triangle splitting error at box corners.
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
      if (strcmp(argv[i],"-n") == 0) 
         params.ntriangles = atoi(argv[i+1]);
      if (strcmp(argv[i],"-s") == 0)
         params.splitmethod = BISECTION;
      if (strcmp(argv[i],"-t") == 0)
         params.trisplit = TRUE;
      if (strcmp(argv[i],"-g") == 0)
         params.growlength = atof(argv[i+1]);
   }
   strcpy(objname,argv[argc-1]);

   // Triangles specified as thousands
   params.ntriangles *= 1000;

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
   while (GenerateBBoxes(params.ntriangles/2)) 
      ;
   stoptime = GetRunTime();
   if (params.verbose)
      fprintf(stderr,"Time create boxes: %.1lf seconds\n",stoptime-starttime);

   // Create an obj file showing the bounding boxes, debugging purposes only
   if (params.verbose) 
		SaveSplitting();

   // Read the faces and allocate to correct subvolume
	starttime = GetRunTime();
   ProcessFaces(objname);
   stoptime = GetRunTime();
   if (params.verbose)
      fprintf(stderr,"Time process geometry: %.1lf seconds\n",stoptime-starttime);

   // Create an obj file showing the bounding boxes, debugging purposes only
   if (params.verbose) 
		SaveBBoxes();

   // Create the meta tileset
   SaveTileset(objname);

   exit(0);
}

void GiveUsage(char *s)
{
   fprintf(stderr,"Usage: %s [options] objfilename\n",s);
	fprintf(stderr,"Version: %s\n",VERSION);
   fprintf(stderr,"Options\n");
   fprintf(stderr,"   -v     verbose mode, default: disabled\n");
   fprintf(stderr,"   -g n   grow the splitting boxes by this amount, default: 0\n");
   fprintf(stderr,"   -s     switch to bisection method to split boxes, default: center of mass\n");
	fprintf(stderr,"   -t     whether to split triangles at boundary, default: off\n");
   fprintf(stderr,"   -n n   triangle target per box in thousands, default: %ld\n",params.ntriangles);

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
   int i,j,n,k;
   int xyzcount,uvcount;
	int currentmaterial = -1;
   char objname[256];
   FILE *fobj;
   XYZ p;
   XYZ big = {1e32,1e32,1e32},small = {-1e32,-1e32,-1e32};
   UV u;
   FACE *aface = NULL;
	int nf = 0;
	int maxface = 50; // Actually I think 16 is the maximum possible

	aface = malloc(maxface*sizeof(FACE));

   // Process each box in turn
   for (n=0;n<nbboxes;n++) {
		currentmaterial = -1;
      if (params.verbose)
         fprintf(stderr,"Processing faces for box %10d of %10d\n",n+1,nbboxes);

		// Will determine the actual bounding box since faces can extend
		// outside of the splitting box if triangle splitting is off
      bboxes[n].datamin = big;
      bboxes[n].datamax = small;
      bboxes[n].facecount = 0;

      // Create the nth output obj file
      FormFileName(objname,fname,n);
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
      fprintf(fobj,"# Section %d of %d sections\n",n+1,nbboxes);
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
			if (i % (nfaces/10) == 0)
				fprintf(stderr,"   Processing face %d of %ld\n",i,nfaces);

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

	         for (j=0;j<3;j++) 
	            MinMaxXYZ(vertices[aface[k].xyzid[j]],&bboxes[n].datamin,&bboxes[n].datamax);

   	      bboxes[n].facecount++;
			} // k of nf split faces
		}
	
      // Report on the data bounds and centroid
      fprintf(fobj,"#\n");
      fprintf(fobj,"# Faces X bounds: %lf <= x <= %lf, length: %lf, halflength: %lf\n",
         bboxes[n].datamin.x,bboxes[n].datamax.x,bboxes[n].datamax.x-bboxes[n].datamin.x,
         (bboxes[n].datamax.x-bboxes[n].datamin.x)/2);
      fprintf(fobj,"# Faces Y bounds: %lf <= y <= %lf, length: %lf, halflength: %lf\n",
         bboxes[n].datamin.y,bboxes[n].datamax.y,bboxes[n].datamax.y-bboxes[n].datamin.y,
         (bboxes[n].datamax.y-bboxes[n].datamin.y)/2);
      fprintf(fobj,"# Faces Z bounds: %lf <= z <= %lf, length: %lf, halflength: %lf\n",
         bboxes[n].datamin.z,bboxes[n].datamax.z,bboxes[n].datamax.z-bboxes[n].datamin.z,
         (bboxes[n].datamax.z-bboxes[n].datamin.z)/2);
      fprintf(fobj,"# Faces center: %lf %lf %lf\n",
         (bboxes[n].datamin.x+bboxes[n].datamax.x)/2,
         (bboxes[n].datamin.y+bboxes[n].datamax.y)/2,
         (bboxes[n].datamin.z+bboxes[n].datamax.z)/2);
      fprintf(fobj,"# Faces center (ION): %lf %lf %lf\n",
         (bboxes[n].datamin.x+bboxes[n].datamax.x)/2,
         (bboxes[n].datamin.y+bboxes[n].datamax.y)/2,
         bboxes[n].datamin.z);
      fprintf(fobj,"# Total faces: %ld\n",bboxes[n].facecount);
      fprintf(fobj,"#\n");
      fclose(fobj);

	   if (params.verbose)
   	   fprintf(stderr,"   Contains %ld faces\n",bboxes[n].facecount);
   } // n

   return(TRUE);
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
   Create the bounding boxes with no more than maxtarget vertices
   This is faster than considering faces
   In general a connected model will have have twice the number of vertices as faces
   Return true if any existing bounding box was split
   Lots of ways of making this more efficient!
*/
int GenerateBBoxes(int maxtarget)
{
   int i,n,noriginal;
   int wassplit = FALSE;
   XYZ length,midp,zero = {0,0,0};
   dXYZ sum,dzero = {0,0,0}; // Double precision, adding LOTS of numbers

   if (params.verbose)
      fprintf(stderr,"Computing the bounding boxes\n");

   // At the start: Set a single box equal to the bounding box of the entire model
   if (nbboxes == 0) {
      bboxes = realloc(bboxes,sizeof(BBOX));
      bboxes[0].themin = themin;
      bboxes[0].themax = themax;
      bboxes[0].vertexcount = 0;
      bboxes[0].centerofmass = zero;
      bboxes[0].facecount = 0;
      nbboxes = 1;
   }

   // Count the number of vertices in each bounding box
   // Also calculate center of mass as the average of all vertices
   for (n=0;n<nbboxes;n++) {
      bboxes[n].vertexcount = 0;
      sum = dzero;
      for (i=0;i<nvertices;i++) {
         if (VertexInBox(vertices[i],bboxes[n]) > 0) {
            bboxes[n].vertexcount++;
            sum.x += vertices[i].x;
            sum.y += vertices[i].y;
            sum.z += vertices[i].z;
         }
      }
      if (bboxes[n].vertexcount > 1) {
         bboxes[n].centerofmass.x = sum.x / bboxes[n].vertexcount;
         bboxes[n].centerofmass.y = sum.y / bboxes[n].vertexcount;
         bboxes[n].centerofmass.z = sum.z / bboxes[n].vertexcount;
      }
   }
   if (params.verbose) {
      for (n=0;n<nbboxes;n++) {
         fprintf(stderr,"   Box %3d has %10ld vertices",n+1,bboxes[n].vertexcount);
			if (bboxes[n].vertexcount > maxtarget)
				fprintf(stderr,", it will be split on next iteration.");
			fprintf(stderr,"\n");
		}
	}

   // Do any of the current boxes need splitting?
   // Split any box along the longest side length
   noriginal = nbboxes;
   for (n=0;n<noriginal;n++) {
      if (bboxes[n].vertexcount > maxtarget) {

         // Create a new box
         bboxes = realloc(bboxes,(nbboxes+1)*sizeof(BBOX));
         bboxes[nbboxes].themin = bboxes[n].themin;
         bboxes[nbboxes].themax = bboxes[n].themax;

         // Determine the lengths along each axis
         length.x = bboxes[n].themax.x - bboxes[n].themin.x;
         length.y = bboxes[n].themax.y - bboxes[n].themin.y;
         length.z = bboxes[n].themax.z - bboxes[n].themin.z;

         // Determine the splitting point
			// Generally imagined that center of mass is better
         if (params.splitmethod == BISECTION) 
            midp = MidPoint(bboxes[n].themin,bboxes[n].themax);
         else // Center of mass
            midp = bboxes[n].centerofmass;

         // Split along the longest axis of the box
         if (length.x >= length.y && length.x >= length.z) {
            bboxes[nbboxes].themin.x = midp.x;
            bboxes[n].themax.x       = midp.x;
         } else if (length.y >= length.x && length.y >= length.z) {
            bboxes[nbboxes].themin.y = midp.y;
            bboxes[n].themax.y       = midp.y;
         } else {
            bboxes[nbboxes].themin.z = midp.z;
            bboxes[n].themax.z       = midp.z;
         }

         nbboxes++;
         wassplit = TRUE;
      }
   }
   
   // Optionally extend all the dimensions of the splitting boxes, see -g option
   // Could result in target triangle count being exceeded.
	// Only do once all splitting is complete
   if (!wassplit && params.growlength > 0) {
      for (n=0;n<nbboxes;n++) {
         bboxes[n].themin.x -= params.growlength;
         bboxes[n].themin.y -= params.growlength;
         bboxes[n].themin.z -= params.growlength;
         bboxes[n].themax.x += params.growlength;
         bboxes[n].themax.y += params.growlength;
         bboxes[n].themax.z += params.growlength;
      }
   }

   return(wassplit);
}

/*
   Return true if a vertex is within a box
*/
int VertexInBox(XYZ p,BBOX box)
{
   if (p.x < box.themin.x || p.x > box.themax.x)
      return(0);
   if (p.y < box.themin.y || p.y > box.themax.y)
      return(0);
   if (p.z < box.themin.z || p.z > box.themax.z)
      return(0);
   return(1);
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
   Return the midpoint between two vertices
*/
XYZ MidPoint(XYZ p1,XYZ p2)
{
   XYZ p;

   p.x = (p1.x + p2.x) / 2;
   p.y = (p1.y + p2.y) / 2;
   p.z = (p1.z + p2.z) / 2;

   return(p);
}

/*
   Calculate the unit normal at p given two other points
   p1,p2 on the surface. The normal points in the direction
   of p1-p crossproduct p2-p
*/
XYZ CalcNormal(XYZ p,XYZ p1,XYZ p2)
{
   XYZ n,pa,pb;

   pa.x = p1.x - p.x;
   pa.y = p1.y - p.y;
   pa.z = p1.z - p.z;
   pb.x = p2.x - p.x;
   pb.y = p2.y - p.y;
   pb.z = p2.z - p.z;
   n.x = pa.y * pb.z - pa.z * pb.y;
   n.y = pa.z * pb.x - pa.x * pb.z;
   n.z = pa.x * pb.y - pa.y * pb.x;
   Normalise(&n);

   return(n);
}

/*
   Normalise a vector
*/
void Normalise(XYZ *p)
{
   double length;

   length = p->x * p->x + p->y * p->y + p->z * p->z;
   if (length > 0) {
      length = sqrt(length);
      p->x /= length;
      p->y /= length;
      p->z /= length;
   } else {
      p->x = 0;
      p->y = 0;
      p->z = 0;
   }
}

/*
   Return the distance between two points
*/
double VectorLength(XYZ p1,XYZ p2)
{
   XYZ d;

   d.x = p1.x - p2.x;
   d.y = p1.y - p2.y;
   d.z = p1.z - p2.z;

   return(sqrt(d.x*d.x + d.y*d.y + d.z*d.z));
}

/*
   Create the oupput file name for the obj files
   Strip extension and add box number
*/
void FormFileName(char *outname,char *inname,int n)
{
   int i;
   char tmp[256];

   strcpy(tmp,inname);
   for (i=strlen(tmp)-1;i>0;i--) {
      if (tmp[i] == '.') {
         tmp[i] = '\0';
         break;
      }
   }
   sprintf(outname,"%s_%02d.obj",tmp,n);
}

/*
   Remove any non printing characters
*/
void CleanString(char *s)
{
   int i;

   for (i=0;i<strlen(s);i++) {
      if (s[i] < ' ' || s[i] > '~')
         s[i] = ' ';   
   }
}

/*
   Save a meta tileset
*/
void SaveTileset(char *fname)
{
   int i,n;
   char basename[256];
   FILE *fout;
   char outname[300];
   double r;

   strcpy(basename,fname);
   for (i=strlen(basename)-1;i>0;i--) {
      if (basename[i] == '.') {
         basename[i] = '\0';
         break;
      }
   }

   sprintf(outname,"%s.json",basename);
   if (params.verbose)
      fprintf(stderr,"Saving meta json file \"%s\"\n",outname);
   if ((fout = fopen(outname,"w")) == NULL) {
      fprintf(stderr,"Failed to open output json file \"%s\"\n",outname);
      exit(-1);
   }

   // Header
   fprintf(fout,"{\n");
   fprintf(fout,"  \"asset\":{\n");
   fprintf(fout,"    \"version\":\"1.0\"\n");
   fprintf(fout,"  },\n");
   fprintf(fout,"  \"geometricError\":25.0,\n");  // why not "0"?

   // Root
   fprintf(fout,"  \"root\":{\n");
   fprintf(fout,"    \"refine\":\"ADD\",\n");  // ADD or REPLACE
   fprintf(fout,"    \"geometricError\":25.0,\n");
   fprintf(fout,"    \"boundingVolume\":{\n");
   fprintf(fout,"    \"sphere\":[");
   r = VectorLength(themin,themax)/2;
   fprintf(fout,"%lf,%lf,%lf,%lf]\n",themid.x,themid.y,themid.z,r);
   fprintf(fout,"    },\n");

   // Each of the children
   fprintf(fout,"    \"children\":[\n");
   for (n=0;n<nbboxes;n++) {
      fprintf(fout,"      {\n");
      fprintf(fout,"        \"refine\":\"REPLACE\",\n");
      fprintf(fout,"        \"geometricError\":25.0,\n");
      fprintf(fout,"        \"boundingVolume\":{\n");
      fprintf(fout,"          \"sphere\":[");
      r = VectorLength(bboxes[n].datamin,bboxes[n].datamax)/2;
      fprintf(fout,"%lf,%lf,%lf,%lf]\n",themid.x,themid.y,themid.z,r);
      fprintf(fout,"        },\n");
      fprintf(fout,"        \"transform\":[");
      fprintf(fout,"1.0,0.0,0.0,0.0,");
      fprintf(fout,"0.0,1.0,0.0,0.0,");
      fprintf(fout,"0.0,0.0,1.0,0.0,");
      fprintf(fout,"%lf,%lf,%lf,1.0",
         (bboxes[n].datamin.x+bboxes[n].datamax.x)/2,
         (bboxes[n].datamin.y+bboxes[n].datamax.y)/2,
         bboxes[n].datamin.z);
      fprintf(fout,"],\n");
      fprintf(fout,"        \"content\":{\n");
      fprintf(fout,"          \"uri\":\"%s_%02d/tileset.json\"\n",basename,n);
      fprintf(fout,"        }\n");
      fprintf(fout,"      }");
      if (n < nbboxes-1)
         fprintf(fout,",\n");
      else
         fprintf(fout,"\n");
   }
   fprintf(fout,"    ]\n");

   // Close root
   fprintf(fout,"  }\n");

   // Close header
   fprintf(fout,"}\n");
}

void CopyFace(FACE *source,FACE *destination)
{
	int i;

   for (i=0;i<3;i++) {
      destination->xyzid[i] = source->xyzid[i];
      destination->uvid[i] = source->uvid[i];
   }
	destination->materialid = source->materialid;
}

void Init(void)
{
	params.verbose = FALSE;
	params.splitmethod = CENTEROFMASS; // or BISECTION
	params.trisplit = FALSE;           // Whether to split triangles at boundaries
	params.ntriangles = 1000;          // The maximum number of triangles per box, in thousands
	params.growlength = 0;             // Extend the splitting boxes by this amount

   // Create a default material
	// Not really necessary but makes it easier to handle a bad OBJ without one
   materials = realloc(materials,sizeof(MATERIALNAME));
   sprintf(materials[0].name,"unknown");
   nmaterials = 1;
}

void SaveSplitting(void)
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
      r = VectorLength(bboxes[i].themin,bboxes[i].themax)/500.0;
      ObjBBox(fobj,fmtl,bboxes[i].themin,bboxes[i].themax,r,16,colour);
      p.x = (bboxes[i].themin.x + bboxes[i].themax.x)/2;
      p.y = (bboxes[i].themin.y + bboxes[i].themax.y)/2;
      p.z = (bboxes[i].themin.z + bboxes[i].themax.z)/2;
      ObjSphere(fobj,fmtl,p,5*r,colour,32);
   }
   fclose(fobj);
   fclose(fmtl);
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
   fobj = fopen("boundingboxes.obj","w");
   fprintf(fobj,"mtllib boundingboxes.mtl\n");
   fmtl = fopen("boundingboxes.mtl","w");
   for (i=0;i<nbboxes;i++) {
      colour.r = i / (nbboxes-1.0);
      colour.g = 0;
      colour.b = 1 - colour.r;
      r = VectorLength(bboxes[i].datamin,bboxes[i].datamax)/500.0;
      ObjBBox(fobj,fmtl,bboxes[i].datamin,bboxes[i].datamax,r,16,colour);
      p.x = (bboxes[i].datamin.x + bboxes[i].datamax.x)/2;
      p.y = (bboxes[i].datamin.y + bboxes[i].datamax.y)/2;
      p.z = (bboxes[i].datamin.z + bboxes[i].datamax.z)/2;
      ObjSphere(fobj,fmtl,p,5*r,colour,32);
   }
   fclose(fobj);
   fclose(fmtl);
}

/*
   Line progress reports
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

double GetRunTime(void)
{
   double sec = 0;
   struct timeval tp;

   gettimeofday(&tp,NULL);
   sec = tp.tv_sec + tp.tv_usec / 1000000.0;

   return(sec);
}

#include "objlib.c"

