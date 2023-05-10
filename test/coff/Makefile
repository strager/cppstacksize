all: small.obj primitives.obj

small.obj: small.cpp Makefile
	cl /c /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb

primitives.obj: primitives.cpp Makefile
	cl /c /Od /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb
