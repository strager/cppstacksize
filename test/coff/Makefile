all: call.obj small.obj multiple-functions.obj primitives.obj primitive-parameters.obj primitive-parameters-call.obj \
	parameters-int-0-callee.obj parameters-int-0-caller.obj \
	parameters-int-1-callee.obj parameters-int-1-caller.obj \
	parameters-int-2-callee.obj parameters-int-2-caller.obj \
	parameters-int-3-callee.obj parameters-int-3-caller.obj \
	parameters-int-4-callee.obj parameters-int-4-caller.obj \
	parameters-int-5-callee.obj parameters-int-5-caller.obj \
	parameters-int-6-callee.obj parameters-int-6-caller.obj \
	pointer.obj \
	struct.obj \
	union.obj \
	enum.obj \
	typedef.obj \
	function-type.obj \
	array.obj \
	alloca.obj \
	member-function.obj

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

parameters-int-0-callee.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=0 /DCALLEE %s
parameters-int-0-caller.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=0 /DCALLER %s

parameters-int-1-callee.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=1 /DCALLEE %s
parameters-int-1-caller.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=1 /DCALLER %s

parameters-int-2-callee.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=2 /DCALLEE %s
parameters-int-2-caller.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=2 /DCALLER %s

parameters-int-3-callee.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=3 /DCALLEE %s
parameters-int-3-caller.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=3 /DCALLER %s

parameters-int-4-callee.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=4 /DCALLEE %s
parameters-int-4-caller.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=4 /DCALLER %s

parameters-int-5-callee.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=5 /DCALLEE %s
parameters-int-5-caller.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=5 /DCALLER %s

parameters-int-6-callee.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=6 /DCALLEE %s
parameters-int-6-caller.obj: parameters-int.cpp Makefile
	cl /c /O2 /Z7 /Fo:$(@) /DPARAM_COUNT=6 /DCALLER %s

pointer.obj: pointer.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

struct.obj: struct.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

union.obj: union.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

enum.obj: enum.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

typedef.obj: typedef.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

function-type.obj: function-type.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

array.obj: array.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

alloca.obj: alloca.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s

member-function.obj: member-function.cpp Makefile
	cl /c /Od /Z7 /Fo:$(@) %s
