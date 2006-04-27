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

echo "  cp -f mcpp_g*${gcc_maj_ver}${gcc_min_ver}_predef_*.h ${inc_dir}"
cp -f mcpp_g*${gcc_maj_ver}${gcc_min_ver}_predef_*.h ${inc_dir}

# write shell-script to call of 'cpp0', 'cc1 -E' or so is replaced to call of
# mcpp_std
echo "  cd ${cpp_path}"
cd ${cpp_path}
echo '#!/bin/sh'        >  mcpp.sh

# for GNU C V.2, V.3.3 and later
if test x${gcc_maj_ver} = x2 || test x${cpp_base} = xcc1; then
    echo 'for i in $@'  >> mcpp.sh
    echo '    do'       >> mcpp.sh
    echo '    case $i in'           >> mcpp.sh
    echo '        -traditional*| -fpreprocessed)'           >> mcpp.sh
    echo "            ${cpp_path}/${cpp_base}_gnuc"' "$@"'  >> mcpp.sh
    echo '            exit ;;'      >> mcpp.sh
    echo '    esac'     >> mcpp.sh
    echo 'done'         >> mcpp.sh
fi

# for GNU C V.3.3 and later
if test x${cpp_base} = xcc1; then
    echo '#!/bin/sh'    >  mcpp_plus.sh
    echo 'for i in $@'  >> mcpp_plus.sh
    echo '    do'       >> mcpp_plus.sh
    echo '    case $i in'           >> mcpp_plus.sh
    echo '        -traditional*| -fpreprocessed)'       >> mcpp_plus.sh
    echo "            ${cpp_path}/cc1plus_gnuc"' "$@"'  >> mcpp_plus.sh
    echo '            exit ;;'      >> mcpp_plus.sh
    echo '    esac'     >> mcpp_plus.sh
    echo 'done'         >> mcpp_plus.sh
fi

# for GNU C V.2 and V.3
echo ${cpp_path}/mcpp_std -23j '"$@"'   >>  mcpp.sh
chmod a+x mcpp.sh
if test x${cpp_base} = xcc1; then
    echo ${cpp_path}/mcpp_std -+23j '"$@"'  >> mcpp_plus.sh
    chmod a+x mcpp_plus.sh
fi

# backup GNU C / cpp or cc1, cc1plus
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

# for GNU C V.3 make ${CC}.sh and ${CXX}.sh to add -no-integrated-cpp option
echo "  cd ${gcc_path}"
cd ${gcc_path}

sym_link=`ls -l ${CC}${EXEEXT} | sed 's/^l.*/l/; s/^[^l].*//'`
if test x${sym_link} = xl; then     # symbolic link
    clink=`ls -L -i ${CC}${EXEEXT}`                         # dereference
    inode=`echo ${clink} | sed 's/^ *//' | sed 's/ .*//'`
    c_entity=`ls -i | grep ${inode} | sed '1p; 1,$d' | sed 's/^[ 0-9]*//'   \
            | sed 's/\*$//'`
    if test x${c_entity} = x${CC}.sh; then
        exit 0
    fi
    if test x${EXEEXT} != x; then
        c_entity_base=`echo ${c_entity} | sed "s/${EXEEXT}//"`
    else
        c_entity_base=${c_entity}
    fi
    echo "  mv ${c_entity} ${c_entity_base}_proper${EXEEXT}"
    mv -f ${c_entity} ${c_entity_base}_proper${EXEEXT}
else                                # not symbolic link
    echo "  mv ${CC}${EXEEXT} ${CC}_proper${EXEEXT}"
    mv -f ${CC}${EXEEXT} ${CC}_proper${EXEEXT}
    c_entity=${CC}${EXEEXT}
fi

sym_link=`ls -l ${CXX}${EXEEXT} | sed 's/^l.*/l/; s/^[^l].*//'`
if test x${sym_link} = xl; then     # symbolic link
    clink=`ls -L -i ${CXX}${EXEEXT}`
    inode=`echo ${clink} | sed 's/^ *//' | sed 's/ .*//'`
    cxx_entity=`ls -i | grep ${inode} | sed '1p; 1,$d' | sed 's/^[ 0-9]*//' \
            | sed 's/\*$//'`
    if test x${cxx_entity} = x${CXX}.sh; then
        exit 0
    fi
    if test x${EXEEXT} != x; then
        cxx_entity_base=`echo ${cxx_entity} | sed "s/${EXEEXT}//"`
    else
        cxx_entity_base=${cxx_entity}
    fi
    echo "  mv ${cxx_entity} ${cxx_entity_base}_proper${EXEEXT}"
    mv -f ${cxx_entity} ${cxx_entity_base}_proper${EXEEXT}
else
    echo "  mv ${CXX}${EXEEXT} ${CXX}_proper${EXEEXT}"
    mv -f ${CXX}${EXEEXT} ${CXX}_proper${EXEEXT}
    cxx_entity=${CXX}${EXEEXT}
fi

echo '#!/bin/sh' > ${CC}.sh
echo ${gcc_path}/${c_entity}_proper -no-integrated-cpp '"$@"'   >> ${CC}.sh
echo '#!/bin/sh' > ${CXX}.sh
echo ${gcc_path}/${cxx_entity}_proper -no-integrated-cpp '"$@"' >> ${CXX}.sh
chmod a+x ${CC}.sh ${CXX}.sh

echo "  ${LN_S} ${CC}.sh ${c_entity}"
${LN_S} ${CC}.sh ${c_entity}
echo "  ${LN_S} ${CXX}.sh ${cxx_entity}"
${LN_S} ${CXX}.sh ${cxx_entity}

