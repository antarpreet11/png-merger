CC=gcc
CFLAGS=-Wall -g

# Executables
EXECUTABLES = findpng catpng

# Build rule
all: $(EXECUTABLES)

# Rule to build findpng
findpng: source/findpng.c source/pnginfo.c source/crc.c
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build catpng
catpng: source/catpng.c source/pnginfo.c source/crc.c
	$(CC) $(CFLAGS) -o $@ $^

# Clean rule
clean:
	rm -f $(EXECUTABLES)