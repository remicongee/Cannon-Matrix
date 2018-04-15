hosts="$1"

echo -e "\033[36m[0/1 required]\033[0mNeed general matrix multiplication:"
echo -e "\033[36m0--square case, 1--general case.\033[0m"
read gen

if [ $gen -eq 0 ]
  then
  echo "Square matrix multiplication confirmed."
  echo -e "\033[36m[One positive integer required]\033[0mPlease set square size:"
  read size
elif [ $gen -eq 1 ]
  then
  echo "General matrix multiplication confirmed."
  echo -e "\033[36m[Three positive integers required]\033[0mPlease set matrices size:"
  echo -e "\033[36mExample: input [M N S] will calculate MxN matrix multiplied NxS matrix.\033[0m"
  read -p "M N S:" m n s
  size=$m
  if [ $size -lt $n ]
    then
    size=$n
  fi
  if [ $size -lt $s ]
    then
    size=$s
  fi
else
  echo -e "\033[31mGeneral option should be 0/1.\033[0m"
  exit 1
fi

echo -e "\033[36m[One positive integer required]\033[0mPlease set process number:"
read num

if [ $num -le 0 ]
  then
  echo -e "\033[31mProcess number should be strictly positive.\033[0m"
  exit 1
fi

sqnum=$( echo "sqrt($num)" | bc )
mod=$(( $size % $sqnum ))
if [ $mod -eq 0 ]
  then
  echo "Divisible case confirmed."
  code="./matrix.o"
else
  echo "Non-divisible case confirmed."
  code="./matrixUp.o"
fi

if [ ! -e $code ]
  then
  echo -e "\033[31mNot yet compiled.\033[0m"
  exit 1
fi

mult=0
if [ -e $hosts ]
  then
  mult=1
else
  echo "Single node is confirmed."
fi

if [ $mult -eq 1 ]
  then
  if [ $gen -eq 1 ]
    then
    mpirun -n $num -f $hosts $code $gen $m $n $s
  else
    mpirun -n $num -f $hosts $code $gen $size
  fi  
else
  if [ $gen -eq 1 ]
    then
    mpirun -n $num $code $gen $m $n $s
  else
    mpirun -n $num $code $gen $size
  fi
fi
