#pragma once
#include <stdio.h>
#include "lab_png.h"

void write_chunk(FILE *fp, chunk_p new_chunk);
void write_png_file(const char *filename, simple_PNG_p new_png);
int catpng(char **buf, int count);