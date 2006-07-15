#!/bin/sh
# script to set MCPP to be called from gcc
# ./set_mcpp.sh ${gcc_path} ${gcc_maj_ver} ${gcc_min_ver} ${cpp_call} ${CC} \
#       ${CXX} x${EXEEXT} ${LN_S} ${inc_dir}

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

# remove ".exe" or such
EXEEXT=`echo $7 | sed 's/^x//'`
if test x${EXEEXT} != x; then
    cpp_base=`echo ${cpp_name} | sed "s/${EXEEXT}//"`
else
    cpp_base=${cpp_name}
fi

echo "  cd ${inc_dir}"
cd ${inc_dir}
if test ! -f mcpp_gcc${gcc_maj_ver}${gcc_min_ver}_predef_std.h; then
    echo "  generating mcpp_g*.h header files"
    echo '' | ${CC} -E -xc -dM - | sort | grep ' *#define *_'       \
            > mcpp_gcc${gcc_maj_ver}${gcc_min_ver}_predef_std.h
    echo '' | ${CC} -E -xc -dM - | sort | grep -E ' *#define *[A-Za-z]+'    \
            > mcpp_gcc${gcc_maj_ver}${gcc_min_ver}_predef_old.h
    echo '' | ${CXX} -E -xc++ -dM - | sort | grep ' *#define *_'    \
            > mcpp_gxx${gcc_maj_ver}${gcc_min_ver}_predef_std.h
    echo '' | ${CXX} -E -xc++ -dM - | sort | grep -E ' *#define *[A-Za-z]+' \
            > mcpp_gxx${gcc_maj_ver}${gcc_min_ver}_predef_old.h
fi

# write shell-script to call of 'cpp0', 'cc1 -E' or so is replaced to call of
# mcpp
echo "  cd ${cpp_path}"
cd ${cpp_path}
echo '#!/bin/sh'        >  mcpp.sh

# for GCC V.3.3 and later
if test x${cpp_base} = xcc1; then
    echo 'for i in $@'  >> mcpp.sh
    echo '    do'       >> mcpp.sh
    echo '    case $i in'           >> mcpp.sh
    echo '        -fpreprocessed)'          >> mcpp.sh
    echo "            ${cpp_path}/${cpp_base}_gnuc"' "$@"'  >> mcpp.sh
    echo '            exit ;;'      >> mcpp.sh
    echo '    esac'     >> mcpp.sh
    echo 'done'         >> mcpp.sh
    echo '#!/bin/sh'    >  mcpp_plus.sh
    echo 'for i in $@'  >> mcpp_plus.sh
    echo '    do'       >> mcpp_plus.sh
    echo '    case $i in'           >> mcpp_plus.sh
    echo '        -fpreprocessed)'  >> mcpp_plus.sh
    echo "            ${cpp_path}/cc1plus_gnuc"' "$@"'  >> mcpp_plus.sh
    echo '            exit ;;'      >> mcpp_plus.sh
    echo '    esac'     >> mcpp_plus.sh
    echo 'done'         >> mcpp_plus.sh
fi

# for GCC V.2, V.3 and V.4
echo ${cpp_path}/mcpp '"$@"'   >>  mcpp.sh
chmod a+x mcpp.sh
if test x${cpp_base} = xcc1; then
    echo ${cpp_path}/mcpp -+ '"$@"'  >> mcpp_plus.sh
    chmod a+x mcpp_plus.sh
fi

# backup GCC / cpp or cc1, cc1plus
if test `echo '' | ${cpp_call} -v - 2>&1 | grep 'MCPP' > /dev/null; echo $?`;
        then
    sym_link=`ls -l ${cpp_name} | sed 's/^l.*/l/; s/^[^l].*//'`
    if test x${sym_link} != xl; then
        echo "  mv ${cpp_name} ${cpp_base}_gnuc${EXEEXT}"
        mv -f ${cpp_name} ${cpp_base}_gnuc${EXEEXT}
        if test x${cpp_base} = xcc1; then
            echo "  mv cc1plus${EXEEXT} cc1plus_gnuc${EXEEXT}"
            mv -f cc1plus${EXEEXT} cc1plus_gnuc${EXEEXT}
        fi
    fi
fi
if test -f ${cpp_name}; then
    rm -f ${cpp_name}
    if test x${cpp_base} = xcc1; then
        rm -f cc1plus${EXEEXT}
    fi
fi

# make symbolic link of mcpp.sh to 'cpp0' or 'cc1', 'cc1plus'
echo "  ${LN_S} mcpp.sh ${cpp_name}"
${LN_S} mcpp.sh ${cpp_name}
if test x${cpp_base} = xcc1; then
    echo "  ${LN_S} mcpp_plus.sh cc1plus${EXEEXT}"
    ${LN_S} mcpp_plus.sh cc1plus${EXEEXT}
fi

if test x${gcc_maj_ver} = x2; then
    exit 0
fi

# for GCC V.3 or V.4 make ${CC}.sh and ${CXX}.sh to add -no-integrated-cpp
# option
echo "  cd ${gcc_path}"
cd ${gcc_path}

ref=${CC}${EXEEXT}
while ref=`readlink ${ref}`
do
    c_entity=${ref};
done
if test x${c_entity} != x; then     # symbolic linked file dereferenced
    if test ${c_entity} = ${CC}.sh; then    # gcc.sh already installed
        exit 0
    fi
    if test x${EXEEXT} != x; then
        c_entity_base=`echo ${c_entity} | sed "s/${EXEEXT}//"`
    else
        c_entity_base=${c_entity}
    fi
else                                # not symbolic link
    echo "  mv ${CC}${EXEEXT} ${CC}_proper${EXEEXT}"
    mv -f ${CC}${EXEEXT} ${CC}_proper${EXEEXT}
    c_entity_base=${gcc_path}/${CC}_proper
fi

ref=${CXX}${EXEEXT}
while ref=`readlink ${ref}`
do
    cxx_entity=${ref};
done
if test x${cxx_entity} != x; then      # symbolic linked file dereferenced
    if test ${cxx_entity} = ${CXX}.sh; then
        exit 0
    fi
    if test x${EXEEXT} != x; then
        cxx_entity_base=`echo ${cxx_entity} | sed "s/${EXEEXT}//"`
    else
        cxx_entity_base=${cxx_entity}
    fi
else
    echo "  mv ${CXX}${EXEEXT} ${CXX}_proper${EXEEXT}"
    mv -f ${CXX}${EXEEXT} ${CXX}_proper${EXEEXT}
    cxx_entity_base=${gcc_path}/${CXX}_proper
fi

echo '#!/bin/sh' > ${CC}.sh
echo ${c_entity_base} -no-integrated-cpp '"$@"'     \
        >> ${CC}.sh
echo '#!/bin/sh' > ${CXX}.sh
echo ${cxx_entity_base} -no-integrated-cpp '"$@"'   \
        >> ${CXX}.sh
chmod a+x ${CC}.sh ${CXX}.sh

echo "  ${LN_S} ${CC}.sh ${CC}"
${LN_S} -f ${CC}.sh ${CC}
echo "  ${LN_S} ${CXX}.sh ${CXX}"
${LN_S} -f ${CXX}.sh ${CXX}

