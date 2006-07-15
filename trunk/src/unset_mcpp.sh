#!/bin/sh
# script to set GNU CPP or CC1, CC1PLUS to be called from gcc
# ./unset_mcpp.sh ${gcc_path} ${gcc_maj_ver} ${gcc_min_ver} ${cpp_call}     \
#       ${CC} ${CXX} x${EXEEXT} ${LN_S} ${inc_dir}

gcc_maj_ver=$2
gcc_min_ver=$3
cpp_call=$4
CC=$5
CXX=$6
LN_S=$8
inc_dir=$9
cpp_name=`echo ${cpp_call} | sed 's,.*/,,'`
cpp_path=`echo ${cpp_call} | sed "s,/${cpp_name},,"`
gcc_path=`echo $1 | sed "s,/${CC}\$,,"`
EXEEXT=`echo $7 | sed 's/^x//'`
if test x${EXEEXT} != x; then
    cpp_base=`echo ${cpp_name} | sed "s/${EXEEXT}//"`
else
    cpp_base=${cpp_name}
fi

echo "  rm -f ${inc_dir}/mcpp_g*_predef_*.h"
rm -f ${inc_dir}/mcpp_g*_predef_*.h

echo "  cd ${cpp_path}"
cd ${cpp_path}

sym_link=`ls -l ${cpp_name} | sed 's/^l.*/l/; s/^[^l].*//'`;
if test x${sym_link} = xl && test -f ${cpp_base}_gnuc${EXEEXT}; then
    rm -f ${cpp_name}
    rm -f mcpp.sh
    echo "  mv ${cpp_base}_gnuc${EXEEXT} ${cpp_name}"
    mv -f ${cpp_base}_gnuc${EXEEXT} ${cpp_name}
    if test x${cpp_base} = xcc1; then
        rm -f cc1plus${EXEEXT}
        rm -f mcpp_plus.sh
        echo "  mv cc1plus_gnuc${EXEEXT} cc1plus${EXEEXT}"
        mv -f cc1plus_gnuc${EXEEXT} cc1plus${EXEEXT}
    fi
fi

if test x${gcc_maj_ver} = x2; then
    exit 0
fi

echo "  cd ${gcc_path}"
cd ${gcc_path}
c_sh=`readlink ${CC}`
if test x${c_sh} = x${CC}.sh; then
    if test `grep _proper ${CC}.sh > /dev/null; echo $? = 0`; then
        c_entity=`grep ${gcc_path} ${CC}.sh | sed "s,${gcc_path}/,,"  \
                | sed 's/_proper.*$//'`
        rm -f ${c_entity}
        echo "  mv ${c_entity}_proper${EXEEXT} ${c_entity}${EXEEXT}"
        mv -f ${c_entity}_proper${EXEEXT} ${c_entity}${EXEEXT}
    else
        c_entity=`grep integrated ${CC}.sh | sed "s, .*,,"`
        echo "  ${LN_S} ${c_entity} ${CC}"
        ${LN_S} -f ${c_entity} ${CC}
    fi
    rm -f ${CC}.sh
fi
cxx_sh=`readlink ${CXX}`
if test x${cxx_sh} = x${CXX}.sh; then
    if test `cat ${CXX}.sh | grep _proper > /dev/null; echo $? = 0`; then
        cxx_entity=`cat ${CXX}.sh | grep ${gcc_path} | sed "s,${gcc_path}/,,"  \
                | sed 's/_proper.*$//'`
        rm -f ${cxx_entity}
        echo "  mv ${cxx_entity}_proper${EXEEXT} ${cxx_entity}${EXEEXT}"
        mv -f ${cxx_entity}_proper${EXEEXT} ${cxx_entity}${EXEEXT}
    else
        cxx_entity=`grep integrated ${CXX}.sh | sed "s, .*,,"`
        echo "  ${LN_S} ${cxx_entity} ${CXX}"
        ${LN_S} -f ${cxx_entity} ${CXX}
    fi
    rm -f ${CXX}.sh
fi
