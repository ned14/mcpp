#! /bin/sh
# script to set GNU CPP or CC1, CC1PLUS to be called from gcc
# ./unset_mcpp.sh ${gcc_path} ${gcc_maj_ver} ${gcc_min_ver} ${cpp_call}     \
#       ${CC} ${CXX} x${EXEEXT} ${inc_dir}

gcc_maj_ver=$2
gcc_min_ver=$3
cpp_call=$4
CC=$5
CXX=$6
inc_dir=$8
cpp_name=`echo ${cpp_call} | sed 's,.*/,,'`
cpp_path=`echo ${cpp_call} | sed "s,/${cpp_name},,"`
gcc_path=`echo $1 | sed "s,/${CC}\$,,"`
EXEEXT=`echo $7 | sed 's/^x//'`
if test x${EXEEXT} != x; then
    cpp_base=`echo ${cpp_name} | sed "s/${EXEEXT}//"`
else
    cpp_base=${cpp_name}
fi

echo "  rm -f ${inc_dir}/mcpp_g*${gcc_maj_ver}${gcc_min_ver}_predef_*.h"
rm -f ${inc_dir}/mcpp_g*${gcc_maj_ver}${gcc_min_ver}_predef_*.h

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
sym_link=`ls -l ${CC}${EXEEXT} | sed 's/^l.*/l/; s/^[^l].*//'`
if test x${sym_link} = xl; then     # symbolic link
    clink=`ls -L -i ${CC}${EXEEXT}`                         # dereference
    inode=`echo ${clink} | sed 's/^ *//' | sed 's/ .*//'`
    c_sh=`ls -i | grep ${inode} | sed 's/^[ 0-9]*//' | sed 's/\*$//'`
    if test x${c_sh} = x${CC}.sh; then
        c_entity=`cat ${CC}.sh | grep ${gcc_path} | sed "s,${gcc_path}/,,"  \
                | sed 's/_proper.*$//'`
        rm -f ${CC}.sh
        echo "  mv ${c_entity}_proper${EXEEXT} ${c_entity}${EXEEXT}"
        mv -f ${c_entity}_proper${EXEEXT} ${c_entity}${EXEEXT}
    fi
fi
sym_link=`ls -l ${CXX} | sed 's/^l.*/l/; s/^[^l].*//'`
if test x${sym_link} = xl; then     # symbolic link
    clink=`ls -L -i ${CXX}${EXEEXT}`
    inode=`echo ${clink} | sed 's/^ *//' | sed 's/ .*//'`
    cxx_sh=`ls -i | grep ${inode} | sed 's/^[ 0-9]*//' | sed 's/\*$//'`
    if test x${cxx_sh} = x${CXX}.sh; then
        cxx_entity=`cat ${CXX}.sh | grep ${gcc_path}    \
                | sed "s,${gcc_path}/,," | sed 's/_proper.*$//'`
        rm -f ${CXX}.sh
        echo "  mv ${cxx_entity}_proper${EXEEXT} ${cxx_entity}${EXEEXT}"
        mv -f ${cxx_entity}_proper${EXEEXT} ${cxx_entity}${EXEEXT}
    fi
fi

