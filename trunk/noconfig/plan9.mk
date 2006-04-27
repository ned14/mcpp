# mkfile for PLAN9 PCC to mk MCPP
# 		2004/02, 2005/02	kamtsui

</$objtype/mkfile

CC=pcc
LD=pcc
PREPROCESSED=0
CFLAGS= -DPREPROCESSED=$PREPROCESSED -c -FVw

TARG=mcpp_std

OFILES=main.$O\
	control.$O\
	eval.$O\
	expand.$O\
	support.$O\
	system.$O\
	mbchar.$O\
	lib.$O

HFILES=system.H\
	internal.H\
	noconfig.H

BIN=/$objtype/bin
</sys/src/cmd/mkone

preprocessed:V:
	mcpp_std preproc.c mcpp.H
	mk all

clean:V:
	rm -f *.$O $O.out mcpp.H

