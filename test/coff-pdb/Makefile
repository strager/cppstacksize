all: example.obj example.pdb

example.obj: example.cpp Makefile
	del example.pdb
	cl /c /Od /Zi /Fo:$(@) /Fd:example.pdb %s
example.pdb: example.obj
