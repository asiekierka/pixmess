
# TODO: provide an optional MinGW set of... things
# (hint: -static-libgcc will be helpful here!)
#
# OK, this is my commandline...
# make CC=i686-pc-mingw32-gcc BINNAME=pixmess.exe CFLAGS="-Iluainc -Isdlinc -D_GNU_SOURCE=1 -Dmain=SDL_main -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter" LIBS="-L. -lmingw32 -lSDLmain -lSDL -lz -llua -lwsock32"
# Yes, I have copied the lua/SDL/zlib stuff into dirs.
# --GM

LIBS = `sdl-config --libs` -lz -llua
CFLAGS = -g `sdl-config --cflags` -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable $(CFLAGS_EXTRA)
LDFLAGS = -g $(LDFLAGS_EXTRA)
BINNAME = pixmess
PLATFORM = sdl
RENDERER = $(PLATFORM)

OBJS = main.o event.o event_$(PLATFORM).o fixme.o lua.o map.o misc_$(PLATFORM).o player.o \
	render.o render_$(RENDERER).o tile.o interface.o network.o client.o server.o \
	map_rw.o physics.o chat.o

INCLUDES = common.h render.h misc.h render_data.h player.h tile.h event.h map.h \
	interface.h network.h client.h server.h types.h lua.h physics.h chat.h
all: $(BINNAME)

$(BINNAME): $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: all clean

clean:
	rm -f $(OBJS) $(BINNAME)

