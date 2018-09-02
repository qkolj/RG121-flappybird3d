PROGRAM = flappybird3D
CC      = gcc
CFLAGS  = -g -ansi -Wall -I/usr/X11R6/include -I/usr/pkg/include
LDFLAGS = -L/usr/X11R6/lib -L/usr/pkg/lib
LDLIBS  = -lglut -lGLU -lGL -lm

$(PROGRAM): source/main.o source/image.o
	$(CC) $(LDFLAGS) -o $(PROGRAM) $^ $(LDLIBS)

.PHONY: clean

clean:
	-rm source/*.o $(PROGRAM)
