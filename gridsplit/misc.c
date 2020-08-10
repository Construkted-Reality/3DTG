#include "gridsplit.h"

/*
	Support functions, not explicitely part of the algorithm
*/

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

/*
   Calculate the closest but lower power of two of a number
   twopm = 2**m <= n
   Return TRUE if 2**m == n
*/
int Powerof2(int n,int *m,int *twopm)
{
   if (n <= 1) {
      *m = 0;
      *twopm = 1;
      return(FALSE);
   }

   *m = 1;
   *twopm = 2;
   do {
      (*m)++;
      (*twopm) *= 2;
   } while (2*(*twopm) <= n);

   if (*twopm != n)
      return(FALSE);
   else
      return(TRUE);
}

int ObjExists(char *fname)
{
   FILE *fptr;
	char objname[256];

	strcpy(objname,fname);
	strcat(objname,".obj");
   if ((fptr = fopen(objname,"r")) == NULL) {
      return(FALSE);
   } else {
      fclose(fptr);
      return(TRUE);
   }
}

