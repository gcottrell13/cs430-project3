
#include <math.h>


static inline float min(float a, float b)
{
	return a > b ? b : a;
}
static inline float max(float a, float b)
{
	return a < b ? b : a;
}
static inline float clamp(float a, float lo, float hi)
{
	return max(lo, min(a, hi));
}

static inline float sqr(float v)
{
	return v*v;
}

void printv(char* str, float* v)
{
	printf("%s <%f %f %f>\n", str, v[0], v[1], v[2]);
}

void add(float* a, float* b, float* out)
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}
void subtract(float* a, float* b, float* out)
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}
void multiply(float* a, float* b, float* out)
{
	out[0] = a[0] * b[0];
	out[1] = a[1] * b[1];
	out[2] = a[2] * b[2];
}
void scale(float* a, float b, float* out)
{
	out[0] = a[0] * b;
	out[1] = a[1] * b;
	out[2] = a[2] * b;
}
float dot(float* a, float* b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
void cross(float* a, float*b, float* out)
{
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - b[2]*a[0];
	out[2] = a[0]*b[1] - a[1]*b[0];
}

void normalize(float* a)
{
	float len = sqrt(sqr(a[0]) + sqr(a[1]) + sqr(a[2]));
	a[0] /= len;
	a[1] /= len;
	a[2] /= len;
}


float* quadratic_formula(float a, float b, float c)
{
	float* results = malloc(sizeof(float) * 2);
	
	float det = sqr(b) - 4 * a * c;
	if(det < 0) {
		results[0] = NAN;
		return results;
	}
	
	det = sqrt(det);
	
	results[0] = (-b - det) / (2 * a);
	results[1] = (-b + det) / (2 * a);
	
	return results;
}


