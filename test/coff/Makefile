all: small.obj multiple-functions.obj primitives.obj primitive-parameters.obj

small.obj: small.cpp Makefile
	cl /c /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb

multiple-functions.obj: multiple-functions.cpp Makefile
	cl /c /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb

primitives.obj: primitives.cpp Makefile
	cl /c /Od /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb

primitive-parameters.obj: primitive-parameters.cpp Makefile
	cl /c /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb
