#!/bin/sh
## ./after_test.sh ${CC} ${gcc_path} ${cpp_call}

CC=$1
cpp_name=`echo $3 | sed 's,.*/,,'`
cpp_path=`echo $3 | sed "s,/${cpp_name},,"`
#cur_dir=`pwd`

echo "  cd ${cpp_path}"
cd "${cpp_path}"
echo "  removing '-23j' options from mcpp invocation"
for i in mcpp*.sh
do
    cat $i | sed 's/mcpp -23j/mcpp/' > tmp
    mv -f tmp $i
    chmod a+x $i
done

if test ${CC} = gcc; then
#    echo "  cd ${cur_dir}"
#    cd "${cur_dir}"
    exit 0
fi

gcc_path=`expr $2 : "\(.*\)/${CC}\$"`
echo "  cd ${gcc_path}"
cd "${gcc_path}"
echo "  rm gcc"
rm gcc
if test -f "gcc.save"; then
    echo "  mv gcc.save gcc"
    mv gcc.save gcc
fi
#echo "  cd ${cur_dir}"
#cd "${cur_dir}"
