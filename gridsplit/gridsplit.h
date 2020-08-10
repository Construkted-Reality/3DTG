#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#define MESHLABPATH "/Applications/meshlab.app/Contents/MacOS/meshlabserver"

#define TRUE  1
#define FALSE 0

#define EPS 0.00001

#define ABS(x) (x < 0 ? -(x) : (x))
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)
#define sech(x) (1.0/cosh(x))
#define SIGN(x) (x < 0 ? (-1) : 1)
#define MODULUS(p) (sqrt(p.x*p.x + p.y*p.y + p.z*p.z))
#define CROSSPROD(p1,p2,p3) \
   p3.x = p1.y*p2.z - p1.z*p2.y; \
   p3.y = p1.z*p2.x - p1.x*p2.z; \
   p3.z = p1.x*p2.y - p1.y*p2.x
#define DOTPRODUCT(v1,v2) ( v1.x*v2.x + v1.y*v2.y + v1.z*v2.z )

typedef struct {
   double x,y,z;
} dXYZ;
typedef struct {
   float x,y,z;
	int flag;
} XYZ;

typedef struct {
   float u,v;     // Double not required for uv coordinates
	int flag;
} UV;

typedef struct {
   XYZ themin,themax;            // The bounds chosen for splitting
   long vertexcount,facecount;
	int ix,iy,iz;                 // Box index
} BBOX;

typedef struct {
   int xyzid[3];
   int uvid[3];
	short materialid;
} FACE;

typedef struct {
	char name[256];
} MATERIALNAME;

typedef struct {
   double r,g,b;
} COLOUR;

typedef struct {
	int verbose;          // How chatty
	int trisplit;         // Whether to split triangles at boundaries
	int depth;           // The number of times to split along each axis
} PARAMS;

// Prototypes
void GiveUsage(char *);
int ReadObj(FILE *);
int ProcessFaces(char *);
void WriteDecimateLine(FILE *,char *);
int SplitFaceX(BBOX,FACE *,int);
int SplitFaceY(BBOX,FACE *,int);
int SplitFaceZ(BBOX,FACE *,int);
XYZ SplitXEdge(double,XYZ,XYZ,double *);
XYZ SplitYEdge(double,XYZ,XYZ,double *);
XYZ SplitZEdge(double,XYZ,XYZ,double *);
void AddXYZUV(XYZ,XYZ,UV,UV);
int ParseFace(char *,FACE *);
void GenerateBBoxes(void);
int FaceInBox(FACE,BBOX);
void MinMaxXYZ(XYZ,XYZ *,XYZ *);
void FormFileName(char *,char *,int,int,int,int);
void SaveTileset(char *);
void Init(void);
void SaveBBoxes(void);

// Misc
int VertexInBox(XYZ,BBOX);
XYZ MidPoint(XYZ,XYZ);
XYZ CalcNormal(XYZ,XYZ,XYZ);
void Normalise(XYZ *);
double VectorLength(XYZ,XYZ);
void CopyFace(FACE *,FACE *);
void CleanString(char *);
void LineStatus(long);
double GetRunTime(void);
int Powerof2(int,int *,int *);
int ObjExists(char *);

#include "objlib.h"

