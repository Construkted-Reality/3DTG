
/*
	Libray of random OBJ file function
*/

/*
	Create sphere
*/
void ObjSphere(FILE *fobj,FILE *fmtl,XYZ pc,double r,COLOUR c,int n)
{
   int i,j;
   double theta1,theta2,theta3;
   XYZ e[4],p[4];

   if (r < 0)
      r = -r;
   if (n < 0)
      n = -n;
	if (n < 4)
		n = 4;

   // Make n even 
   n /= 2;
   n *= 2;

   for (j=0;j<n/2;j++) {
      theta1 = j * M_PI*2 / n - 0.5*M_PI;
      theta2 = (j + 1) * M_PI*2 / n - 0.5*M_PI;

      for (i=0;i<n;i++) {
         theta3 = i * M_PI*2 / n;

         e[0].x = cos(theta2) * cos(theta3);
         e[0].y = sin(theta2);
         e[0].z = cos(theta2) * sin(theta3);
         p[0].x = pc.x + r * e[0].x;
         p[0].y = pc.y + r * e[0].y;
         p[0].z = pc.z + r * e[0].z;

         e[1].x = cos(theta1) * cos(theta3);
         e[1].y = sin(theta1);
         e[1].z = cos(theta1) * sin(theta3);
         p[1].x = pc.x + r * e[1].x;
         p[1].y = pc.y + r * e[1].y;
         p[1].z = pc.z + r * e[1].z;

         theta3 = (i+1) * 2*M_PI / n;

         e[3].x = cos(theta2) * cos(theta3);
         e[3].y = sin(theta2);
         e[3].z = cos(theta2) * sin(theta3);
         p[3].x = pc.x + r * e[3].x;
         p[3].y = pc.y + r * e[3].y;
         p[3].z = pc.z + r * e[3].z;

         e[2].x = cos(theta1) * cos(theta3);
         e[2].y = sin(theta1);
         e[2].z = cos(theta1) * sin(theta3);
         p[2].x = pc.x + r * e[2].x;
         p[2].y = pc.y + r * e[2].y;
         p[2].z = pc.z + r * e[2].z;

			ObjFace(fobj,fmtl,p,e,c,4);
      }
   }
}

void ObjAxes(FILE *fobj,FILE *fmtl,XYZ offset,double length,double r)
{
	int i;
	COLOUR red = {1,0,0},green = {0,1,0}, blue = {0,0,1};
	XYZ p[4]={{0,0,0},{1,0,0},{0,1,0},{0,0,1}};

	for (i=0;i<4;i++) {
		p[i].x *= length;
      p[i].y *= length;
      p[i].z *= length;
      p[i].x += offset.x;
      p[i].y += offset.y;
      p[i].z += offset.z;
	}

	ObjArrow(fobj,fmtl,p[0],p[1],r,8,red);
   ObjArrow(fobj,fmtl,p[0],p[2],r,8,green);
   ObjArrow(fobj,fmtl,p[0],p[3],r,8,blue);
}

void ObjArrow(FILE *fobj,FILE *fmtl,XYZ p1,XYZ p2,double r,int m,COLOUR c)
{
	XYZ p3,n;

	ObjCone(fobj,fmtl,p1,p2,r,r,m,c,TRUE);
	n.x = p2.x - p1.x;
   n.y = p2.y - p1.y;
   n.z = p2.z - p1.z;
	Normalise(&n);
	p3.x = p2.x + 8 * r * n.x;
   p3.y = p2.y + 8 * r * n.y;
   p3.z = p2.z + 8 * r * n.z;
   ObjCone(fobj,fmtl,p2,p3,2*r,0.0,m,c,TRUE);
}

/*
   A cone, capped or uncapped
	cap = 0, no caps
	cap = 1, flat ends
	cap = 2, spherical cap
   From vertex p1 to p2, radii r1 and r2
   Resolution: m
*/
void ObjCone(FILE *fobj,FILE *fmtl,XYZ p1,XYZ p2,double r1,double r2,int m,COLOUR c,int caps)
{
   int i;
   double theta1 = 0,theta2 = 2*M_PI;
   XYZ n1,n[4],r[4],p[4],q,perp;

   // Normal pointing from p1 to p2 
   n1.x = p1.x - p2.x;
   n1.y = p1.y - p2.y;
   n1.z = p1.z - p2.z;
   Normalise(&n1);

   // Create two perpendicular vectors perp and q on the plane of the disk
   perp = n1;
   if (n1.x == 0 && n1.z == 0)
      perp.x += 1;
   else
      perp.y += 1;
   CROSSPROD(perp,n1,q);
   CROSSPROD(n1,q,perp);
   Normalise(&perp);
   Normalise(&q);

   for (i=0;i<m;i++) {
      theta1 = i * 2*M_PI / m;
      n[0].x = cos(theta1) * perp.x + sin(theta1) * q.x;
      n[0].y = cos(theta1) * perp.y + sin(theta1) * q.y;
      n[0].z = cos(theta1) * perp.z + sin(theta1) * q.z;
      Normalise(&(n[0]));
		n[1] = n[0];

      p[0].x = p1.x + r1 * n[0].x;
      p[0].y = p1.y + r1 * n[0].y;
      p[0].z = p1.z + r1 * n[0].z;
      p[1].x = p2.x + r2 * n[1].x;
      p[1].y = p2.y + r2 * n[1].y;
      p[1].z = p2.z + r2 * n[1].z;

      theta2 = (i+1) * 2*M_PI / m;
      n[2].x = cos(theta2) * perp.x + sin(theta2) * q.x;
      n[2].y = cos(theta2) * perp.y + sin(theta2) * q.y;
      n[2].z = cos(theta2) * perp.z + sin(theta2) * q.z;
      Normalise(&(n[2]));
		n[3] = n[2];

      p[3].x = p1.x + r1 * n[3].x;
      p[3].y = p1.y + r1 * n[3].y;
      p[3].z = p1.z + r1 * n[3].z;
      p[2].x = p2.x + r2 * n[2].x;
      p[2].y = p2.y + r2 * n[2].y;
      p[2].z = p2.z + r2 * n[2].z;

      ObjFace(fobj,fmtl,p,n,c,4);

      if (caps == 1) {
         r[0] = p1;
         r[1] = p[0];
         r[2] = p[3];
			n[0].x = -p1.x; n[0].y = -p1.y; n[0].z = -p1.z;
			n[1] = n[0];
			n[2] = n[0];
         ObjFace(fobj,fmtl,r,n,c,3);
         r[0] = p2;
         r[1] = p[2];
         r[2] = p[1];
         n[0].x = p1.x; n[0].y = p1.y; n[0].z = p1.z;
         n[1] = n[0];
         n[2] = n[0];
         ObjFace(fobj,fmtl,r,n,c,3);
      }
   }
	
	if (caps == 2) {
		ObjSphere(fobj,fmtl,p1,r1,c,m);
      ObjSphere(fobj,fmtl,p2,r2,c,m);
	}

}

/*
	Box defined by 2 corners
*/
void ObjBox(FILE *fobj,FILE *fmtl,XYZ p1,XYZ p2,COLOUR c)
{
	XYZ p[4],n;

	// Top Bottom
	p[0].x = p1.x; p[0].y = p1.y; p[0].z = p1.z;
   p[1].x = p2.x; p[1].y = p1.y; p[1].z = p1.z;
   p[2].x = p2.x; p[2].y = p2.y; p[2].z = p1.z;
   p[3].x = p1.x; p[3].y = p2.y; p[3].z = p1.z;
	n.x = 0; n.y = 0; n.z = -1;
	ObjFace2(fobj,fmtl,p,n,c,4);
   p[0].x = p1.x; p[0].y = p1.y; p[0].z = p2.z;
   p[1].x = p2.x; p[1].y = p1.y; p[1].z = p2.z;
   p[2].x = p2.x; p[2].y = p2.y; p[2].z = p2.z;
   p[3].x = p1.x; p[3].y = p2.y; p[3].z = p2.z;
   n.x = 0; n.y = 0; n.z = 1;
   ObjFace2(fobj,fmtl,p,n,c,4);

	// Left right
   p[0].x = p1.x; p[0].y = p1.y; p[0].z = p1.z;
   p[1].x = p1.x; p[1].y = p2.y; p[1].z = p1.z;
   p[2].x = p1.x; p[2].y = p2.y; p[2].z = p2.z;
   p[3].x = p1.x; p[3].y = p1.y; p[3].z = p2.z;
   n.x = -1; n.y = 0; n.z = 0;
   ObjFace2(fobj,fmtl,p,n,c,4);
   p[0].x = p2.x; p[0].y = p1.y; p[0].z = p1.z;
   p[1].x = p2.x; p[1].y = p2.y; p[1].z = p1.z;
   p[2].x = p2.x; p[2].y = p2.y; p[2].z = p2.z;
   p[3].x = p2.x; p[3].y = p1.y; p[3].z = p2.z;
   n.x = 1; n.y = 0; n.z = 0;
   ObjFace2(fobj,fmtl,p,n,c,4);

	// Front back
   p[0].x = p1.x; p[0].y = p1.y; p[0].z = p1.z;
   p[1].x = p2.x; p[1].y = p1.y; p[1].z = p1.z;
   p[2].x = p2.x; p[2].y = p1.y; p[2].z = p2.z;
   p[3].x = p1.x; p[3].y = p1.y; p[3].z = p2.z;
   n.x = 0; n.y = -1; n.z = 0;
   ObjFace2(fobj,fmtl,p,n,c,4);
   p[0].x = p1.x; p[0].y = p2.y; p[0].z = p1.z;
   p[1].x = p2.x; p[1].y = p2.y; p[1].z = p1.z;
   p[2].x = p2.x; p[2].y = p2.y; p[2].z = p2.z;
   p[3].x = p1.x; p[3].y = p2.y; p[3].z = p2.z;
   n.x = 0; n.y = 1; n.z = 0;
   ObjFace2(fobj,fmtl,p,n,c,4);
}

/*
	Write a 3 or 4 vertex face as one or two triangles
*/
void ObjFace(FILE *fobj,FILE *fmtl,XYZ *p,XYZ *n,COLOUR c,int m)
{
	int i;
	int cindex;
	int nv[4];

   cindex = AddColour(fmtl,c);
   fprintf(fobj,"usemtl c%d\n",cindex);

	for (i=0;i<m;i++)
   	nv[i] = ObjVertex(fobj,p[i],n[i]);

	if (m >= 3)
   	fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[2],nv[2],nv[1],nv[1]);
	if (m == 4)
   	fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[3],nv[3],nv[2],nv[2]);
}

/*
   Write a 3 vertex face
*/
void ObjFace3(FILE *fobj,FILE *fmtl,XYZ p1,XYZ p2,XYZ p3,COLOUR c)
{
   int cindex;
   int nv[3];
	XYZ n;

   cindex = AddColour(fmtl,c);
   fprintf(fobj,"usemtl c%d\n",cindex);

	n = CalcNormal(p1,p2,p3);
   nv[0] = ObjVertex(fobj,p1,n);
   nv[1] = ObjVertex(fobj,p2,n);
   nv[2] = ObjVertex(fobj,p3,n);

   fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[2],nv[2],nv[1],nv[1]);
}

/*
   Write a 3 or 4 vertex face, single normal
*/
void ObjFace2(FILE *fobj,FILE *fmtl,XYZ *p,XYZ n,COLOUR c,int m)
{
   int cindex;
   int nv[5];

   cindex = AddColour(fmtl,c);
   fprintf(fobj,"usemtl c%d\n",cindex);

   nv[0] = ObjVertex(fobj,p[0],n);
   nv[1] = ObjVertex(fobj,p[1],n);
   nv[2] = ObjVertex(fobj,p[2],n);
   if (m == 4) {
      nv[3] = ObjVertex(fobj,p[3],n);
	}
   if (m == 5) {
      nv[3] = ObjVertex(fobj,p[3],n);
		nv[4] = ObjVertex(fobj,p[4],n);
   }

/* Triangle solution
   if (m >= 3)
      fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[2],nv[2],nv[1],nv[1]);
   if (m == 4)
      fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[3],nv[3],nv[2],nv[2]);
*/
	// Quad solution
   if (m == 3)
      fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[1],nv[1],nv[2],nv[2]);
   if (m == 4)
      fprintf(fobj,"f %d/%d %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[1],nv[1],nv[2],nv[2],nv[3],nv[3]);
   if (m == 5)
      fprintf(fobj,"f %d/%d %d/%d %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[1],nv[1],nv[2],nv[2],nv[3],nv[3],nv[4],nv[4]);
}

/*
   Write a uv triangle
*/
void ObjuvTri(FILE *fobj,FILE *fmtl,XYZ *p,XYZ *n,float *u,float *v)
{
   int nv[4];

   nv[0] = ObjuvVertex(fobj,p[0],n[0],u[0],v[0]);
   nv[1] = ObjuvVertex(fobj,p[1],n[1],u[1],v[1]);
   nv[2] = ObjuvVertex(fobj,p[2],n[2],u[2],v[2]);

   fprintf(fobj,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
		nv[0],nv[0],nv[0],nv[2],nv[2],nv[2],nv[1],nv[1],nv[1]);
}

/*
	Try to represent a point
*/
void ObjPoint(FILE *fobj,FILE *fmtl,XYZ p,COLOUR c,double r) 
{
	int cindex,nv[4];
	XYZ n = {0,0,0},p1,p2,p3;

   cindex = AddColour(fmtl,c);
   fprintf(fobj,"usemtl c%d\n",cindex);

	if (r > 0) {
		p1 = p; p1.x += r;
		p2 = p; p2.y += r;
		p3 = p; p3.z += r;
		nv[0] = ObjVertex(fobj,p,n);
   	nv[1] = ObjVertex(fobj,p1,n);
   	nv[2] = ObjVertex(fobj,p2,n);
		nv[3] = ObjVertex(fobj,p3,n);
   	fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[1],nv[1],nv[2],nv[2]);
      fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[2],nv[2],nv[3],nv[3]);
      fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[3],nv[3],nv[1],nv[1]);
	} else {
		nv[0] = ObjVertex(fobj,p,n);
		fprintf(fobj,"p %d\n",nv[0]);
	}
}

void ObjLine(FILE *fobj,FILE *fmtl,XYZ p1,XYZ p2,COLOUR c)
{
	int cindex;
	int nv[3];
	XYZ midp,n = {0,0,0};

   cindex = AddColour(fmtl,c);
   fprintf(fobj,"usemtl c%d\n",cindex);

   nv[0] = ObjVertex(fobj,p1,n);
	nv[1] = ObjVertex(fobj,p2,n);
	midp.x = (p1.x + p2.x) / 2;
   midp.y = (p1.y + p2.y) / 2;
   midp.z = (p1.z + p2.z) / 2;
   nv[2] = ObjVertex(fobj,midp,n);

	// Lines are not always supported
   //fprintf(fobj,"l %d %d\n",nv[0],nv[1]);

	fprintf(fobj,"f %d/%d %d/%d %d/%d\n",nv[0],nv[0],nv[2],nv[2],nv[1],nv[1]);
}

/*
	Write a vertex and normal pair
	Return current vertex
	Reset count if fptr = NULL
*/
int ObjVertex(FILE *fptr,XYZ p,XYZ n) 
{
	static int nv = 0;

	if (fptr == NULL) {
		nv = 0;
		return(0);
	}

	fprintf(fptr,"v %lf %lf %lf\n",p.x,p.y,p.z);
	fprintf(fptr,"vn %lf %lf %lf\n",n.x,n.y,n.z);
	nv++;

	return(nv);
}

/*
   Write a vertex,normal and UV
   Return current vertex
*/
int ObjuvVertex(FILE *fptr,XYZ p,XYZ n,float u,float v)
{
   static int nv = 0;

   fprintf(fptr,"v %lf %lf %lf\n",p.x,p.y,p.z);
	Normalise(&n);
   fprintf(fptr,"vn %lf %lf %lf\n",n.x,n.y,n.z);
   fprintf(fptr,"vt %lf %lf\n",u,v);
   nv++;

   return(nv);
}

/*
	Add a colour and return ID
*/
int AddColour(FILE *fptr,COLOUR c)
{
	static COLOUR *clist = NULL;
	static int nclist = 0;
	int i;
	double dr,dg,db;
	
	if (fptr == NULL) {
		nclist = 0;
		return(0);
	}

	// Does the colour already exist in the mtl file
	for (i=0;i<nclist;i++) {
		dr = c.r - clist[i].r;
      dg = c.g - clist[i].g;
      db = c.b - clist[i].b;
		if (sqrt(dr*dr + dg*dg + db*db) < 1/256.0) 
			return(i);
	}

	// If not, then add it
	fprintf(fptr,"newmtl c%d\n",nclist);
	fprintf(fptr,"Ka 1.0 1.0 1.0\n");
	fprintf(fptr,"Kd %g %g %g\n",c.r,c.g,c.b);
	fprintf(fptr,"Ks 0.0 0.0 0.0\n");
	fprintf(fptr,"d 1.0\n");
	fprintf(fptr,"Ns 0.0\n");
	fprintf(fptr,"illum 0\n");
	fprintf(fptr,"\n");

	// Add to list
	clist = realloc(clist,(nclist+1)*sizeof(COLOUR));
	clist[nclist] = c;
	nclist++;
	
	return(nclist-1);
}

void ObjGroup(FILE *fptr,char *s)
{
	fprintf(fptr,"g %s\n",s);
}

void ObjFaceN(FILE *fobj,FILE *fmtl,XYZ *p,XYZ *n,COLOUR c,int m)
{
   int i;
   int cindex;
   int *nv = NULL;

   nv = malloc(m*sizeof(int));

   cindex = AddColour(fmtl,c);
   fprintf(fobj,"usemtl c%d\n",cindex);

   for (i=0;i<m;i++)
      nv[i] = ObjVertex(fobj,p[i],n[i]);
   fprintf(fobj,"f");

   for (i=0;i<m;i++)
      fprintf(fobj," %d/%d",nv[i],nv[i]);

   fprintf(fobj,"\n");
   free(nv);
}

/*
   Box frame defined by 2 corners
*/
void ObjBBox(FILE *fobj,FILE *fmtl,XYZ p1,XYZ p2,double r,int m,COLOUR c)
{
   XYZ pa,pb;

	pa = p1; pb = p1;
	pb.x = p2.x;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
   pa = p1; pb = p1;
   pb.y = p2.y;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
   pa = p1; pb = p1;
   pb.z = p2.z;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);

   pa = p2; pb = p2;
   pb.x = p1.x;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
   pa = p2; pb = p2;
   pb.y = p1.y;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
   pa = p2; pb = p2;
   pb.z = p1.z;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);

   pa = p1; pb = p2;
   pa.x = p2.x;
	pb.y = p1.y;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
   pa = p1; pb = p2;
   pa.x = p2.x;
   pb.z = p1.z;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);

   pa = p1; pb = p2;
   pa.z = p2.z;
   pb.x = p1.x;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
   pa = p1; pb = p2;
   pa.z = p2.z;
   pb.y = p1.y;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);

   pa = p1; pb = p2;
   pa.y = p2.y;
   pb.x = p1.x;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
   pa = p1; pb = p2;
   pa.y = p2.y;
   pb.z = p1.z;
   ObjCone(fobj,fmtl,pa,pb,r,r,m,c,TRUE);
}

