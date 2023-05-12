all: call.obj small.obj multiple-functions.obj primitives.obj primitive-parameters.obj primitive-parameters-call.obj

call.obj: call.cpp Makefile
	cl /c /O2 /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb

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

primitive-parameters-call.obj: primitive-parameters-call.cpp Makefile
	cl /c /O2 /Zi /Fo:$(@) /Fd:$(@).pdb %s
	del $(@).pdb
