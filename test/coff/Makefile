all: small.obj

small.obj: small.cpp Makefile
	cl /c /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb
