#include <math.h>
#include "mpi.h"
#include "utils.h"

int genBlock(double*** _block, int _sizeBlock)
{
  double* _test = (double*)malloc(sizeof(double) * _sizeBlock * _sizeBlock);
  if (!_test) return -1;

  (*_block) = (double**)malloc(sizeof(double*) * _sizeBlock);
  if (!(*_block)) {
     free(_test);
     return -1;
  }
  int _i, _j;
  for (_i = 0;_i < _sizeBlock;_i++)
  {
      (*_block)[_i] = &(_test[_i * _sizeBlock]);
      for(_j = 0;_j < _sizeBlock;_j++)
      {
        (*_block)[_i][_j] = 0.0;
      }
  }
  return 0;
}

MPI_Datatype setBlockType(int _sizeBlock, int _cut)
{
  int _globalSize[2] = {_sizeBlock * _cut, _sizeBlock * _cut}, \
      _subSize[2] = {_sizeBlock, _sizeBlock}, _start[2] = {0, 0};
  MPI_Datatype _type, _resizedType;
  MPI_Type_create_subarray(2, _globalSize, _subSize, _start, MPI_ORDER_C, MPI_DOUBLE, &_type);
  MPI_Type_create_resized(_type, 0, _sizeBlock * sizeof(double), &_resizedType);
  MPI_Type_commit(&_resizedType);
  return _resizedType;
}

void cartTopo(int _cut, MPI_Comm _comm, MPI_Comm* _comm2D)
{
  int  _dims[2], _periods[2], _ndim = 2, _reorder = 1;
  _dims[0] = _dims[1] = _cut;
  _periods[0] = _periods[1] = 1;
  MPI_Cart_create(_comm, _ndim, _dims, _periods, _reorder, _comm2D);
}

void getRank2D(int _rank, MPI_Comm _comm2D, int* _rank2D)
{
  int _ndim = 2;
  MPI_Cart_coords(_comm2D, _rank, _ndim, _rank2D);
}

void canScatter(double** _matrix, double** _block, int _numProcess, int _sizeBlock, int _rank, MPI_Comm _comm2D)
{
  int _sendCounts[_numProcess];
  int _displace[_numProcess];
  int _i, _rank2D[2], _cut = sqrt(_numProcess), _areaBlock = _sizeBlock * _sizeBlock;
  if(_rank == 0)
  {
    for(_i = 0;_i < _numProcess;_i++)
    {
      _sendCounts[_i] = 1;
      getRank2D(_i, _comm2D, _rank2D);
      // MPI_Cart_coords(_comm2D, _rank, 2, _rank2D);
      // _displace[_i] = _cut * (_cut - _rank2D[1] - 1) * _sizeBlock + _rank2D[0];
      _displace[_i] = _cut * _rank2D[0] * _sizeBlock + _rank2D[1];
      // printf("Process %d rank (%d %d) dispalce %d\n", _i, _rank2D[0], _rank2D[1], _displace[_i]);
    }
  }
  MPI_Barrier(_comm2D);
  MPI_Datatype _type = setBlockType(_sizeBlock, _cut);
  double* _globalPointer = NULL;
  if(_rank == 0) _globalPointer = &(_matrix[0][0]);
  MPI_Scatterv(_globalPointer, _sendCounts, _displace, _type, &(_block[0][0]), _sizeBlock * _sizeBlock, MPI_DOUBLE, 0, _comm2D);
}

void canRotate(int _rank, int* _rank2D, double** _blockA, double** _blockB, int _sizeBlock, MPI_Comm _comm2D)
{
  int _left, _right, _top, _botton, _source, _dest, \
      _sideways = 0, _upDown = 1;
  MPI_Cart_shift(_comm2D, _sideways, 1, &_left, &_right);
  MPI_Cart_shift(_comm2D, _upDown, 1, &_botton, &_top);
  // if(_rank == 3)
  // {
  //   printf("(left %d right %d)\n", _left, _right);
  //   printf("(botton %d up %d)\n", _botton, _top);
  //   printf("(source %d dest %d)\n", _source, _dest);
  // }
  MPI_Status _status;
  MPI_Sendrecv_replace((&_blockA)[0][0], _sizeBlock * _sizeBlock, MPI_DOUBLE, _botton, 1, _top, 1, _comm2D, &_status);

  MPI_Cart_shift(_comm2D, _upDown, 1, &_botton, &_top);
  MPI_Sendrecv_replace((&_blockB)[0][0], _sizeBlock * _sizeBlock, MPI_DOUBLE, _left, 1, _right, 1, _comm2D, &_status);
}

void canMul(double** _blockA, double** _blockB, double*** _blockC, int _sizeBlock, int _rank)
{
  int _i, _j, _k;
  for(_i = 0;_i < _sizeBlock;_i++)
  {
    for(_j = 0;_j < _sizeBlock;_j++)
    {
      for(_k = 0;_k < _sizeBlock;_k++)
      {
        // if(_rank == 1) printf("%lfx%lf->%lf\n", _blockA[_i][_k], _blockB[_k][_j],(*_blockC)[_i][_j]);
        (*_blockC)[_i][_j] += _blockA[_i][_k] * _blockB[_k][_j];
      }
    }
  }
  // printf("%lf\n", (*_blockC)[_i][_j]);
}

void gatherResult(double** _local, double** _global, int _cut, int _sizeBlock, int _numProcess, int _rank, MPI_Comm _comm2D)
{
  int _i, _recvCount[_numProcess], _displace[_numProcess], _rank2D[2];
  if(_rank == 0)
  {
    for(_i = 0;_i < _numProcess;_i++)
    {
      _recvCount[_i] = 1;
      getRank2D(_i, _comm2D, _rank2D);
      // MPI_Cart_coords(_comm2D, _rank, 2, _rank2D);
      _displace[_i] = _cut * _rank2D[0] * _sizeBlock + _rank2D[1];
      // printf("Process %d rank (%d %d) dispalce %d\n", _i, _rank2D[0], _rank2D[1], _displace[_i]);
    }
  }
  MPI_Barrier(_comm2D);
  MPI_Datatype _type = setBlockType(_sizeBlock, _cut);
  double* _globalPointer = NULL;
  if(_rank == 0) _globalPointer = &(_global[0][0]);
  MPI_Gatherv((&_local)[0][0], _sizeBlock * _sizeBlock, MPI_DOUBLE, _globalPointer, _recvCount, _displace, _type, 0, _comm2D);
}
