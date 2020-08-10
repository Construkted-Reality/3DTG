#include "retexture.h"

/*
	The following are general functions, not specifically part of the algorithm
*/

void StripExtension(char *s)
{
	int i;

   for (i=strlen(s)-1;i>0;i--) {
      if (s[i] == '.') {
         s[i] = '\0';
         break;
      }
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

/*
   Return normalised direction vector from p1 to p2
*/
POINT CalcDir(POINT p1,POINT p2)
{
   POINT d;

   d.h = p2.h - p1.h;
   if (d.h >= 1)
      d.h = 1;
   if (d.h <= -1)
      d.h = -1;
   d.v = p2.v - p1.v;
   if (d.v >= 1)
      d.v = 1;
   if (d.v <= -1)
      d.v = -1;
   return(d);
}

/*
	Determine if a point is inside a polygon
	If not inside calculate closeness
*/
int InsidePolygon(POINT *polygon,int n,POINT p,float *closest)
{
   int i,dh,dv;
   double angle=0;
	float distmin = 1e32,dist;
   POINT p1,p2;

   for (i=0;i<n;i++) {
      p1.h = polygon[i].h - p.h;
      p1.v = polygon[i].v - p.v;
      p2.h = polygon[(i+1)%n].h - p.h;
      p2.v = polygon[(i+1)%n].v - p.v;
      angle += Angle2D(p1.h,p1.v,p2.h,p2.v);

		dh = p2.h - p1.h;
		dv = p2.v - p1.v;
		dist = dh*dh + dv*dv;
		if (dist < distmin) 
			distmin = dist;
   }

   if (ABS(angle) < PI) {
		*closest = sqrt(distmin);
      return(FALSE);
   } else {
		*closest = 0;
      return(TRUE);
	}
}

/*
   Return the angle between two vectors on a plane
   The angle is from vector 1 to vector 2, positive anticlockwise
   The result is between -pi -> pi
*/
double Angle2D(double x1, double y1, double x2, double y2)
{
   double dtheta,theta1,theta2;

   theta1 = atan2(y1,x1);
   theta2 = atan2(y2,x2);
   dtheta = theta2 - theta1;
   while (dtheta > PI)
      dtheta -= TWOPI;
   while (dtheta < -PI)
      dtheta += TWOPI;

   return(dtheta);
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

/*
	Clip a point to bounds
*/
void PointClip(int width,int height,POINT *p)
{
	if (p->h < 0)
		p->h = 0;
	if (p->v < 0)
		p->v = 0;
	if (p->h >= width)
		p->h = width-1;
	if (p->v >= height)
		p->v = height-1;
}

