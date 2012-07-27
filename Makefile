
# TODO: provide an optional MinGW set of... things
# (hint: -static-libgcc will be helpful here!)

LIBS = `sdl-config --libs`
CFLAGS = -g `sdl-config --cflags`
LDFLAGS = -g
BINNAME = c64pixels
PLATFORM = sdl

OBJS = main.o event.o event_$(PLATFORM).o fixme.o map.o misc_$(PLATFORM).o player.o render.o \
	render_$(PLATFORM).o tile.o
INCLUDES = common.h render.h misc.h render_data.h player.h tile.h event.h map.h
all: $(BINNAME)

$(BINNAME): $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: all clean

clean:
	rm -f $(OBJS) $(BINNAME)

