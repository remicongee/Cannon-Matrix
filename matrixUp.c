#include"matrix_utils.h"
#include"cannon_utils.h"
#include<mpi.h>
#include <time.h>
// row is not p's multiple
int main(int argc,char* argv[]){
	
	int r;
	int n;
	int h;
	int row;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc,&argv);
	double tic = -MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD,&r);
	MPI_Comm_size(MPI_COMM_WORLD,&n);
	MPI_Get_processor_name(processor_name,&h);

	//processor 0 generate matrix
	float *A,*B,*C;
	float *NA,*NB,*NC;
	int sizeA[2],sizeB[2],sizeC[2];
        int type;
	
	prepare(argv, n , r, &row, sizeA, sizeB, &type);
	sizeC[0] = sizeA[0];
	sizeC[1] = sizeB[1];

	srand(time(NULL));
	MPI_Comm comm2D;
	int periods[]={1,1};
	int p=sqrt(n);
	int dims[]={p,p};
	int ranks[2];
	MPI_Cart_create(MPI_COMM_WORLD,2,dims,periods,1,&comm2D);
	MPI_Comm_rank(comm2D,&r);
	MPI_Cart_coords(comm2D,r,2,ranks);
	//printf("%d,%d\n",ranks[0],ranks[1]);

	C=(float *)malloc((row*row)*sizeof(float *));
	int sizeNA[2],sizeNB[2],sizeNC[2];
	if(r==0){
		generate_matrix(row,&A,&B,sizeA,sizeB);
		printf("------A------\n");
		print_matrix(sizeA[0],sizeA[1],A,row);
		printf("------B------\n");
		print_matrix(sizeB[0],sizeB[1],B,row);
		distri_mat(row,p,comm2D,A,&NA,sizeNA,0);
		distri_mat(row,p,comm2D,B,&NB,sizeNB,1);
	}

	if(r!=0){
		receive_mat(r,0,row,p,comm2D,sizeNA,&NA);
		receive_mat(r,1,row,p,comm2D,sizeNB,&NB);
	}
	//print_matrix(sizeNB[0],sizeNB[1],NB);
	NC=calcul_cannon(p,NA,NB,sizeNA,sizeNB,sizeNC,comm2D,r);
	if(r==0){
	gather_mat(C,sizeNC,row,p,comm2D,NC);
	printf("------C------\n");
	print_matrix(sizeC[0],sizeC[1],C,row);
	tic += MPI_Wtime();
	printf ("Total elapsed time: %10.6f.\n", tic);
	free(A);
    	free(B);
    	free(C);
	}



	MPI_Finalize();
	return 0;


}
