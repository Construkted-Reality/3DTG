#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include "bitmaplib.h"

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
   float x,y;
} XY;

typedef struct {
   double x,y,z;
} dXYZ;
typedef struct {
   float x,y,z;
} XYZ;

typedef struct {
   float u,v;              // Double not required for uv coordinates
	short materialid;       // Which material texture are in
} UV;

typedef struct {
   int xyzid[3];
   int uvid[3];
   short materialid;       // Which material texture are in
} FACE;

typedef struct {
   char name[256];
   char texture[256];
   BITMAP4 *image;
   int width,height;
} MATERIAL;

typedef struct {
   double r,g,b;
} COLOUR;

typedef struct {
   int h,v;
} POINT;

typedef struct {
   int matid;            // An island is associated with a particular texturemap
   POINT *poly;          // Polygon around the island
   int npoly;            // Length of polygon
   int area;             // In pixels
   POINT themin,themax;  // Bounding box
   POINT offset;         // The offset of this island after packing
} ISLAND;

typedef struct {
   int verbose;          // How chatty
	int debug;            // Saves images at various stages, for debugging
} PARAMS;

// Prototypes
void GiveUsage(char *);
int ReadObj(FILE *);
int ReadTextures(void);
int ReadMaterial(char *,char *,int);
int ProcessFaces(char *);
int ParseFace(char *,FACE *);
void Init(void);
void MaskTextures(void);
void FindIslands(void);
void PackIslands(char *);
int PointInIsland(int,POINT,float *);
int FindBestPosition(ISLAND,BITMAP4 *,int,int,POINT *);
int TestRectAlpha(POINT,int,int,BITMAP4 *,int,int);
int WriteOBJFile(char *,char *);

// --- debugging stuff
void SaveMasks(void);
void SaveIslands(void);
void SaveIslands2(BITMAP4 *,int,int);

// Misc ... secondary functions
int isLeft(POINT,POINT,POINT);
int InsidePolygon(POINT,POINT *,int);
void StripExtension(char *);
void CleanString(char *);
double GetRunTime(void);
void MinMaxXYZ(XYZ,XYZ *,XYZ *);
XYZ CalcNormal(XYZ,XYZ,XYZ);
void Normalise(XYZ *);
double PointLine2D(XY,XY,XY,XY *,double *);
POINT CalcDir(POINT,POINT);
float ClosestPoint(POINT *,int,POINT);
double Angle2D(double,double,double,double);
int islandcompare(const void *,const void *);
void CopyFace(FACE *,FACE *);
void LineStatus(long);
int AlphaZero(POINT,BITMAP4 *,int,int);
void PointClip(int,int,POINT *);

