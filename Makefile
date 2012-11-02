
# TODO: provide an optional MinGW set of... things
# (hint: -static-libgcc will be helpful here!)
#
# OK, this is my commandline...
# make CC=i686-pc-mingw32-gcc BINNAME=pixmess.exe CFLAGS="-Iluainc -Isdlinc -D_GNU_SOURCE=1 -Dmain=SDL_main -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter" LIBS="-L. -lmingw32 -lSDLmain -lSDL -lz -llua -lwsock32"
# Yes, I have copied the lua/SDL/zlib stuff into dirs.
# --GM

# THIS IS THE PART YOU WANT TO EDIT.
OBJS_EXTRA = misc_sdl.o audio_sdl.o render_sdl.o event_sdl.o
#DEFS_EXTRA = -DDEBUG

# ADD -llua if needed
LIBS = `sdl-config --libs` -lz
CFLAGS = -g `sdl-config --cflags` -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable $(CFLAGS_EXTRA) $(DEFS_EXTRA)
LDFLAGS = -g $(LDFLAGS_EXTRA)
BINNAME = pixmess

OBJS = main.o event.o fixme.o lua.o map.o player.o render.o tile.o interface.o \
	network.o client.o server.o map_rw.o physics.o chat.o audio.o \
	$(OBJS_EXTRA)

INCLUDES = common.h render.h misc.h render_data.h player.h tile.h event.h map.h \
	interface.h network.h client.h server.h types.h lua.h physics.h chat.h \
	audio.h config.h
all: $(BINNAME)

$(BINNAME): $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: all clean

clean:
	rm -f $(OBJS) $(BINNAME)

