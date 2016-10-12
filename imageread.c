
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Pixel;

typedef struct {
	int width;
	int height;
	int max;
	int type;
} PPMmeta;

// functions that will verify PPM, then check which type (ascii P3 or rawbits P6)
// then load file into memory with an unsigned char pointer

// @return int - error code
int WritePPM(Pixel* data, char* output, PPMmeta meta);

char* intToStr(int i, int size);

int WritePPM(Pixel* data, char* output, PPMmeta meta)
{
	FILE *out = fopen(output, "w+");
	int out_type = meta.type;
	
	// values are newline delimited
	
	if(out_type == 3)
	{
		
		fprintf(out, "P%d\n", out_type);
		fprintf(out, "%d %d\n", meta.width, meta.height);
		fprintf(out, "%d\n", meta.max);
	
		int c;
		char newline = 10;
		for(c = 0; c < meta.width * meta.height; c++)
		{
			Pixel p = data[c];
			//printf("%d %d %d\n", p.r, p.g, p.b);
			char* r = intToStr(p.r, 4);
			char* g = intToStr(p.g, 4);
			char* b = intToStr(p.b, 4);
			fprintf(out, "%s\n", r);
			fprintf(out, "%s\n", g);
			fprintf(out, "%s\n", b);
		}
	}
	else if(out_type == 6)
	{
		
		fprintf(out, "P%d\n", out_type);
		fprintf(out, "%d %d\n", meta.width, meta.height);
		fprintf(out, "%d\n", meta.max);
		
		int c;
		for(c = 0; c < meta.width * meta.height; c++)
		{
			Pixel p = data[c];
			//fwrite(p.r, sizeof(char), sizeof(p.r), out);
			//fwrite(p.g, sizeof(char), sizeof(p.g), out);
			//fwrite(p.b, sizeof(char), sizeof(p.b), out);
			fprintf(out, "%c", p.r);
			fprintf(out, "%c", p.g);
			fprintf(out, "%c", p.b);
		}
	}
	else
	{
		fclose(out);
		// write to stderr "Unsupported PPM type: %i!", type
		fprintf(stderr, "Unsupported PPM type: %d!\n", out_type);
		return 1;
	}
	fclose(out);
	return 0;
}

char* intToStr(int i, int size)
{
	char* str = malloc(sizeof(char) * size);
	sprintf(str, "%d", i);
	return str;
}