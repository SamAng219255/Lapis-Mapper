#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

uint64_t* pow2;

double pi=3.141592653589793238462643383279502884197169;
double e=2.71828182845904553488480814849027;
double sqrt2=1.41421356237309504880168872420969;
double sqrt1_2=0.70710678118654752440084436210484;

#define MATH_NULL_ENDED_STR -1

#define TRUE 1
#define FALSE 0

typedef struct {
	int values[64];
	size_t length;
} intarray;

typedef struct {
	double values[64][64];
	int rows;
	int columns;
} matrix;
typedef struct {
	double values[64];
	int dimensions;
} vector;
typedef struct {
	vector a;
	vector b;
} line;
typedef struct {
	vector a;
	vector b;
	vector c;
} plane;
typedef struct {
	int faceCount;
	plane faces[64];
} polygon;
typedef struct {
	double real;
	double imaginary;
} complex;

void initPow2() {
	pow2=(uint64_t*)malloc(1025*sizeof(uint64_t));
	pow2[0]=1;
	for(int i=1; i<1025; i++) {
		pow2[i]=pow2[i-1]<<1;
	}
}

void printmatrix(matrix matrIX);

int randint(int Min,int Max) {
	int MIN=Min;
	int MAX=Max;
	if(MIN>MAX) {
		MIN=Max;
		MAX=Min;
	}
	if(MIN==MAX) {
		return MIN;
	}
	int delta=-1;
	int outcome=0;
	while(delta<=0) {
		int foo=(int)pow(2,ceil(log2(MAX-MIN)));
		outcome=(int)(rand()%foo)+MIN;
		delta=MAX-outcome;
	}
	return outcome;
}

double randDoub() {
	return randint(0,1000000)/((double)1000000);
}

matrix copyMatrix(matrix original) {
	matrix newmatrix;
	memcpy(newmatrix.values, original.values, 64*64*sizeof(double));
	newmatrix.rows=original.rows;
	newmatrix.columns=original.columns;
	return newmatrix;
}
vector copyVector(vector original) {
	vector newvector;
	memcpy(newvector.values, original.values, 64*sizeof(double));
	newvector.dimensions=original.dimensions;
	return newvector;
}

matrix newMatrix(int rows,int columns,double data[rows][columns]) {
	matrix newMat;
	newMat.rows=rows;
	newMat.columns=columns;
	for(int i=0; i<rows; i++) {
		for(int j=0; j<columns; j++) {
			newMat.values[i][j]=data[i][j];
		}
	}
	return newMat;
}
vector newVector(int dimensions,double data[dimensions]) {
	vector newVec;
	newVec.dimensions=dimensions;
	for(int i=0; i<dimensions; i++) {
		newVec.values[i]=data[i];
	}
	return newVec;
}
line newLine(vector a, vector b) {
	line newLin;
	newLin.a=copyVector(a);
	newLin.b=copyVector(b);
	return newLin;
}
plane newPlane(vector a, vector b, vector c) {
	plane newPln;
	newPln.a=copyVector(a);
	newPln.b=copyVector(b);
	newPln.c=copyVector(c);
	return newPln;
}
complex newComplex(double real, double imaginary) {
	complex newComp;
	newComp.real=real;
	newComp.imaginary=imaginary;
	return newComp;
}
complex newComplexRot(double rot) {
	return newComplex(cos(rot),sin(rot));
}

matrix vector2matrix(vector start, int mode) {
	matrix newmatrix;
	if(mode==0) {
		newmatrix.rows=1;
		newmatrix.columns=start.dimensions;
		for(int i=0; i<start.dimensions; i++) {
			newmatrix.values[0][i]=start.values[i];
		}
	}
	else {
		newmatrix.columns=1;
		newmatrix.rows=start.dimensions;
		for(int i=0; i<start.dimensions; i++) {
			newmatrix.values[i][0]=start.values[i];
		}
	}
	return newmatrix;
}
vector matrix2vector(matrix start) {
	vector newvector;
	if(start.rows==1) {
		newvector.dimensions=start.columns;
		for(int i=0; i<start.columns; i++) {
			newvector.values[i]=start.values[0][i];
		}
	}
	else {
		newvector.dimensions=start.rows;
		for(int i=0; i<start.rows; i++) {
			newvector.values[i]=start.values[i][0];
		}
	}
	return newvector;
}

double csc(double x){
	return 1/sin(x);
}
double sec(double x){
	return 1/cos(x);
}
double cot(double x){
	return 1/tan(x);
}

double clamp(double x,double min,double max) {
	return max<x ? max : (min>x?min:x);
}

vector vectorAdd(vector vector1,vector vector2) {
	vector newVec;
	newVec.dimensions=vector1.dimensions;
	for(int i=0; i<newVec.dimensions; i++) {
		newVec.values[i]=vector1.values[i]+vector2.values[i];
	}
	return newVec;
}
vector vectorSub(vector vector1,vector vector2) {
	vector newVec;
	newVec.dimensions=vector1.dimensions;
	for(int i=0; i<newVec.dimensions; i++) {
		newVec.values[i]=vector1.values[i]-vector2.values[i];
	}
	return newVec;
}
vector vectorMulti(vector vector1,double scal) {
	vector newVec;
	newVec.dimensions=vector1.dimensions;
	for(int i=0; i<newVec.dimensions; i++) {
		newVec.values[i]=vector1.values[i]*scal;
	}
	return newVec;
}
complex complexAdd(complex complex1,complex complex2) {
	return newComplex(complex1.real+complex2.real,complex1.imaginary+complex2.imaginary);
}
complex complexScale(complex complex1,double scal) {
	complex foo=newComplex(complex1.real*scal,complex1.imaginary*scal);
	return foo;
}
complex complexMulti(complex complex1,complex complex2) {
	return newComplex(complex1.real*complex2.real-complex1.imaginary*complex2.imaginary,complex1.real*complex2.imaginary+complex1.imaginary*complex2.real);
}
complex complexConjugate(complex complex1) {
	return newComplex(complex1.real,-complex1.imaginary);
}
complex complexDivide(complex complex1, complex complex2) {
	return complexScale(complexMulti(complex1,complexConjugate(complex2)),1/(complex2.real*complex2.real+complex2.imaginary*complex2.imaginary));
}
complex complexExp(double base, complex exp) {
	double theta=log(base)*exp.imaginary;
	return complexScale(newComplex(cos(theta),sin(theta)),pow(base,exp.real));
}
double complexAbs(complex complex1) {
	return sqrt(complex1.real*complex1.real+complex1.imaginary*complex1.imaginary);
}
double complexArg(complex complex1) {
	return atan(complex1.imaginary/complex1.real)+((complex1.real<0) ? pi : 0);
}
complex complexBase(complex base, double exp) {
	double nTheta=exp*complexArg(base);
	return complexScale(newComplex(cos(nTheta),sin(nTheta)),pow(complexAbs(base),exp));
}
complex complexPow(complex base, complex exp) {
	return complexMulti(complexExp(complexAbs(base),exp),complexExp(e,complexScale(exp,complexArg(base))));
}

matrix findMinorMat(int I,int J,matrix mat);

void rowOperation(double rows[64][64], int beingadded, int addedto, double cofactor, int columns) {
	for(int i=0; i<columns; i++) {
		rows[addedto][i]+=rows[beingadded][i]*cofactor;
	}
}

double matrixDet(matrix matRiX) {
	int size=matRiX.rows;
	matrix matriX=copyMatrix(matRiX);
	int matLength=size;
	double tempDet=0;
	if(matLength>2) {
		/*tempDet=0;
		int sign=1;
		for(int i=0; i<matLength; i++) {
			matrix newMat=findMinorMat(0,i,matriX);
			tempDet+=sign*matrixDet(newMat)*matriX.values[0][i];
			sign*=-1;
		}*/
		double total=1;
		for(int i=0; i<matRiX.columns; i++) {
			total*=matRiX.values[i][i];
			for(int j=i+1; j<matRiX.rows; j++) {
				rowOperation(matRiX.values,i,j,-matRiX.values[j][i]/matRiX.values[i][i],matRiX.columns);
			}
		}
		return total;
	}
	else if(matLength==2) {
		tempDet=(matriX.values[0][0]*matriX.values[1][1])-(matriX.values[0][1]*matriX.values[1][0]);
	}
	else if(matLength==1){
		tempDet=matriX.values[0][0];
	}
	else {
		tempDet=NAN;
	}
	return tempDet;
}
matrix findMinorMat(int I,int J,matrix mat) {
	matrix temp={.rows=mat.rows-1,.columns=mat.columns-1};
	for(int i=0; i<mat.rows; i++) {
		for(int j=0; j<mat.columns; j++) {
			int ii=i;
			int jj=j;
			if(i>I) {
				ii--;
			}
			if(j>J) {
				jj--;
			}
			temp.values[ii][jj]=mat.values[i][j];
		}
	}
	return temp;
}
matrix matrixInv(matrix matrIX) {
	int size=matrIX.rows;
	double det=matrixDet(matrIX);
	if(det!=0 && det!=NAN) {
		if(size==1) {
			matrix temp;
			temp.columns=1;
			temp.rows=1;
			temp.values[0][0]=1/matrIX.values[0][0];
			return temp;
		}
		else if (size==2) {
			matrix temp;
			temp.columns=2;
			temp.rows=2;
			temp.values[0][0]=matrIX.values[1][1]/det;
			temp.values[0][1]=-matrIX.values[0][1]/det;
			temp.values[1][0]=-matrIX.values[1][0]/det;
			temp.values[1][1]=matrIX.values[0][0]/det;
			return temp;
		}
		else if (size>2) {
			matrix temp;
			matrix signmat={.rows=size,.columns=size};
			temp.columns=size;
			temp.rows=size;
			for(int i=0; i<size; i++) {
				for(int j=0; j<size; j++) {
					matrix tempMat=findMinorMat(i,j,matrIX);
					temp.values[j][i]=(matrixDet(tempMat)*(2*(1-(i+j)%2)-1))/det;
					signmat.values[i][j]=(2*(1-(i+j)%2)-1);
				}
			}
			//printmatrix(signmat);
			return temp;
		}
	}
	matrix no={.rows=0,.columns=0};
	return no;
}
matrix matrixMulti(matrix matrix1,matrix matrix2){
	matrix matrix3;
	matrix3.rows=matrix1.rows;
	matrix3.columns=matrix2.columns;
	for(int i=0; i<matrix1.rows; i++) {
		for(int j=0; j<matrix2.columns; j++) {
			matrix3.values[i][j]=0;
		}
	}
	for(int i=0; i<matrix1.rows; i++) {
		for(int j=0; j<matrix2.columns; j++) {
			for(int k=0; k<matrix1.columns; k++) {
				matrix3.values[i][j]+=matrix1.values[i][k]*matrix2.values[k][j];
			}
		}
	}
	return matrix3;
}
matrix identity(int degree){
	matrix total;
	total.rows=degree;
	total.columns=degree;
	for(int i=0; i<degree; i++) {
		for(int j=0; j<degree; j++) {
			total.values[i][j]=0;
		}
	}
	for(int i=0; i<degree; i++) {
		total.values[i][i]=1;
	}
	return total;
}
matrix matrixAdd(matrix matrix1,matrix matrix2) {
	if(matrix1.rows==matrix2.rows && matrix1.columns==matrix2.columns) {
		matrix matrix3;
		matrix3.rows=matrix1.rows;
		matrix3.columns=matrix2.columns;
		for(int i=0; i<matrix1.rows; i++) {
			for(int j=0; j<matrix2.columns; j++) {
				matrix3.values[i][j]=0;
			}
		}
		for(int i=0; i<matrix1.rows; i++) {
			for(int j=0; j<matrix2.columns; j++) {
				matrix3.values[i][j]=matrix1.values[i][j]+matrix2.values[i][j];
			}
		}
		return matrix3;
	}
	else{
		matrix no={.rows=0,.columns=0};
		return no;
	}
}
matrix matrixSub(matrix matrix1,matrix matrix2){
	if(matrix1.rows==matrix2.rows && matrix1.columns==matrix2.columns) {
		matrix matrix3;
		matrix3.rows=matrix1.rows;
		matrix3.columns=matrix2.columns;
		for(int i=0; i<matrix1.rows; i++) {
			for(int j=0; j<matrix2.columns; j++) {
				matrix3.values[i][j]=0;
			}
		}
		for(int i=0; i<matrix1.rows; i++) {
			for(int j=0; j<matrix2.columns; j++) {
				matrix3.values[i][j]=matrix1.values[i][j]-matrix2.values[i][j];
			}
		}
		return matrix3;
	}
	else{
		matrix no={.rows=0,.columns=0};
		return no;
	}
}
matrix matrixMultiScal(matrix matrix1,double num){
	matrix matrix2;
	matrix2.rows=matrix1.rows;
	matrix2.columns=matrix1.columns;
	for(int i=0; i<matrix1.rows; i++) {
		for(int j=0; j<matrix1.columns; j++) {
			matrix2.values[i][j]=matrix1.values[i][j]*num;
		}
	}
	return matrix2;
}
matrix matrixRot(matrix matrix1,double xRot, double yRot, double zRot) {
	double a0=cos(xRot);
	double b0=cos(yRot);
	double c0=cos(zRot);
	double a1=sin(xRot);
	double b1=sin(yRot);
	double c1=sin(zRot);
	matrix rotMatrix={.rows=3,.columns=3,.values={{a0*b0,-a1*c0+a0*b1*c1,a1*c1+a0*b1*c0},{a1*b0,a0*c0+a1*b1*c1,-a0*c1+a1*b1*c0},{-b1,b0*c1,b0*c0}}};
	return matrixMulti(rotMatrix,matrix1);
}
void printmatrix(matrix matrIX) {
	for(int i=0; i<matrIX.rows; i++) {
		for(int j=0; j<matrIX.columns; j++) {
			printf("%.2f ",matrIX.values[i][j]);
		}
		printf("\n");
	}
}
matrix sksycrpr(vector matrix1){
	matrix temp;
	temp.rows=3;
	temp.columns=3;
	temp.values[0][0]=0;
	temp.values[0][0]=-matrix1.values[2];
	temp.values[0][0]=matrix1.values[1];
	temp.values[0][0]=matrix1.values[2];
	temp.values[0][0]=0;
	temp.values[0][0]=-matrix1.values[0];
	temp.values[0][0]=-matrix1.values[1];
	temp.values[0][0]=matrix1.values[0];
	temp.values[0][0]=0;
	return temp;
}

vector cross(int dimensionality,...){
	va_list args;
	va_start(args,dimensionality-1);
	vector a;
	matrix mat=identity(dimensionality);
	for(int i=1; i<dimensionality; i++) {
		vector temp=va_arg(args,vector);
		for(int j=0; j<dimensionality; j++) {
			mat.values[i][j]=temp.values[j];
		}
	}
	for(int i=0; i<dimensionality; i++) {
		a.values[i]=matrixDet(findMinorMat(0, i, mat));
	}
	a.dimensions=dimensionality;
	va_end(args);
	return a;
}
double dot(vector v1,vector v2){
	double total=0;
	for(int i=0; i<v1.dimensions; i++) {
		total+=v1.values[i]*v2.values[i];
	}
	return total;
}
double norm(vector v){
	return pow(pow(v.values[0],2)+pow(v.values[1],2)+pow(v.values[2],2),0.5);
}
double sqrt(double x){
	return pow(x,0.5);
}
double intPow(double x, int a) {
	double cur=1;
	for(int i=0; i<a; i++) {
		cur*=x;
	}
	return cur;
}
vector unitVec(vector v){
	double m=norm(v);
	vector newvec;
	newvec.dimensions=3;
	newvec.values[0]=v.values[0]/m;
	newvec.values[0]=v.values[1]/m;
	newvec.values[0]=v.values[2]/m;
	return newvec;
}
vector alignVectors(vector v1,vector v2){
	vector V1=unitVec(v1);
	vector V2=unitVec(v2);
	vector v=cross(3,V1,V2);
	double c=dot(V1,V2);
	vector R;
	R.dimensions=3;
	matrix r=matrixAdd(matrixAdd(identity(3),sksycrpr(v)),matrixMultiScal(matrixMulti(sksycrpr(v),sksycrpr(v)),1/(1+c)));
	R.values[0]=r.values[0][0];
	R.values[1]=r.values[0][1];
	R.values[2]=r.values[0][2];
	return R;
}
double distance(vector p1,vector p2){
	return pow(pow(p1.values[0]-p2.values[0],2)+pow(p1.values[1]-p2.values[1],2)+pow(p1.values[2]-p2.values[2],2),0.5);
}
double line2point(vector p1,vector p2,vector p3) {
	double a=distance(p1,p2);
	double b=distance(p2,p3);
	double c=distance(p3,p1);
	double cosB=(pow(a,2)+pow(c,2)-pow(b,2))/(2*a*c);
	double sinB=sqrt(1-pow(cosB,2));
	return c*sinB;
}
double plane2point(vector p1,vector p2,vector p3,vector p4){
	matrix mat;
	mat.rows=3;
	mat.columns=3;
	for(int i=0; i<3; i++) {
		mat.values[0][i]=p1.values[i];
		mat.values[1][i]=p2.values[i];
		mat.values[2][i]=p3.values[i];
	}
	matrix newmat=matrixInv(mat);
	matrix tempmathmat;
	tempmathmat.rows=1;
	tempmathmat.columns=3;
	tempmathmat.values[0][0]=1;
	tempmathmat.values[0][1]=1;
	tempmathmat.values[0][2]=1;
	matrix res=matrixMulti(newmat,tempmathmat);
	printmatrix(newmat);
	double a=res.values[0][0];
	double b=res.values[1][0];
	double c=res.values[2][0];
	double d=1;
	return fabs(a*p4.values[0]+b*p4.values[1]+c*p4.values[2]-d)/sqrt(pow(a,2)+pow(b,2)+pow(c,2));
}
double tetraVol(vector p1,vector p2,vector p3,vector p4){
	//return distance(p1,p2)*line2point(p1,p2,p3)*plane2point(p1,p2,p3,p4)/6;
	return fabs(dot(vectorSub(p2,p1),cross(3,vectorSub(p3,p1),vectorSub(p4,p1)))/6);
}
int segmentIntersectsPlane(line segment, plane Plane) {
	vector n=cross(3,vectorSub(Plane.b,Plane.a),vectorSub(Plane.c,Plane.a));
	double den=dot(n,vectorSub(segment.b,segment.a));
	if(den==0) {
		return 0;
	}
	double r1=dot(n,vectorSub(Plane.a,segment.a))/den;
	if(r1>=0 && r1<=1) {
		return 1;
	}
	else {
		return 0;
	}
}
int segmentIntersectsTriangle(line segment, plane Plane) {
	vector u=vectorSub(Plane.b,Plane.a);
	vector v=vectorSub(Plane.c,Plane.a);
	vector n=cross(3,u,v);
	if(!segmentIntersectsPlane(segment,Plane)) {
		return 0;
	}
	vector w=vectorSub(segment.b,Plane.a);
	double uv=dot(u,v);
	double vv=dot(v,v);
	double uu=dot(u,u);
	double wu=dot(w,u);
	double wv=dot(w,v);
	double den=pow(uv,2)-(uu*vv);
	double s=(uv*wv-vv*wu)/den;
	double t=(uv*wu-uu*wv)/den;
	if(s>0 && t>0 && (s+t)<1) {
		return 1;
	}
	else {
		return 0;
	}
}
double polyVol(polygon poly) {
	double total=0;
	double partVolume;
	//vector point={.dimensions=3,.values={0,0,0}};
	vector point={.dimensions=3,.values={2*randDoub()-1,2*randDoub()-1,2*randDoub()-1}};
	for(int i=0; i<poly.faceCount; i++) {
		int faceEncounters=0;
		line countLine=newLine(point,vectorMulti(vectorAdd(vectorAdd(poly.faces[i].a,poly.faces[i].b),poly.faces[i].c),1/3));
		for(int j=0; j<poly.faceCount; j++) {
			if(i!=j && segmentIntersectsTriangle(countLine,poly.faces[j])) {
				faceEncounters++;
			}
		}
		//partVolume=tetraVol(point,poly.faces[i].a,poly.faces[i].b,poly.faces[i].c);
		partVolume=fabs(dot(poly.faces[i].a,cross(3,poly.faces[i].b,poly.faces[i].c))/6);
		if(partVolume==0) {
			partVolume=fabs(dot(poly.faces[i].b,cross(3,poly.faces[i].a,poly.faces[i].c))/6);
		}
		if(partVolume==0) {
			partVolume=fabs(dot(poly.faces[i].c,cross(3,poly.faces[i].a,poly.faces[i].b))/6);
		}
		printf("%f %i %i\n",partVolume,faceEncounters,i+1);
		if(fmod(faceEncounters,2)) {
			total-=partVolume;
		}
		else {
			total+=partVolume;
		}
	}
	return fabs(total);
}
double polyVolAlt(polygon poly) {
	double total=0;
	double partVolume;
	vector point={.dimensions=3,.values={3,5,7}};
	for(int i=0; i<poly.faceCount; i++) {
		int faceEncounters=0;
		line countLine=newLine(point,poly.faces[i].a);
		for(int j=0; j<poly.faceCount; j++) {
			if(i!=j && segmentIntersectsTriangle(countLine,poly.faces[j])) {
				faceEncounters++;
			}
		}
		partVolume=tetraVol(point,poly.faces[i].a,poly.faces[i].b,poly.faces[i].c);
		if(partVolume==0) {
			partVolume=fabs(dot(poly.faces[i].b,cross(3,poly.faces[i].a,poly.faces[i].c))/6);
		}
		if(partVolume==0) {
			partVolume=fabs(dot(poly.faces[i].c,cross(3,poly.faces[i].a,poly.faces[i].b))/6);
		}
		printf("%f %i\n",partVolume,faceEncounters);
		if(fmod(faceEncounters,2)) {
			total-=partVolume;
		}
		else {
			total+=partVolume;
		}
	}
	return fabs(total);
}

double diangle(int n,int n1){
	vector v1={.dimensions=3,.values={cos(pi*(n-2)/(2*n)),sin(pi*(n-2)/(2*n)),0}};
	vector v2={.dimensions=3,.values={cos(pi*(n-2)/(2*n)),-sin(pi*(n-2)/(2*n)),0}};
	vector v3={.dimensions=3,.values={sin(pi*(n1-2)/(2*n1))*csc(pi/n),0,sqrt(1-pow(sin(pi*(n1-2)/(2*n1))*csc(pi/n),2))}};
	vector u=cross(3,v1,v3);
	vector v=cross(3,v2,v3);
	return acos(dot(u,v)/(norm(u)*norm(v)));
}

int intFromStr(char* str) {
	int y=0;
	int i=0;
	int isNegative=0;
	char cur=str[i];
	while(cur!='\x00') {
		if(i==0 && cur=='-') {
			isNegative=1;
		}
		else {
			if(y>214748364 || (y==214748364 && (cur=='8' || cur=='9'))) {
				return 2147483647;
				/*
				If:
					y is large enough that the decimal shift will overflow it OR
					we're adding an '8' or '9' where the max without overflow is a '7'
				Then:
					return the max int
				*/
			}
			y*=10;
			switch(cur) {
				case '0':
					y+=0;
					break;
				case '1':
					y+=1;
					break;
				case '2':
					y+=2;
					break;
				case '3':
					y+=3;
					break;
				case '4':
					y+=4;
					break;
				case '5':
					y+=5;
					break;
				case '6':
					y+=6;
					break;
				case '7':
					y+=7;
					break;
				case '8':
					y+=8;
					break;
				case '9':
					y+=9;
					break;
				default:
					return 0;
					break;
			}
		}
		i++;
		cur=str[i];
	}
	if(isNegative) y*=-1;
	return y;
}

double doubleFromStr(char* str) {
	double y=0;
	int i=0;
	int decimals=0;
	int pastDecimal=0;
	int isNegative=0;
	char cur=str[i];
	while(cur!='\x00') {
		if(i==0 && cur=='-') {
			isNegative=1;
		}
		else {
			y*=10;
			if(pastDecimal) {
				decimals++;
			}
			switch(cur) {
				case '0':
					y+=0;
					break;
				case '1':
					y+=1;
					break;
				case '2':
					y+=2;
					break;
				case '3':
					y+=3;
					break;
				case '4':
					y+=4;
					break;
				case '5':
					y+=5;
					break;
				case '6':
					y+=6;
					break;
				case '7':
					y+=7;
					break;
				case '8':
					y+=8;
					break;
				case '9':
					y+=9;
					break;
				case '.':
					if(pastDecimal) {
						return 0;
					}
					y/=10;
					pastDecimal=1;
					break;
				default:
					return 0;
					break;
			}
		}
		i++;
		cur=str[i];
	}
	for(int j=0; j<decimals; j++) {
		y/=10;
	}
	if(isNegative) y*=-1;
	return y;
}
int numPlaces(int n) {
	if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	return 10;
}

size_t fwriteE(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	void* nptr=malloc(nmemb*size);
	for(size_t i=0; i<nmemb; i++) {
		size_t ind=i*size;
		for(size_t j=0; j<size; j++) {
			((uint8_t*)nptr)[ind+size-j-1]=((uint8_t*)ptr)[ind+j];
		}
	}
	size_t result=fwrite(nptr,size,nmemb,stream);
	free(nptr);
	return result;
}
size_t freadE(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	void* nptr=malloc(nmemb*size);
	size_t result=fread(nptr,size,nmemb,stream);
	for(size_t i=0; i<result; i++) {
		size_t ind=i*size;
		for(size_t j=0; j<size; j++) {
			//printf("(%zu/%zu, %zu/%zu): %02x\n",i,result,j,size,((uint8_t*)nptr)[ind+j]);
			((uint8_t*)ptr)[ind+j]=((uint8_t*)nptr)[ind+size-j-1];
		}
	}
	free(nptr);
	return result;
}

int compareStrings(size_t len, char* str1, char* str2) {
	size_t i=0;
	while(len!=MATH_NULL_ENDED_STR || str1[i]!='\x00') {
		if((len==MATH_NULL_ENDED_STR && str2[i]=='\x00') || str1[i]!=str2[i])
			return FALSE;
		i++;
		if(len!=MATH_NULL_ENDED_STR && i==len)
			return TRUE;
	}
	if(str2[i]!='\x00')
		return FALSE;
	return TRUE;
}

int split(int delimiterlen, int strlen, int arrlen, int arrstrlen, char delimiter[delimiterlen], char initial[strlen], char seperated[arrlen][arrstrlen]) {
	int place=0;
	int place2=0;
	int deliplace=0;
	for(int i=0; i<strlen; i++) {
		if(initial[i]==delimiter[deliplace]) {
			deliplace++;
		}
		else {
			deliplace=0;
		}
		if(deliplace==delimiterlen) {
			place++;
			place2=0;
			deliplace=0;
		}
		else {
			if(deliplace==0) {
				seperated[place][place2]=initial[i];
				place2++;
			}
		}
		place%=arrlen;
		place2%=arrstrlen;
	}
	return place+1;
	//to future me:
	//this function splits "initial" by the string "delimiter" and saves it into "seperated"
	//delimiterlen is the length of the "delimiter" string
	//strlen is length of the "initial" string
	//arrlen is the length of the "seperated" array
	//arrstrlen is the length of strings in the "seperated" array
}
