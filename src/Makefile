
CC = gcc
CFLAGS = -O2 -Wall -fPIC $(shell pkg-config --cflags unity grilo-0.2) -DGETTEXT_PACKAGE=\"unity-scope-grilo\"
LDFLAGS = -shared -Wl,-no-undefined -z defs
LIBS = $(shell pkg-config --libs unity grilo-0.2)

unity-scope-hollywood.so: main.o base-scope.o music-scope.o video-scope.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

main.o: main.c scope.h
base-scope.o: base-scope.c scope.h
music-scope.o: music-scope.c scope.h
video-scope.o: video-scope.c scope.h

.c.o:
	$(CC) $(CFLAGS) -c $<

run: ./unity-scope-hollywood.so
	LD_LIBRARY_PATH=/home/james/src/unity/prefix/lib:/home/james/src/unity/prefix/lib/x86_64-linux-gnu unity-scope-loader -m `pwd`/$<

.PHONY: run
