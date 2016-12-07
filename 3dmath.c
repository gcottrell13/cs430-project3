
#include <math.h>

#define pi 3.141592653589793238462643383279502

static inline float deg2rad(float deg)
{
	return deg * pi / 180;
}

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
void vector_copy(float* a, float* b)
{
	b[0] = a[0];
	b[1] = a[1];
	b[2] = a[2];
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

float length(float* a)
{
	return sqrt(sqr(a[0]) + sqr(a[1]) + sqr(a[2]));
}

void normalize(float* a)
{
	float len = length(a);
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

void interpolate(float* a, float* b, float i, float* c)
{
	c[0] = b[0] * i + a[0] * (1 - i);
	c[1] = b[1] * i + a[1] * (1 - i);
	c[2] = b[2] * i + a[2] * (1 - i);
}

// snell's law
//	sin(Θ1) = n1
//  ------    --
//  sin(Θ2)   n2
// 
// returns -1 if there is total internal reflection
float snells_law(float t1, float n1, float n2)
{
	float r = n2 * sin(t1) / n1;
	if(r <= 1 && r >= -1)
		return asin(r);
	return -1; // asin(n2 / n1);
}

// do-it-all function for calculating
void smellit(float* a, float* n, float n1, float n2, float* b)
{
	scale(a, -1, a);
	normalize(a);
	float a1 = acos(dot(a, n));
	float a2 = snells_law(a1, n1, n2);
	interpolate(n, a, a2 / a1, b);
	normalize(b);
	scale(b, -1, b);
	scale(a, -1, a);
}
