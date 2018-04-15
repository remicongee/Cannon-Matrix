#include <mpi.h>
#include <math.h>
#include"utils.h"
int main(int argc,char* argv[]){
  MPI_Init(&argc, &argv);
	double tic = -MPI_Wtime();
	double** A, ** B, ** C;
	int  * globalSize = (int*)malloc(sizeof(int) * 2);
	int  * globalSizeA = (int*)malloc(sizeof(int) * 2);
	int  * globalSizeB = (int*)malloc(sizeof(int) * 2);
	int type = atoi(argv[1]);
	if(type == 0)
	{	
		int size=atoi(argv[2]);
		globalSizeA[0] = globalSizeA[1] = \
 		globalSizeB[0] = globalSizeB[1] = \
		globalSize[0] = globalSize[1] = size;


		
	}
	else if(type == 1)
	{	
		int Arow=atoi(argv[2]);
		int Acol=atoi(argv[3]);
		int Bcol=atoi(argv[4]);
	
		globalSize[0] = globalSizeA[0]=Arow;
   		globalSizeA[1] = Acol;
		globalSizeB[0] = Acol;
		globalSize[1] = globalSizeB[1]=Bcol;

	}
	

		printf("------A------\n");
    		genMatrix(&A, globalSizeA);
    		printMatrix(A, globalSizeA);
    		printf("------B------\n");
   		genMatrix(&B, globalSizeB);
		printMatrix(B, globalSizeB);
		C=mulMatrix(A, B, globalSizeA, globalSizeB);
		printf("------C------\n");
		printMatrix(C, globalSize);

	tic += MPI_Wtime();
    	printf ("Total elapsed time: %10.6f.\n", tic);

    free(A);
    free(B);
    free(C);
  MPI_Finalize();
return 0;
}
