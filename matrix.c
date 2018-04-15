#include "cannon.h"

int main(int argc, char **argv)
{
  double **A, **B, **C, **AP, **BP, **CC;
  int rank, numProcess;
  int nameLen;
  char processorName[MPI_MAX_PROCESSOR_NAME];
  // printf("start mpi\n");
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcess);
  MPI_Get_processor_name(processorName, &nameLen);

  int cut, sizeBlock, gen,
      *sizeA = (int *)malloc(sizeof(int) * 2),
      *sizeB = (int *)malloc(sizeof(int) * 2),
      *globalSize = (int *)malloc(sizeof(int) * 2),
      *realSize = (int *)malloc(sizeof(int) * 2);
  prepare(argv, &cut, &sizeBlock, &sizeA, &sizeB, &globalSize, &gen, &realSize, numProcess, rank);
  srand(time(NULL));

  MPI_Comm comm2D;
  cartTopo(cut, MPI_COMM_WORLD, &comm2D);
  MPI_Comm_rank(comm2D, &rank);
  if (rank == 0)
  {
    printf("------A------\n");
    genMatrix(&A, sizeA);
    printMatrix(A, sizeA);
    printf("\n");
    printf("------B------\n");
    genMatrix(&B, sizeB);
    printMatrix(B, sizeB);
    printf("\n");
    if(gen) padMatrix(A, B, sizeA, sizeB, &AP, &BP);
    genBlock(&C, globalSize[0]);
  }
  //MPI_Barrier(comm2D);

  int rank2D[2];
  MPI_Cart_coords(comm2D, rank, 2, rank2D);
  // printf("Process %d (%d, %d)\n", rank, rank2D[0], rank2D[1]);

  double **blockA, **blockB, **blockC;
  genBlock(&blockA, sizeBlock);
  genBlock(&blockB, sizeBlock);
  genBlock(&blockC, sizeBlock);

  double tic = -MPI_Wtime();

  if (gen)
    canScatter(AP, blockA, numProcess, sizeBlock, rank, comm2D);
  else
    canScatter(A, blockA, numProcess, sizeBlock, rank, comm2D);
  int source, dest;
  MPI_Status status;
  MPI_Cart_shift(comm2D, 1, -rank2D[0], &source, &dest);
  MPI_Sendrecv_replace((&blockA)[0][0], sizeBlock * sizeBlock, MPI_DOUBLE, dest, 1, source, 1, comm2D, &status);
  // gatherResult(blockA, C, cut, sizeBlock, numProcess, rank, comm2D);

  if (gen)
    canScatter(BP, blockB, numProcess, sizeBlock, rank, comm2D);
  else
    canScatter(B, blockB, numProcess, sizeBlock, rank, comm2D);
  MPI_Cart_shift(comm2D, 0, -rank2D[1], &source, &dest);
  MPI_Sendrecv_replace((&blockB)[0][0], sizeBlock * sizeBlock, MPI_DOUBLE, dest, 1, source, 1, comm2D, &status);
  // gatherResult(blockB, C, cut, sizeBlock, numProcess, rank, comm2D);

  //MPI_Barrier(comm2D);

  canMul(blockA, blockB, &blockC, sizeBlock, rank);
  //MPI_Barrier(comm2D);

  for (int i = 0; i < cut - 1; i++)
  {
    canRotate(rank, rank2D, blockA, blockB, sizeBlock, comm2D);
    canMul(blockA, blockB, &blockC, sizeBlock, rank);
    //MPI_Barrier(comm2D);
  }

  gatherResult(blockC, C, cut, sizeBlock, numProcess, rank, comm2D);

  tic += MPI_Wtime();

  if (rank == 0)
  {
    printf("------C------\n");
    if (gen)
    {
      cutMatrix(C, globalSize, &CC, realSize);
      printMatrix(CC, realSize);
    }
    else
      printMatrix(C, globalSize);
    printf("\n");
    printf("Calculated in %lf.\n", tic);
    free(A);
    free(B);
    free(C);
  }

  free(blockA);
  free(blockB);
  free(blockC);

  MPI_Finalize();
  return 0;
}
