CC=gcc

# Executables
EXECUTABLES = findpng catpng

# Build rule
all: $(EXECUTABLES)

# Rule to build findpng
findpng: source/findpng.c source/pnginfo.c source/crc.c
	$(CC) -o $@ $^

# Rule to build catpng
catpng: source/catpng.c source/pnginfo.c source/crc.c source/zutil.c
	$(CC) -o $@ $^ -lz

# Clean rule
clean:
	rm -f $(EXECUTABLES) *.o *.out