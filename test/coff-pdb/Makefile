all: example.obj example.pdb \
	member-function.obj member-function.pdb

example.obj: example.cpp Makefile
	del example.pdb
	cl /c /Od /Zi /Fo:$(@) /Fd:example.pdb %s
example.pdb: example.obj

member-function.obj: member-function.cpp Makefile
	del member-function.pdb
	cl /c /Od /Zi /Fo:$(@) /Fd:member-function.pdb %s
member-function.pdb: member-function.obj
