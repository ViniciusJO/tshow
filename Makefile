CFLAGS:=-Wall -Wextra -Wno-implicit-fallthrough -Wno-misleading-indentation -Wno-shift-negative-value -ggdb
INCLUDES:=-I/usr/include/SDL2
DEFINED:=-D_GNU_SOURCE=1 -D_REENTRANT 
LIBS:=-lraylib -lm -lX11 -linput -ludev -lXfixes -lSDL2 -lXext -lGL -lglfw
BINARY:=tshow

.PHONY: all run clear

all: ${BINARY}

${BINARY}: ${BINARY}.c nanovg/src/nanovg.c
	${CC} ${CFLAGS} ${INCLUDES} ${DEFINES} -o $@ $^ ${LIBS}

run: ${BINARY}
	./${BINARY}

clear:
	rm ${BINARY} 2> /dev/null
