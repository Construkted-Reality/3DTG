#ifndef OBJLIB_H
#define OBJLIB_H

void ObjSphere(FILE *,FILE *,XYZ,double,COLOUR,int);
void ObjPoint(FILE *,FILE *,XYZ,COLOUR,double);
void ObjLine(FILE *,FILE *,XYZ,XYZ,COLOUR);
void ObjCone(FILE *,FILE *,XYZ,XYZ,double,double,int,COLOUR,int);
void ObjAxes(FILE *,FILE *,XYZ,double,double);
void ObjArrow(FILE *,FILE *,XYZ,XYZ,double,int,COLOUR);
void ObjBox(FILE *,FILE *,XYZ,XYZ,COLOUR);
void ObjFace(FILE *,FILE *,XYZ *,XYZ *,COLOUR,int);
void ObjFace3(FILE *,FILE *,XYZ,XYZ,XYZ,COLOUR);
void ObjFace2(FILE *,FILE *,XYZ *,XYZ,COLOUR,int);
void ObjFaceN(FILE *,FILE *,XYZ *,XYZ *,COLOUR,int);
void ObjuvTri(FILE *,FILE *,XYZ *,XYZ *,float *,float *);
int ObjVertex(FILE *,XYZ,XYZ);
int ObjuvVertex(FILE *,XYZ,XYZ,float,float);
void ObjGroup(FILE *,char *);
int AddColour(FILE *,COLOUR);
void ObjBBox(FILE *,FILE *,XYZ,XYZ,double,int,COLOUR);

#endif
