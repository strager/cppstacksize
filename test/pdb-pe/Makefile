all: \
	temporary.pdb \
	multi-obj.pdb \
	line-numbers.pdb

temporary.dll: temporary.c Makefile
	del temporary.pdb
	cl /Od /Zi /GS- /LD /MD /Fe:$(@) /Fo:temporary.obj /Fd:temporary.pdb %s /link /INCREMENTAL:NO /OPT:REF /NODEFAULTLIB /ENTRY:_start /SUBSYSTEM:CONSOLE /NOIMPLIB /NOEXP
temporary.obj: temporary.dll
temporary.pdb: temporary.dll

multi-obj.dll: multi-obj-1.c multi-obj-2.c Makefile
	del multi-obj.pdb
	cl /Od /Zi /GS- /LD /MD /Fe:$(@) /Fd:multi-obj.pdb multi-obj-1.c multi-obj-2.c /link /INCREMENTAL:NO /OPT:REF /NODEFAULTLIB /ENTRY:_start /SUBSYSTEM:CONSOLE /NOIMPLIB /NOEXP
multi-obj.obj: multi-obj.dll
multi-obj.pdb: multi-obj.dll

line-numbers.dll: line-numbers.c line-numbers.inc Makefile
	del line-numbers.pdb
	cl /Od /Zi /GS- /LD /MD /Fe:$(@) /Fo:line-numbers.obj /Fd:line-numbers.pdb %s /link /INCREMENTAL:NO /OPT:REF /NODEFAULTLIB /ENTRY:_start /SUBSYSTEM:CONSOLE /NOIMPLIB /NOEXP
line-numbers.obj: line-numbers.dll
line-numbers.pdb: line-numbers.dll
