#!/bin/sh
# script to set GNU CPP or CC1, CC1PLUS to be called from gcc
# ./unset_mcpp.sh ${gcc_path} ${gcc_maj_ver} ${gcc_min_ver} ${cpp_call}     \
#       ${CC} ${CXX} x${EXEEXT} ${LN_S} ${inc_dir} ${host_system}

gcc_maj_ver=$2
gcc_min_ver=$3
cpp_call=$4
CC=$5
CXX=$6
LN_S=$8
inc_dir=$9
host_system=${10}
cpp_name=`echo ${cpp_call} | sed 's,.*/,,'`
cpp_path=`echo ${cpp_call} | sed "s,/${cpp_name},,"`
gcc_path=`echo $1 | sed "s,/${CC}\$,,"`
EXEEXT=`echo $7 | sed 's/^x//'`
if test x${EXEEXT} != x; then
    cpp_base=`echo ${cpp_name} | sed "s/${EXEEXT}//"`
else
    cpp_base=${cpp_name}
fi

echo "  rm -fr ${inc_dir}/mcpp-gcc*"
rm -fr ${inc_dir}/mcpp-gcc*
if test x${host_system} = xSYS_CYGWIN; then
    echo "  rm -fr ${inc_dir}/mingw/mcpp-gcc"
    rm -fr ${inc_dir}/mingw/mcpp-gcc
fi

echo "  cd ${cpp_path}"
cd ${cpp_path}

if (test -h ${cpp_name} || test x${host_system} = xSYS_MINGW)  \
        && test -f ${cpp_base}_gnuc${EXEEXT}; then
    rm -f ${cpp_name} mcpp.sh
    echo "  mv ${cpp_base}_gnuc${EXEEXT} ${cpp_name}"
    mv -f ${cpp_base}_gnuc${EXEEXT} ${cpp_name}
    if test x${cpp_base} = xcc1; then
        rm -f cc1plus${EXEEXT} mcpp_plus.sh
        echo "  mv cc1plus_gnuc${EXEEXT} cc1plus${EXEEXT}"
        mv -f cc1plus_gnuc${EXEEXT} cc1plus${EXEEXT}
    fi
fi

if test x${gcc_maj_ver} = x2; then
    exit 0
fi

echo "  cd ${gcc_path}"
cd ${gcc_path}

for cc in ${CC} ${CXX}
do
    if test x${host_system} != xSYS_MINGW; then
        ref=`readlink ${cc}`
    fi
    if test x${ref} = x${cc}.sh || test x${host_system} = xSYS_MINGW; then
        entity=`grep ${gcc_path} ${cc}.sh | sed "s,${gcc_path}/,,"  \
                | sed 's/_proper.*$//'`
        rm -f ${entity}
        echo "  mv ${entity}_proper${EXEEXT} ${entity}${EXEEXT}"
        mv -f ${entity}_proper${EXEEXT} ${entity}${EXEEXT}
        if test ${entity} != ${cc}; then
            echo "  ${LN_S} ${entity} ${cc}"
            ${LN_S} -f ${entity} ${cc}
        fi
        rm -f ${cc}.sh
    fi
done
