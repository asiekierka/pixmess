
# TODO: provide an optional MinGW set of... things
# (hint: -static-libgcc will be helpful here!)
#
# OK, this is my commandline... --GM
# make CC=i686-pc-mingw32-gcc CFLAGS=-I/usr/i686-pc-mingw32/sys-root/mingw/include/SDL/ BINNAME=c64pixels.exe

LIBS = `sdl-config --libs` -lz
CFLAGS = -g `sdl-config --cflags` -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -g
BINNAME = pixmess
PLATFORM = sdl
RENDERER = $(PLATFORM)

OBJS = main.o event.o event_$(PLATFORM).o fixme.o map.o misc_$(PLATFORM).o player.o \
	render.o render_$(RENDERER).o tile.o interface.o network.o client.o server.o \
	map_rw.o

INCLUDES = common.h render.h misc.h render_data.h player.h tile.h event.h map.h \
	interface.h network.h client.h server.h types.h
all: $(BINNAME)

$(BINNAME): $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: all clean

clean:
	rm -f $(OBJS) $(BINNAME)

