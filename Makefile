all: rasterizer

%.o: %.c
	$(CC) -c -o $@ $<

rasterizer: aiv_math.o aiv_rasterizer.o aiv_obj_parser.o main.o
	$(CC) -o $@ $^ -lSDL2
