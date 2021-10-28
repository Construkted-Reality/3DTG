#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bitmaplib.h"

#define MESHLABPATH "/Applications/meshlab.app/Contents/MacOS/meshlabserver"
#define CONVERTPATH "/opt/local/bin/convert"

#define TRUE  1
#define FALSE 0

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
	float x,y,z;
} XYZ;

typedef struct {
   double r,g,b;
} COLOUR;

void UpStream(void);
int DownStream(void);

int ObjStats(char *,XYZ *,XYZ *);
void FormFileName(char *,int,int,int,int);
void FormFileName2(char *,int,int);
int FileExists(char *);
void IssueCmd(char *);
void ScaleTexture(char *);
void GiveUsage(char *);
XYZ CalcNormal(XYZ,XYZ,XYZ);
void Normalise(XYZ *);
double VectorLength(XYZ,XYZ);
void CreateObjView(int);

#include "objlib.h"

