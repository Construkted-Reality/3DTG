#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#define TRUE  1
#define FALSE 0

#define EPS 0.00001

#define ABS(x) (x < 0 ? -(x) : (x))
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)
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
} XYZ;

typedef struct {
   float u,v;     // Double not required for uv coordinates
} UV;

typedef struct {
   int xyzid[3];
   int uvid[3];
} FACE;

typedef struct {
	XYZ p;
	int n;        // Number of points in the average
	int xyzid;
	int uvid;
} GRIDCELL;

typedef struct {
	int verbose;              // How chatty
	int n;                    // Resolution along longest edge of model
} PARAMS;

// Prototypes
void GiveUsage(char *);
int ReadObj(FILE *);
void GridVertices(void);
int ProcessFaces(FILE *,char *);
int ParseFace(char *,FACE *);
void MinMaxXYZ(XYZ,XYZ *,XYZ *);
void Init(void);
void LineStatus(long);
double GetRunTime(void);
void XYZ2grid(XYZ,int *,int *,int *);
int SaveFace(FILE *,FACE);


