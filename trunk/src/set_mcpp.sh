#!/bin/sh
# script to set MCPP to be called from gcc
# ./set_mcpp.sh ${gcc_path} ${gcc_maj_ver} ${gcc_min_ver} ${cpp_call} ${CC} \
#       ${CXX} x${EXEEXT} ${LN_S} ${inc_dir} ${host_system}

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

# remove ".exe" or such
EXEEXT=`echo $7 | sed 's/^x//'`
if test x${EXEEXT} != x; then
    cpp_base=`echo ${cpp_name} | sed "s/${EXEEXT}//"`
else
    cpp_base=${cpp_name}
fi

if test ${host_system} = SYS_MINGW && test ! -f cc1${EXEEXT}; then
    ## cc1.exe has not yet compiled
    echo "  do 'make COMPILER=GNUC mcpp cc1'; then do 'make COMPILER=GNUC install'"
    exit 1
fi

cwd=`pwd`

mkdir -p ${inc_dir}/mcpp-gcc
echo "  cd ${inc_dir}/mcpp-gcc"
cd ${inc_dir}/mcpp-gcc
if test ! -f gcc${gcc_maj_ver}${gcc_min_ver}_predef_std.h; then
    echo "  generating g*.h header files"
    ${CC} -E -xc -dM /dev/null | sort | grep ' *#define *_'       \
            > gcc${gcc_maj_ver}${gcc_min_ver}_predef_std.h
    ${CC} -E -xc -dM /dev/null | sort | grep -E ' *#define *[A-Za-z]+'    \
            > gcc${gcc_maj_ver}${gcc_min_ver}_predef_old.h
    ${CXX} -E -xc++ -dM /dev/null | sort | grep ' *#define *_'    \
            > gxx${gcc_maj_ver}${gcc_min_ver}_predef_std.h
    ${CXX} -E -xc++ -dM /dev/null | sort | grep -E ' *#define *[A-Za-z]+' \
            > gxx${gcc_maj_ver}${gcc_min_ver}_predef_old.h
fi
if test ${host_system} = SYS_CYGWIN; then
    cd ..
    mkdir -p mingw/mcpp-gcc
    cd mingw/mcpp-gcc
    if test ! -f gcc${gcc_maj_ver}${gcc_min_ver}_predef_std.h; then
        echo "  generating g*.h header files for cygwin/mingw"
        ${CC} -E -xc -dM -mno-cygwin /dev/null | sort |   \
                grep ' *#define *_'       \
                > gcc${gcc_maj_ver}${gcc_min_ver}_predef_std.h
        ${CC} -E -xc -dM -mno-cygwin /dev/null | sort |   \
                grep -E ' *#define *[A-Za-z]+'    \
                > gcc${gcc_maj_ver}${gcc_min_ver}_predef_old.h
        ${CXX} -E -xc++ -dM -mno-cygwin /dev/null | sort |    \
                grep ' *#define *_'    \
                > gxx${gcc_maj_ver}${gcc_min_ver}_predef_std.h
        ${CXX} -E -xc++ -dM -mno-cygwin /dev/null | sort |    \
                grep -E ' *#define *[A-Za-z]+' \
                > gxx${gcc_maj_ver}${gcc_min_ver}_predef_old.h
    fi
fi

# write shell-script to call of 'cpp0', 'cc1 -E' or so is replaced to call of
# mcpp
echo "  cd ${cpp_path}"
cd ${cpp_path}

# other than MinGW
if test ${host_system} != SYS_MINGW; then

    echo '#!/bin/sh'        >  mcpp.sh

    # for GCC V.3.3 and later
    if test x${cpp_base} = xcc1; then
        cat >> mcpp.sh <<_EOF
for i in \$@
    do
    case \$i in
        -fpreprocessed|-traditional*)
            ${cpp_path}/${cpp_base}_gnuc "\$@"
            exit ;;
    esac
done
_EOF
        cat > mcpp_plus.sh <<_EOF
#!/bin/sh
for i in \$@
    do
    case \$i in
        -fpreprocessed|-traditional*)
            ${cpp_path}/cc1plus_gnuc "\$@"
            exit ;;
    esac
done
_EOF
    fi
    
    # for GCC V.2, V.3 and V.4
    echo ${cpp_path}/mcpp '"$@"'   >>  mcpp.sh
    chmod a+x mcpp.sh
    if test x${cpp_base} = xcc1; then
        echo ${cpp_path}/mcpp -+ '"$@"'  >> mcpp_plus.sh
        chmod a+x mcpp_plus.sh
    fi
fi

# backup GCC / cpp or cc1, cc1plus
if test `${cpp_call} -v /dev/null 2>&1 | grep 'MCPP' > /dev/null; echo $?`;
        then
    if test ${host_system} = SYS_MINGW; then
        if test -f cc1_gnuc${EXEEXT}; then
            sym_link=l          ## cc1.exe already moved to cc1_gnuc.exe
        else
            sym_link=
        fi
    else
        sym_link=`ls -l ${cpp_name} | sed 's/^l.*/l/; s/^[^l].*//'`
    fi
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
if test ${host_system} = SYS_MINGW; then
    echo "  cp ${cwd}/cc1${EXEEXT}"
    cp ${cwd}/cc1${EXEEXT} .
    strip cc1${EXEEXT}
else
    echo "  ${LN_S} mcpp.sh ${cpp_name}"
    ${LN_S} mcpp.sh ${cpp_name}
fi
if test x${cpp_base} = xcc1; then
    if test ${host_system} = SYS_MINGW; then
        echo "  cp cc1${EXEEXT} cc1plus${EXEEXT}"
        cp cc1${EXEEXT} cc1plus${EXEEXT}
    else
        echo "  ${LN_S} mcpp_plus.sh cc1plus${EXEEXT}"
        ${LN_S} mcpp_plus.sh cc1plus${EXEEXT}
    fi
fi

if test x${gcc_maj_ver} = x2; then
    exit 0
fi

# for GCC V.3 or V.4 make ${CC}.sh and ${CXX}.sh to add -no-integrated-cpp
# option
echo "  cd ${gcc_path}"
cd ${gcc_path}

if test ${host_system} != SYS_MINGW; then
    ref=${CC}${EXEEXT}
    while ref=`readlink ${ref}`
    do
        c_entity=${ref};
    done
fi
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
    if test ! ${host_system} = SYS_MINGW || test ! -f ${CC}_proper${EXEEXT};
            then
        echo "  mv ${CC}${EXEEXT} ${CC}_proper${EXEEXT}"
        mv -f ${CC}${EXEEXT} ${CC}_proper${EXEEXT}
    fi
    c_entity_base=${gcc_path}/${CC}_proper
fi

if test ${host_system} != SYS_MINGW; then
    ref=${CXX}${EXEEXT}
    while ref=`readlink ${ref}`
    do
        cxx_entity=${ref};
    done
fi
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
    if test ${host_system} != SYS_MINGW || test ! -f ${CXX}_proper${EXEEXT};
            then
        echo "  mv ${CXX}${EXEEXT} ${CXX}_proper${EXEEXT}"
        mv -f ${CXX}${EXEEXT} ${CXX}_proper${EXEEXT}
    fi
    cxx_entity_base=${gcc_path}/${CXX}_proper
fi

echo '#!/bin/sh' > ${CC}.sh
echo ${c_entity_base} -no-integrated-cpp '"$@"' >> ${CC}.sh
echo '#!/bin/sh' > ${CXX}.sh
echo ${cxx_entity_base} -no-integrated-cpp '"$@"' >> ${CXX}.sh
chmod a+x ${CC}.sh ${CXX}.sh

echo "  ${LN_S} ${CC}.sh ${CC}"
${LN_S} -f ${CC}.sh ${CC}
echo "  ${LN_S} ${CXX}.sh ${CXX}"
${LN_S} -f ${CXX}.sh ${CXX}

