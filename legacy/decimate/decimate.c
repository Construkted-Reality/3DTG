#include "decimate.h"

#define VERSION "1.00"

/*
	Decimate a OBJ file by quantising vertices onto a 3D grid
	Initially based upon original "gridsplit.c"
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
long nfaces = 0;

// The grid
GRIDCELL ***grid;
int gridnx,gridny,gridnz;

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
		if (strcmp(argv[i],"-r") == 0) {
			params.n = atoi(argv[i+1]);
			if (params.n < 10) {
				fprintf(stderr,"Resolution must be at least 10, resetting\n");
				params.n = 10;
			}
		}
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
	stoptime = GetRunTime();
	if (params.verbose)
		fprintf(stderr,"Time to read OBJ file: %.1lf seconds\n",stoptime-starttime);

	// Create the gridded data
	starttime = GetRunTime();
	GridVertices();
   stoptime = GetRunTime();
   if (params.verbose)
      fprintf(stderr,"Time to grid: %.1lf seconds\n",stoptime-starttime);

   // Process the faces writing output file as we go
	starttime = GetRunTime();
	rewind(fptr);
   ProcessFaces(fptr,objname);
	fclose(fptr);
   stoptime = GetRunTime();
   if (params.verbose)
      fprintf(stderr,"Time process geometry: %.1lf seconds\n",stoptime-starttime);

   exit(0);
}

void GiveUsage(char *s)
{
   fprintf(stderr,"Usage: %s [options] objfilename\n",s);
	fprintf(stderr,"Version: %s\n",VERSION);
   fprintf(stderr,"Options\n");
   fprintf(stderr,"   -v      verbose mode, default: disabled\n");
	fprintf(stderr,"   -r n    initial grid resolution along longest side, default: %d\n",params.n);

   exit(0);
}

/*
   Read all vertices and uv coordinates from OBJ file
	Faces are read later during face processing stage
   Currently ignore vertex colours and normals
*/
int ReadObj(FILE *fptr)
{
   long linecount = 0;
   long ram;
   char aline[1024];
   UV t;
   XYZ p;

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
         nuv++;
         continue;
      }

      // Normals, ignore
      if (aline[0] == 'v' && aline[1] == 'n' && aline[2] == ' ') {
         continue;
      }

      // Don't need to read the faces yet
      if (aline[0] == 'f' && aline[1] == ' ') {
			nfaces++;
			continue;
		}

     	// Materials
     	if (strstr(aline,"mtllib ") != NULL) {
     	   continue;
     	}
     	if (strstr(aline,"usemtl ") != NULL) {
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

   return(TRUE);
}

/*
   Process faces and deal with materials
	Mostly just pass through all material lines, ignore vertices
*/
int ProcessFaces(FILE *fptr,char *basename)
{
	int i;
	long linecount = 0;
	long nsaved = 0;
	char fname[256];
	char aline[1204];
	FACE aface;
	FILE *fobj;

	// Create the output file name
	strcpy(fname,basename);
	for (i=strlen(fname)-1;i>0;i--) {
		if (fname[i] == '.') {
			fname[i] = '\0';
			break;
		}
	}
	strcat(fname,"_d.obj");
	fobj = fopen(fname,"w");

   // Read a line at a time
   // Only interested in vertex and texture coordinate lines
	nfaces = 0;
   while (fgets(aline,1023,fptr) != NULL) {
      linecount++;
      if (params.verbose)
         LineStatus(linecount);

		// Comment line
      if (aline[0] == '#') {
         continue;
      }

      // Vertex line
      if (aline[0] == 'v' && aline[1] == ' ') {
			continue;
		}

      // Texture coordinate line
      if (aline[0] == 'v' && aline[1] == 't' && aline[2] == ' ') {
         continue;
      }

      // Normals, ignore
      if (aline[0] == 'v' && aline[1] == 'n' && aline[2] == ' ') {
         continue;
      }

      // Handle faces
      if (aline[0] == 'f' && aline[1] == ' ') {
         if (!ParseFace(aline,&aface)) {
            fprintf(stderr,"Face parsing error occured on line %ld\n",linecount);
            return(FALSE);
         }
			nsaved += SaveFace(fobj,aface);
         nfaces++;
         continue;
      }

      // Materials
      if (strstr(aline,"mtllib ") != NULL) {
			fprintf(fobj,"%s",aline);
         continue;
      }
      if (strstr(aline,"usemtl ") != NULL) {
			fprintf(fobj,"%s",aline);
         continue;
      }

				
	} // fgets
	fclose(fobj);

	if (params.verbose) {
		fprintf(stderr,"Read %ld faces\n",nfaces);
		fprintf(stderr,"Saved %ld faces\n",nsaved);
	}

   return(TRUE);
}

/*
	Save a face to a file
	The vertices must be in the three different cells
	Update the vertex and UV ids as we go
*/
int SaveFace(FILE *fobj,FACE aface)
{
	int i[3],j[3],k[3],n,id;
	int index[3];
	XYZ p[3],q;

	static int xyzcount = 0;
	static int uvcount = 0;
	
	for (n=0;n<3;n++) {
		p[n] = vertices[aface.xyzid[n]];
		XYZ2grid(p[n],&(i[n]),&(j[n]),&(k[n]));
		index[n] = k[n] * gridnx * gridny + j[n] * gridnx + i[n];
	}
	if (index[0] == index[1] || index[1] == index[2] || index[0] == index[2])
		return(0);

	for (n=0;n<3;n++) {
		id = grid[i[n]][j[n]][k[n]].xyzid;
		q = grid[i[n]][j[n]][k[n]].p;
		if (id >= 0) {
			aface.xyzid[n] = id;
		} else {
			fprintf(fobj,"v %f %f %f\n",q.x,q.y,q.z);
			aface.xyzid[n] = xyzcount;
			grid[i[n]][j[n]][k[n]].xyzid = xyzcount;
			xyzcount++;
		}

		id = grid[i[n]][j[n]][k[n]].uvid;
      if (id >= 0) {
         aface.uvid[n] = id;
      } else {
			id = aface.uvid[n];
         fprintf(fobj,"vt %f %f\n",uv[id].u,uv[id].v);
         aface.uvid[n] = uvcount;
			grid[i[n]][j[n]][k[n]].uvid = uvcount;
         uvcount++;
      }
	}
	fprintf(fobj,"f %d/%d %d/%d %d/%d\n",
		aface.xyzid[0]+1,aface.uvid[0]+1,
		aface.xyzid[1]+1,aface.uvid[1]+1,
		aface.xyzid[2]+1,aface.uvid[2]+1);
	
	return(1);
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

void Init(void)
{
	params.verbose = FALSE;
	params.n = 100;
}

double GetRunTime(void)
{
   double sec = 0;
   struct timeval tp;

   gettimeofday(&tp,NULL);
   sec = tp.tv_sec + tp.tv_usec / 1000000.0;

   return(sec);
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

/*
	Create the grid structure
	Determine the average vertex position in each
*/
void GridVertices(void)
{
	int i,j,k,n;
	double dx,dy,dz;
	XYZ zero = {0,0,0};

	if (params.verbose)
		fprintf(stderr,"Populating the grid\n");

	// Determine the longest side length, and the grid cells on each side
	dx = themax.x - themin.x;
	dy = themax.y - themin.y;
   dz = themax.z - themin.z;
	if (dx > dy && dx > dz) {
		gridnx = params.n;
		gridny = params.n * dy / dx;
		gridnz = params.n * dz / dx;
	} else if (dy > dx && dy > dz) {
      gridnx = params.n * dx / dy;
      gridny = params.n;
      gridnz = params.n * dz / dy;
   } else {
      gridnx = params.n * dx / dz;
      gridny = params.n * dy / dz;
      gridnz = params.n;
	}
	if (gridnx < 1) gridnx = 1;
   if (gridny < 1) gridny = 1;
   if (gridnz < 1) gridnz = 1;
	if (params.verbose)
		fprintf(stderr,"Grid dimensions: %d x %d x %d\n",gridnx,gridny,gridnz);

	// Create the grid
	grid = malloc(gridnx*sizeof(GRIDCELL **));
	for (i=0;i<gridnx;i++)
		grid[i] = malloc(gridny*sizeof(GRIDCELL *));
	for (i=0;i<gridnx;i++)
		for (j=0;j<gridny;j++)
			grid[i][j] = malloc(gridnz*sizeof(GRIDCELL));
   for (i=0;i<gridnx;i++) {
      for (j=0;j<gridny;j++) {
			for (k=0;k<gridnz;k++) {
				grid[i][j][k].p = zero;
				grid[i][j][k].n = 0;
				grid[i][j][k].xyzid = -1; // Not yet assigned
				grid[i][j][k].uvid = -1; // Not yet assigned
			}
		}
	}

	// Compute the average vertex to each grid cell
	if (params.verbose)
		fprintf(stderr,"Filling the grid\n");
	for (n=0;n<nvertices;n++) {
		XYZ2grid(vertices[n],&i,&j,&k);
		grid[i][j][k].p.x += vertices[n].x;
      grid[i][j][k].p.y += vertices[n].y;
      grid[i][j][k].p.z += vertices[n].z;
		grid[i][j][k].n++;
	}
	for (i=0;i<gridnx;i++) {
      for (j=0;j<gridny;j++) {
         for (k=0;k<gridnz;k++) {
				if (grid[i][j][k].n > 1) {
					grid[i][j][k].p.x /= grid[i][j][k].n;
    			   grid[i][j][k].p.y /= grid[i][j][k].n;
      			grid[i][j][k].p.z /= grid[i][j][k].n;
				}
			}
		}
	}

}

/*
	Map a point to a grid cell
*/
void XYZ2grid(XYZ p,int *i,int *j,int *k)
{
	*i = gridnx * (p.x - themin.x) / (themax.x - themin.x);
	*j = gridny * (p.y - themin.y) / (themax.y - themin.y);
	*k = gridnz * (p.z - themin.z) / (themax.z - themin.z);
	if (*i >= gridnx) 
		*i = gridnx-1;
   if (*j >= gridny)
      *j = gridny-1;
   if (*k >= gridnz)
      *k = gridnz-1;
}

