#include "retexture.h"

/*
	The following are general functions, not specifically part of the algorithm
*/

double GetRunTime(void)
{
   double sec = 0;
   struct timeval tp;

   gettimeofday(&tp,NULL);
   sec = tp.tv_sec + tp.tv_usec / 1000000.0;

   return(sec);
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
   Calculate how far p3 is from the line from p1 to p2
*/
double PointLine2D(XY p3,XY p1,XY p2,XY *close,double *mu)
{
   double dx,dy,dd;

   dx = p2.x - p1.x;
   dy = p2.y - p1.y;
   dd = dx * dx + dy * dy;

   /* Are the two points p1 and p2 coincident? */
   if (dd < EPS) {
      *mu = 0;
      close->x = p1.x;
      close->y = p1.y;
   } else {
      *mu = ((p3.x - p1.x) * dx + (p3.y - p1.y) * dy) / dd;
      close->x = p1.x + (*mu) * dx;
      close->y = p1.y + (*mu) * dy;
   }
   dx = p3.x - close->x;
   dy = p3.y - close->y;
   return(sqrt(dx * dx + dy * dy));
}

