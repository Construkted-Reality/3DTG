#include "oversight.h"

/*
	Take the output from gridsplit and create the upstream and downstream tiles
	Uses system() calls to MeshLab, convert (imagemagick), retexture and gridsplit
	Expects MeshLab mlx scripts decimate.mlx and flatten.mlx
	For filename conventions see gridsplit. X_NNNNN
	Where X is the depth and NNNNN is the cell numbers
	Cell numbers will range from 0 to depth^3
*/

int MAXDEPTH = 3;        // Maximum supported at time of writing, 8x8x8 grid
int startlevel = 0;      // The depth used when running gridsplit
int maxtricount = 50000; // Hardwired for now, should match decimate.mlx
int maxtsize = 2048;     // Maximum texture size, scale if greater
int verbose = FALSE;

int main(int argc,char **argv)
{
	int i;
	int deepest = 0;

	if (argc < 2) {
		GiveUsage(argv[0]);
		exit(-1);
	}
	startlevel = atoi(argv[argc-1]);

	for (i=1;i<argc-1;i++) {
		if (strcmp(argv[i],"-v") == 0)
			verbose = TRUE;
	}

	UpStream();
	deepest = DownStream();

	CreateObjView(deepest);
}

/*
	Upstream processing involves taking obj files from gridsplit and repeatedly
	merging and decimating from the current depth level to level 0
	Currently supports up to depths of 8, needs to match gridsplit
*/
void UpStream(void)
{
	int i,j,k,ix,iy,iz,d,depth;
	int found = 0;
	char srcname[256],dstname[256];
	char filelist[1024],cmd[1024];

	// Decimate each file first
	depth = startlevel;
	d = 1 << depth;
   do {
		for (ix=0;ix<d;ix++) {
   	   for (iy=0;iy<d;iy++) {
   	      for (iz=0;iz<d;iz++) {

					// Candidate source file
					FormFileName(srcname,ix,iy,iz,depth);
					strcat(srcname,".obj");
					if (!FileExists(srcname))
						continue;
	
					// Destination filename
	            FormFileName(dstname,ix,iy,iz,depth);
	            strcat(dstname,"_d.obj");
	
					// Issue decimate command
					sprintf(cmd,"%s -i %s -o %s -m wt -s decimate.mlx",MESHLABPATH,srcname,dstname);
					fprintf(stderr,"Decimating file \"%s\"\n",srcname);
					IssueCmd(cmd);

               // Retexture decimated file
               sprintf(cmd,"retexture -v -d %s",dstname);
               fprintf(stderr,"Retexturing file \"%s\"\n",dstname);
               IssueCmd(cmd);

					// Check image size and possibly scale
					FormFileName(srcname,ix,iy,iz,depth);
               strcat(srcname,"_d.jpg");
					ScaleTexture(srcname);

				}
			}
		}
		d /= 2;
		depth--;
	} while (d > 0);

	// Repeatedly merge and decimate the files created above
	depth = startlevel;
	d = 1 << depth;
   do {
      for (ix=0;ix<d;ix+=2) {
         for (iy=0;iy<d;iy+=2) {
            for (iz=0;iz<d;iz+=2) {

					found = 0;
					filelist[0] = '\0';
               for (i=0;i<2;i++) {
                  for (j=0;j<2;j++) {
                     for (k=0;k<2;k++) {
                        FormFileName(srcname,ix+i,iy+j,iz+k,depth);
								strcat(srcname,"_d.obj");
                        if (FileExists(srcname)) {
                           strcat(filelist,srcname);
									strcat(filelist," ");
									found++;
                        }
                     }
                  }
               }
					if (found <= 0)
						continue;

               FormFileName(dstname,ix/2,iy/2,iz/2,depth-1);
               strcat(dstname,"_tmp.obj"); 
               sprintf(cmd,"%s -i %s -o %s -m wt -s flatten.mlx\n",MESHLABPATH,filelist,dstname);
					fprintf(stderr,"Merging %d files\n",found);
					IssueCmd(cmd);

					// Decimate the new file
					strcpy(srcname,dstname);
               FormFileName(dstname,ix/2,iy/2,iz/2,depth-1);
               strcat(dstname,"_d.obj");
	            sprintf(cmd,"%s -i %s -o %s -m wt -s decimate.mlx",MESHLABPATH,srcname,dstname);
   	         fprintf(stderr,"Decimating file \"%s\"\n",srcname);
      	      IssueCmd(cmd);

					// Retexture new file
					sprintf(cmd,"retexture -v -d %s",dstname);
               fprintf(stderr,"Retexturing file \"%s\"\n",dstname);
               IssueCmd(cmd);

               // Check image size and possibly scale
               FormFileName(srcname,ix/2,iy/2,iz/2,depth-1);
               strcat(srcname,"_d.jpg");
               ScaleTexture(srcname);
            }
         }
      } 

      d /= 2;
		depth--;
   } while (d > 1);
	
	// Remove "_tmp" files
	IssueCmd("rm -rf *_tmp.obj");
	IssueCmd("rm -rf *_tmp.obj.mtl");
}

/*
	Downstream tiles
*/
int DownStream(void)
{
	int d,depth,nface,index;
	int found = FALSE;
	int xdepth=1,ydepth=1,zdepth=1; // 2x2x2 default splitting
	char srcname[256],dstname[256];
	char cmd[1204];
	XYZ themin,themax;
	double dx,dy,dz;
	int splitcount = 0;

   depth = startlevel;

	do {
		found = FALSE;
		splitcount = 0;
   	d = 1 << depth;
   	for (index=0;index<d*d*d;index++) {
	
			// Current file
			FormFileName2(srcname,index,depth);
	      strcat(srcname,".obj");
			if (!FileExists(srcname))
				continue;
	
			// Get any stats we need, numbers of faces and bounding box
			nface = ObjStats(srcname,&themin,&themax);
			if (nface < maxtricount)
				continue;
			found = TRUE;
	
			// Determine what type of split
			xdepth = 1;
			ydepth = 1;
			zdepth = 1;
			dx = themax.x - themin.x;
	      dy = themax.y - themin.y;
	      dz = themax.z - themin.z;
			if (dz < dx/2 && dz < dy/2)
				zdepth = 0;
	      if (dy < dz/2 && dy < dx/2)
	         ydepth = 0;
	      if (dx < dy/2 && dx < dz/2)
	         xdepth = 0;
	
			// Issue split command, note -l resets the level
			sprintf(cmd,"gridsplit -v -d -l %d -m %d -n %d %d %d %s",
				depth+1,splitcount,xdepth,ydepth,zdepth,srcname);
			IssueCmd(cmd);
			splitcount += 8; // Maximum if zdepth=ydepth=xdepth=1
		}
	
		// Decimate all tiles at this new level
		depth++;
		d = 1 << depth;
		for (index=0;index<d*d*d;index++) {
	
	      // Candidate source file
	      FormFileName2(srcname,index,depth);
	      strcat(srcname,".obj");
	      if (!FileExists(srcname))
	         continue;
	
	      // Destination filename
	      FormFileName2(dstname,index,depth);
	      strcat(dstname,"_d.obj");
	
	      // Issue decimate command
	      sprintf(cmd,"%s -i %s -o %s -m wt -s decimate.mlx",MESHLABPATH,srcname,dstname);
	      fprintf(stderr,"Decimating file \"%s\"\n",srcname);
	      IssueCmd(cmd);
	
	      // Retexture decimated file
	      sprintf(cmd,"retexture -v -d %s",dstname);
	      fprintf(stderr,"Retexturing file \"%s\"\n",dstname);
	      IssueCmd(cmd);
	
	      // Check image size and possibly scale
	      FormFileName2(srcname,index,depth);
	      strcat(srcname,"_d.jpg");
	      ScaleTexture(srcname);
		}
	} while (found);
	
	return(depth);
}

/*
	Cound the number of faces in an objfile, approximnately
*/
int ObjStats(char *fname,XYZ *themin,XYZ *themax)
{
	int nface = 0,linecount = 0;
	char ignore[256],aline[256];
	FILE *fptr;
	XYZ p,small = {1e32,1e32,1e32},big = {-1e32,-1e32,-1e32};

	fptr = fopen(fname,"r");
	while (fgets(aline,250,fptr) != NULL) {
		if (aline[0] == 'f' && aline[1] == ' ') {
			nface++;
		}
		if (aline[0] == 'v' && aline[1] == ' ') {
			if (sscanf(aline,"%s %f %f %f",ignore,&p.x,&p.y,&p.z) != 4) {
				fprintf(stderr,"Unexpected bad vertex line \"%s\"\n",aline);
				continue;
			}
			small.x = MIN(small.x,p.x);
         small.y = MIN(small.y,p.y);
         small.z = MIN(small.z,p.z);
         big.x = MAX(big.x,p.x);
         big.y = MAX(big.y,p.y);
         big.z = MAX(big.z,p.z);
		}
		linecount++;
	}
	fclose(fptr);

	*themin = small;
	*themax = big;

	return(nface);
}

/*
   Create the output file name for the obj files
*/
void FormFileName(char *outname,int ix,int iy,int iz,int treelevel)
{
   int n,dx,dy,dz;

   dx = 1 << treelevel;
   dy = 1 << treelevel;
   dz = 1 << treelevel;
   n = iz * dx * dy + iy * dx + ix;

   sprintf(outname,"%d_%05d",treelevel,n);
}
void FormFileName2(char *outname,int n,int treelevel)
{
   sprintf(outname,"%d_%05d",treelevel,n);
}

/*
	Return true of an obj file exists
*/
int FileExists(char *fname)
{
   FILE *fptr;
   char objname[256];

   strcpy(objname,fname);
   if ((fptr = fopen(objname,"r")) == NULL) {
      return(FALSE);
   } else {
      fclose(fptr);
      return(TRUE);
   }
}

/*
	Save all commands to a file, debugging and rerunning perhaps
	Issue through system()
*/
void IssueCmd(char *cmd)
{
	static FILE *logfptr = NULL;
	FILE *pptr;

	// Create the command log file the first time
	if (logfptr == NULL) 
		logfptr = fopen("commands.log","w");
	fprintf(logfptr,"%s\n",cmd);
	
	// Display if in verbose mode
	if (verbose)
      fprintf(stderr,"%s\n",cmd);

	// Issue the command
	if (strstr(cmd,"meshlabserver") != NULL) {
		pptr = popen(cmd,"r");
		pclose(pptr);
	} else {
		system(cmd);
	}
}

/*
	Optionally scale the texture image
*/
void ScaleTexture(char *srcname)
{
	int width,height,bitdepth;
	FILE *fptr;
	char cmd[1024];

   if ((fptr = fopen(srcname,"r")) == NULL) {
      fprintf(stderr,"Unexpectedly failed to open texture file \"%s\"\n",srcname);
      return;
   }
   JPEG_Info(fptr,&width,&height,&bitdepth);
   fclose(fptr);
   if (width > maxtsize || height > maxtsize) {
      sprintf(cmd,"%s %s -resize %dx%d %s",
      CONVERTPATH,srcname,maxtsize,maxtsize,srcname);
      IssueCmd(cmd);
   }
}

void GiveUsage(char *s)
{
	fprintf(stderr,"Usage: %s [options] startlevel\n",s);
	fprintf(stderr,"Options\n");
	fprintf(stderr,"   -v      Verbose mode, default: off\n");
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

void CreateObjView(int depth)
{
	int index,d,nface,first = TRUE;
	FILE *fobj,*fmtl;
	COLOUR c = {0.75,0.25,0.25};
	char srcname[256];
	double r;
	XYZ themin,themax;

	fobj = fopen("tileboxes.obj","w");
   fprintf(fobj,"mtllib tileboxes.mtl\n");
   fmtl = fopen("tileboxes.mtl","w");

   do {
      d = 1 << depth;
      for (index=0;index<d*d*d;index++) {

         FormFileName2(srcname,index,depth);
         strcat(srcname,"_d.obj");
         if (!FileExists(srcname))
            continue;

         // Get any stats we need, numbers of faces and bounding box
         nface = ObjStats(srcname,&themin,&themax);

         if (first) {
            r = VectorLength(themin,themax)/100.0;
            first = FALSE;
         }

			ObjBBox(fobj,fmtl,themin,themax,r,16,c);
		}

		depth--;
	} while (depth >= 0);

   fclose(fobj);
   fclose(fmtl);
}

#include "objlib.c"

