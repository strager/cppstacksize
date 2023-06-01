all: \
	temporary.pdb \
	multi-obj.pdb

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
