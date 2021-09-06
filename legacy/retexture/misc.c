#include "retexture.h"

/*
	The following are general functions, not specifically part of the algorithm
*/

// isLeft(): tests if a point is Left|On|Right of an infinite line.
// Input:  three points P0, P1, and P2
// Return: >0 for P2 left of the line through P0 and P1
//         =0 for P2  on the line
//         <0 for P2  right of the line
// See: Algorithm 1 "Area of Triangles and Polygons"
int isLeft(POINT p0,POINT p1,POINT p2)
{
   return ( (p1.h - p0.h) * (p2.v - p0.v) - (p2.h - p0.h) * (p1.v - p0.v) );
}

// inside polygon test using winding number test for a point in a polygon
// Input: p = a point,
//        poly[] = vertex points of a polygon poly[n+1] with poly[n]=poly[0]
// Return:  wn = the winding number (=0 only when p is outside)
int InsidePolygon(POINT p,POINT *poly,int n)
{
   int wn = 0; // the winding number counter
	int i,ip1;

   // loop through all edges of the polygon
   for (i=0;i<n;i++) {                              // edge from poly[i] to poly[i+1]
		ip1 = (i+1)%n;
      if (poly[i].v <= p.v) {                       // start y <= p.y
         if (poly[ip1].v > p.v)                     // an upward crossing
            if (isLeft(poly[i],poly[ip1],p) > 0)    // p left of edge
               wn++;                                // have  a valid up intersect
      } else {                                      // start y > p.y (no test needed)
         if (poly[ip1].v <= p.v)                 // a downward crossing
            if (isLeft(poly[i],poly[ip1],p) < 0)    // p right of edge
               wn--;                                // have a valid down intersect
      }
   }
	if (wn != 0)
		return(TRUE);
	else
   	return(FALSE);
}

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
	Find the closest distance to p on a polygon boundary
*/
float ClosestPoint(POINT *polygon,int n,POINT p)
{
   int i;
   int dh,dv,dd,dmin=2100000000;
   POINT closest;

   for (i=0;i<n;i++) {
      dh = p.h - polygon[i].h;
      dv = p.v - polygon[i].v;
      if ((dd = dh*dh + dv*dv) < dmin) {
         dmin = dd;
         closest = polygon[i];
      }
   }

	return(sqrt((float)dd));
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

/*
	Report on running time
*/
void ReportTime(double tstart,char *msg)
{
	int minutes;
	double dt;

   dt = GetRunTime() - tstart;
	minutes = (int)(dt/60);

   if (dt < 10) {
		;
	} else if (dt < 60) {
      fprintf(stderr,"%s: %.1lf seconds\n",msg,dt);
	} else {
		fprintf(stderr,"%s: %d minutes and %.1lf seconds\n",msg,minutes,dt-minutes*60);
	}
}

