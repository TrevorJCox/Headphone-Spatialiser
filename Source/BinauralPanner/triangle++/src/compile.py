#!/usr/bin/python

import os,sys

OFLAGS='OFLAGS=-g -Wall'
CC=''

if os.system('which icpc >/dev/null 2>&1') == 0:
    CC = 'icpc\n'
    OFLAGS = '\nOFLAGS=-O\n'

if os.system('which g++ >/dev/null 2>&1') == 0:
    CC = 'g++\n'
    OFLAGS = '\nOFLAGS=-O3\n'

print ' Compiler Selected      ... ',CC,
CC = 'CC='+CC+'\n'

TFLAGS = 'TFLAGS=-DREDUCED -DANSI_DECLARATORS -DTRILIBRARY -DCDT_ONLY -DLINUX' 

if sys.platform == 'cygwin':
    TFLAGS += ' -DCYGWIN'

    

L = open("./src/makefile.input").readlines()
L.insert(0, '\n#--------------------------\n')
L.insert(0, TFLAGS)
L.insert(0, OFLAGS)
L.insert(0, CC)
L.insert(0, '\n#--------------------------\n')
f = open("./src/makefile.tmp",'w')
f.writelines(L)
f.close()

print ' Compiling Triangle++   ...',
os.system("make --quiet -f ./src/makefile.tmp; rm ./src/makefile.tmp")
print 'Done.'

