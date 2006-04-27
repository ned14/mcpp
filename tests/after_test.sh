#!/bin/sh
## ./after_test.sh ${CC} ${gcc_path}

CC=$1
if test ${CC} = gcc; then
    exit 0
fi

gcc_path=`expr $2 : "\(.*\)/${CC}\$"`
cur_dir=`pwd`
echo "  cd ${gcc_path}"
cd "${gcc_path}"
echo "  rm gcc"
rm gcc
if test -f "gcc.save"; then
    echo "  mv gcc.save gcc"
    mv gcc.save gcc
fi
echo "  cd ${cur_dir}"
cd "${cur_dir}"
