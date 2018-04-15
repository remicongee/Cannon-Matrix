#ifndef matrixutil_h
#define matrixutil_h

#include<stdio.h>
#include<stdlib.h>
#include<math.h>


int generate_matrix(int row,float** matA,float** matB,int* sizeA,int* sizeB){
	int i=0,j=0;
	
	*matA=(float*)malloc((row*row)*sizeof(float*));
	*matB=(float*)malloc((row*row)*sizeof(float*));
	for(i=0;i<row;i++){
		for(j=0;j<row;j++){
			if(i<sizeA[0]&&j<sizeA[1])
			(*matA)[i*row+j]=rand()/(float)(RAND_MAX/10);
			else
			(*matA)[i*row+j]=0;
			if(i<sizeB[0]&&j<sizeB[1])
			(*matB)[i*row+j]=rand()/(float)(RAND_MAX/10);
			else
			(*matB)[i*row+j]=0;
		}
		
	}
return 0;
}

int print_matrix(int row,int col,float* mat,int size){
	int i,j,k=0;
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
		if(i<row&&j<col)
		printf("%.2lf ",mat[i*size+j]);
		}
	if(i<row)
	printf("\n");
	}
	printf("\n");
return 0;
}

float* multi_matrix(int sizeA[],int sizeB[],float* matA,float* matB){
	int i,j,p,q=0;
	float* C;
	int sizeC[2];
	sizeC[0]=sizeA[0];
	sizeC[1]=sizeB[1];
	C=(float*)malloc((sizeC[0]*sizeC[1])*sizeof(float*));
	for(i=0;i<sizeB[1];i++){
		for(p=0;p<sizeA[0];p++){
		float sum=0;
			for(j=0;j<sizeB[0];j++){
			q=j;
			sum=sum+matA[p*sizeA[1]+q]*matB[j*sizeB[1]+i];
			}
		C[p*sizeC[1]+i]=sum;
		}
		
	}
	
return C;
	
}

#endif /* matrixutil_h */
