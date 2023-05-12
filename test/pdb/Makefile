all: example.pdb

example.dll: example.cpp Makefile
	del example.pdb
	cl /Od /Zi /LD /MD /Fe:$(@) /Fo:example.obj /Fd:example.pdb %s /link /INCREMENTAL:NO /OPT:REF
	del example.exp
	del example.lib
example.obj: example.dll
example.pdb: example.dll
