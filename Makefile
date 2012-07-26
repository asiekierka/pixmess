
# TODO: provide an optional MinGW set of... things
# (hint: -static-libgcc will be helpful here!)

LIBS = `sdl-config --libs`
CFLAGS = -g `sdl-config --cflags`
LDFLAGS = -g
BINNAME = c64pixels

OBJS = main.o render.o render_sdl.o
INCLUDES = common.h render.h
all: $(BINNAME)

$(BINNAME): $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: all clean

clean:
	rm -f $(OBJS) $(BINNAME)

