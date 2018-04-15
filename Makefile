# Makefile

# Compilor
CC = mpicc

# Source
SRCS = matrix.c
SRCS += matrixUp.c
SRCS += naive.c

# Library
LIBS = -lm

# Grammar replacement
OBJS = $(SRCS:.c=.o)

# Target
all : $(OBJS)
		@echo "Compile completed."
		 chmod a+x ./run.sh                 

matrix.o : utils.h cannon.h
		$(CC) matrix.c $(LIBS) -o matrix.o

matrixUp.o : matrix_utils.h cannon_utils.h
		$(CC) matrixUp.c $(LIBS) -o matrixUp.o

naive.o : utils.h
		$(CC) naive.c $(LIBS) -o naive.o

		
.PHONY : clean
clean :
	@-rm $(all) $(OBJS)
