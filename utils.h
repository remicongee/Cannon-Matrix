#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int genMatrix(double ***_matrix, int _size[])
{
  int _row = _size[0], _col = _size[1];
  double *_test = (double *)malloc(sizeof(double) * _row * _col);
  if (!_test)
    return -1;

  (*_matrix) = (double **)malloc(sizeof(double *) * _row);
  if (!(*_matrix))
  {
    free(_test);
    return -1;
  }
  int _i, _j;
  for (_i = 0; _i < _row; _i++)
  {
    (*_matrix)[_i] = &(_test[_i * _col]);
    for (_j = 0; _j < _col; _j++)
    {
      // (*_matrix)[_i][_j] = (double)rand() / RAND_MAX * 10.0;
      (*_matrix)[_i][_j] = rand()/(float)(RAND_MAX/10);
    }
  }
  return 0;
}

double **readMatrix(char *_filePath, int _isSquare)
{
  FILE *_file = fopen(_filePath, "r");
  int _i, _j, _size = 0, _row = 0, _col;
  double _element;
  char _c;
  // printf("Start reading file.\n");
  while (fscanf(_file, "%lf", &_element) != -1)
  {
    // printf("%lf\n", _element);
    _c = fgetc(_file);
    if (_c == '\n' || _c == '\r')
    {
      _row += 1;
    }
    _size++;
  }
  _col = _size / _row;
  if (_col != _row && _isSquare)
  {
    printf("Error: Matrix should be sqaure (size %d).\n", _size);
    exit(1);
  }
  // printf("Start assigning matrix of %dx%d.\n", _size, _size);
  double **_matrix = (double **)malloc(sizeof(double *) * _row);
  fseek(_file, 0, SEEK_SET);
  for (_i = 0; _i < _row; _i++)
  {
    _matrix[_i] = (double *)malloc(sizeof(double) * _col);
    for (_j = 0; _j < _col; _j++)
    {
      fscanf(_file, "%lf", &_element);
      _matrix[_i][_j] = _element;
      // printf("%lf\t", _matrix[_i][_j]);
    }
    // printf("\n");
  }
  fclose(_file);
  return _matrix;
}

void printMatrix(double **_matrix, int _size[])
{
  if (_matrix == 0)
  {
    printf("Error: Can't print NULL matrix.\n");
    exit(1);
  }
  int _i, _j, _row = _size[0], _col = _size[1];
  for (_i = 0; _i < _row; _i++)
  {
    for (_j = 0; _j < _col; _j++)
    {
      printf("%f ", _matrix[_i][_j]);
    }
    printf("\n");
  }
}

double **mulMatrix(double **_A, double **_B, int _sizeA[], int _sizeB[])
{

  int _rowA = _sizeA[0], _colA = _sizeA[1], _rowB = _sizeB[0], _colB = _sizeB[1];
  if (_colA != _rowB)
  {
    printf("Error: Matrix multiply requires size match (%dx%d * %dx%d).\n", _rowA, _colA, _rowB, _colB);
    exit(1);
  }

  int _i, _j, _k;
  double **_res = (double **)malloc(sizeof(double *) * _rowA);
  for (_i = 0; _i < _rowA; _i++)
  {
    _res[_i] = (double *)malloc(sizeof(double) * _colB);
    for (_j = 0; _j < _colB; _j++)
    {
      for (_k = 0; _k < _colA; _k++)
      {
        _res[_i][_j] += _A[_i][_k] * _B[_k][_j];
      }
    }
  }
  return _res;
}

void transMatrix(double **_A, int _size[], double ***_AT)
{
  int _row = _size[0], _col = _size[1];
  double *_ptr = (double *)malloc(sizeof(double) * _row * _col);
  if (!_ptr)
  {
    printf("Error: fail to allocate matrix.");
    exit(1);
  }
  (*_AT) = (double **)malloc(sizeof(double *) * _col);
  if (!(*_AT))
  {
    free(_ptr);
    printf("Error: fail to allocate matrix.");
    exit(1);
  }
  for (int _i = 0; _i < _col; _i++)
  {
    (*_AT)[_i] = &(_ptr[_i * _row]);
    for (int _j = 0; _j < _row; _j++)
    {
      (*_AT)[_i][_j] = _A[_j][_i];
    }
  }
}

int padAux(int _rowA, int _colA, int _colB)
{
  if (_rowA >= _colA && _rowA >= _colB)
    return _rowA;
  if (_colA >= _rowA && _colA >= _colB)
    return _colA;
  if (_colB >= _rowA && _colB >= _colA)
    return _colB;
  // if(_rowA = _colA && _rowA > _colB) return ROWCOLA;
  // if(_colB = _colA && _colB > _rowA) return ROWCOLB;
}

void padMatrix(double **_A, double **_B, int _sizeA[], int _sizeB[], double ***_AP, double ***_BP)
{
  int _rowA = _sizeA[0], _colA = _sizeA[1], _colB = _sizeB[1],
      _pad = padAux(_rowA, _colA, _colB);
  // (*_nSize)[0] = (*_nSize)[1] = _pad;
  double *_ptrA = (double *)malloc(sizeof(double) * _pad * _pad);
  double *_ptrB = (double *)malloc(sizeof(double) * _pad * _pad);
  (*_AP) = (double **)malloc(sizeof(double *) * _pad);
  (*_BP) = (double **)malloc(sizeof(double *) * _pad);
  for (int _i = 0; _i < _pad; _i++)
  {
    (*_AP)[_i] = &(_ptrA[_i * _pad]);
    (*_BP)[_i] = &(_ptrB[_i * _pad]);
    for (int _j = 0; _j < _pad; _j++)
    {
      (*_AP)[_i][_j] = (_j <= _colA - 1 && _i <= _rowA - 1) ? _A[_i][_j] : 0;
      (*_BP)[_i][_j] = (_j <= _colB - 1 && _i <= _colA - 1) ? _B[_i][_j] : 0;
    }
  }
}

void cutMatrix(double **_A, int _sizeA[], double ***_C, int _sizeC[])
{
  int _rowA = _sizeA[0], _colA = _sizeA[1], \
      _rowC = _sizeC[0], _colC = _sizeC[1];
  if(_rowA < _rowC || _colA < _colC)
  {
    printf("Error: cut matrix wrong.\n");
    exit(1);
  }
  double *_ptr = (double *)malloc(sizeof(double) * _rowC * _colC);
  (*_C) = (double **)malloc(sizeof(double *) * _rowC);
  for(int _i = 0;_i < _rowC;_i++)
  {
    (*_C)[_i] = &(_ptr[_i * _colC]);
    for(int _j = 0;_j < _colC;_j++)
    {
      (*_C)[_i][_j] = _A[_i][_j];
    }
  }
}

void prepare(char *argv[], int *_cut, int *_sizeBlock, int **_sizeA, int **_sizeB, int **_globalSize, int *_gen, int **_realSize, int _numProcess, int _rank)
{
  (*_gen) = atoi(argv[1]);
  int _size;
  switch((*_gen))
  {
    case(0):
      (*_sizeA)[0] = (*_sizeB)[0] = (*_sizeA)[1] = (*_sizeB)[1] = _size = atoi(argv[2]);
      break;
    case(1):
      (*_sizeA)[0] = atoi(argv[2]), (*_sizeB)[0] = (*_sizeA)[1] = atoi(argv[3]), \
      (*_sizeB)[1] = atoi(argv[4]);
      _size = padAux((*_sizeA)[0], (*_sizeA)[1], (*_sizeB)[1]);
      (*_realSize)[0] = (*_sizeA)[0];
      (*_realSize)[1] = (*_sizeB)[1];
      break;
    default:
      printf("General option should be 0/1.\n");
      exit(1);
      break;
  }
  if (_size <= 1)
  {
    if (_rank == 0)
      printf("Matrix size should be positive.\n");
    exit(1);
  }
  if ((int)sqrt(_numProcess) != sqrt((double)_numProcess))
  {
    if (_rank == 0)
      printf("Process number should be square.\n");
    exit(1);
  }
  *_cut = (int)(sqrt(_numProcess));
  if (_size % (*_cut) != 0)
  {
    if (_rank == 0)
      printf("Process number does not match matrix size.\n");
    exit(1);
  }
  if (_rank == 0)
    printf("Start calculating matrix(%dx%d) multiplication with %d process.\n", _size, _size, _numProcess);
  *_sizeBlock = _size / (*_cut);
  (*_globalSize)[0] = (*_globalSize)[1] = _size;
}
