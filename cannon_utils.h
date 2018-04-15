#include"matrix_utils.h"
#include<mpi.h>

int BLOCK_LOW(int id,int p,int n){
	return id*n/p;
}

int BLOCK_HIGH(int id,int p,int n){
	return (id+1)*n/p;
}
int BLOCK_SIZE(int id,int p,int n){
	return BLOCK_LOW(id+1,p,n)-BLOCK_LOW(id,p,n);
}


int coords(int ranks[],int tag,int coords[],int size[],int p,int row){
	int x,y;
	y=ranks[0];
	x=p-ranks[1]-1;
	if(tag == 0){
	y=(y+x)%p;
	}
	else if(tag == 1){
	x=(x+y)%p;
	}
	else {
	}
	coords[0] = BLOCK_LOW(x,p,row);
	coords[1] = BLOCK_LOW(y,p,row);
	size[0]= BLOCK_SIZE(x,p,row);
	size[1]= BLOCK_SIZE(y,p,row);

	return 0;
}


int distri_mat(int row,int p,MPI_Comm commN,float* A,float** NA,int sizeNA[],int tag){
	int rank=0,root=0;
	float* recv;
	int i,j;
	int *displs,*scounts;
	MPI_Datatype tmptype,blocktype;
	int n=p*p;
	displs = (int *)malloc(n*sizeof(int));
	scounts = (int *)malloc(n*sizeof(int));
	int offset=0;
	

	for(rank=0;rank<(p*p);rank++){

	int ranks[2];
	MPI_Cart_coords(commN,rank,2,ranks);
	int A_coord[2],sizeA[2];
	coords(ranks,tag,A_coord,sizeA,p,row);
	MPI_Type_hvector(1,sizeA[1],sizeof(float), MPI_FLOAT, &tmptype);
	MPI_Type_hvector(sizeA[0],1,sizeof(float)*row, tmptype, &blocktype);
	MPI_Type_commit(&blocktype);
	if(rank !=0)
	{MPI_Send(A+A_coord[0]*row+A_coord[1],1,blocktype,rank,tag,commN);	
	MPI_Type_free(&blocktype);}
	else{
	*NA=(float *)malloc((sizeA[0]*sizeA[1])*sizeof(float *));
	sizeNA[0]=sizeA[0];
	sizeNA[1]=sizeA[1];
	int k=0;
	for(i=0;i<sizeA[0];i++){
		for(j=0;j<sizeA[1];j++)
		(*NA)[k++]=A[A_coord[0]*row+A_coord[1]+i*row+j];
	}
	}


	}

	

return 0;
}


int receive_mat(int rank,int tag,int row,int p,MPI_Comm commN,int size[],float** NA){
	int coord[2],ranks[2];
	MPI_Cart_coords(commN,rank,2,ranks);
	coords(ranks,tag,coord,size,p,row);
	//printf("%d,%d\n",size[0],size[1]);
	(*NA)=(float *)malloc((size[0]*size[1])*sizeof(float *));
	MPI_Status status;
        MPI_Recv(*NA,size[0]*size[1],MPI_FLOAT,0,tag,commN,&status); 
	
return 0;
}

int replace_matrix(float** A,int tag,MPI_Comm commN,int sizeA[]){
	int source,dest;
	MPI_Status status;
	int tmpsize[2]; tmpsize[0]=sizeA[0];tmpsize[1]=sizeA[1];
	//printf("original size is %d,%d\n",sizeA[0],sizeA[1]);
	if(tag == 0)
	MPI_Cart_shift(commN, tag, 1, &dest,&source);
	else
	MPI_Cart_shift(commN, tag, 1, &source,&dest);
	MPI_Sendrecv_replace(sizeA,2,MPI_INT,dest,tag,source,tag,commN,&status);
	float *tmp;
	tmp=(float*)malloc((sizeA[0]*sizeA[1])*sizeof(float*));
	MPI_Sendrecv(&((*A)[0]),tmpsize[0]*tmpsize[1],MPI_FLOAT,dest,tag,tmp,sizeA[0]*sizeA[1],MPI_FLOAT,source,tag,commN,&status);
	//printf("final size is %d,%d\n",sizeA[0],sizeA[1]);
	free(&((*A)[0]));
	(*A)=tmp;
	
return 0;
}

float* calcul_cannon(int p,float* A,float* B,int sizeA[],int sizeB[],int sizeC[],MPI_Comm commN,int r){
	sizeC[0]=sizeA[0];
	sizeC[1]=sizeB[1];
	float *C,*tmp;
	//C=(float*)malloc((sizeC[0]*sizeC[1])*sizeof(float*));
	//tmp=(float*)malloc((sizeC[0]*sizeC[1])*sizeof(float*));
	C=multi_matrix(sizeA,sizeB,A,B); 
	
	for(int i=1;i<p;i++){
	replace_matrix(&A,0,commN,sizeA); 
	replace_matrix(&B,1,commN,sizeB); 
	tmp=multi_matrix(sizeA,sizeB,A,B);
	for(int k=0;k<sizeC[0]*sizeC[1];k++){
	C[k]=C[k]+ tmp[k];
	}
	}
	if(r!=0){
		MPI_Send(C,sizeC[0]*sizeC[1],MPI_FLOAT,0,p,commN);

	}

return C;
}


int gather_mat(float* C,int sizeNC[],int row,int p,MPI_Comm commN,float* NC){
	
	
	int sizes[2];
	for(int i=0;i<p*p;i++){
	int ranks[2],coord[2],sizes[2];
	MPI_Cart_coords(commN,i,2,ranks);
	coords(ranks,2,coord,sizes,p,row);
	if(i!=0){
	MPI_Datatype tmptype,blocktype;
	MPI_Type_hvector(1,sizes[1],sizeof(float), MPI_FLOAT, &tmptype);
	MPI_Type_hvector(sizes[0],1,sizeof(float)*row, tmptype, &blocktype);
	MPI_Type_commit(&blocktype);
	MPI_Status status;
	MPI_Recv(C+coord[0]*row+coord[1],1,blocktype,i,p,commN,&status);
	MPI_Type_free(&blocktype);
	}
	else{
		int i,j;
		for(i=0;i<sizeNC[0];i++){
			for(j=0;j<sizeNC[1];j++)
			{	//printf("%d,%d\n",p,coord[0]*row+coord[1]+i*row+j);
				C[coord[0]*row+coord[1]+i*row+j]=NC[sizeNC[1]*i+j];}

		}
	}
	}

return 0;
}

void prepare(char* argv[], int _numProcess, int _rank, int* _size, int* _sizeA, int* _sizeB, int *_type)
{
  // if(_rank == 0) printf("%s\n", argv[0]);
  // if(_rank == 0) printf("%s\n", argv[1]);
  *_type = atoi(argv[1]);
  if(*_type == 0){
  	*_size = atoi(argv[2]);
  	(_sizeA)[0] = (_sizeA)[1] = (_sizeB)[0] = (_sizeB)[1] = *_size;
  }
  else{
  	(_sizeA)[0] = atoi(argv[2]);
	(_sizeA)[1] = atoi(argv[3]);
	(_sizeB)[0] = atoi(argv[3]);
	(_sizeB)[1] = atoi(argv[4]);
	if((_sizeA)[0] > (_sizeA)[1]){
		if((_sizeA)[0]<(_sizeB)[1])
		*_size=(_sizeB)[1];
		else
		*_size=(_sizeA)[0];
	}
	else
		if((_sizeA)[1]<(_sizeB)[1])
		*_size=(_sizeB)[1];
		else
		*_size=(_sizeA)[1];
  }
  
  if((_sizeA)[0] <= 1 || (_sizeA)[1] <= 1 || (_sizeB)[1]<=1)
  {
    if(_rank == 0) printf("Matrix size should be bigger than 1.\n");
    exit(1);
  }
  // double _cutD = (double)_size / sqrt((double)_numProcess);
  if((int)sqrt(_numProcess) != sqrt((double)_numProcess))
  {
    if(_rank == 0) printf("Process number should be square.\n");
    exit(1);
  }
  int _cut = (int)(sqrt(_numProcess));

  if(_rank == 0) printf("Start %d-calculating matrix(%dx%d) multiplication with %d process.\n", _cut, *_size, *_size, _numProcess);
}
