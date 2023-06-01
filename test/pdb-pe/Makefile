all: \
	temporary.pdb

temporary.dll: temporary.c Makefile
	del temporary.pdb
	cl /Od /Zi /GS- /LD /MD /Fe:$(@) /Fo:temporary.obj /Fd:temporary.pdb %s /link /INCREMENTAL:NO /OPT:REF /NODEFAULTLIB /ENTRY:_start /SUBSYSTEM:CONSOLE /NOIMPLIB /NOEXP
temporary.obj: temporary.dll
temporary.pdb: temporary.dll
