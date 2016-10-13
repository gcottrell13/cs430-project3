#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_OBJECTS 128
#define MAX_LIGHTS 10

typedef struct {
	int kind;
	float color[3]; // also diffuse color for non-lights
	float position[3];
	float direction[3]; // for lights only
	float a;
	float b;
	float c;
	float d;
	float specular[3];
	float transparency;
} Object;

typedef struct {
	int num_objects;
	Object objects[MAX_OBJECTS + 1];
	int num_lights;
	Object lights[MAX_LIGHTS + 1];
	float camera_width;
	float camera_height;
	float background_color[3]; // for fun!
} Scene;

typedef struct {
	float point[3];
	Object* object;
	int object_id;
} Intersection;

#include "3dmath.c"
#include "imageread.c"
#include "jsonread.c"
#include "raycast.c"

// diffuse reflection
// used for a rough surface, light bounces off in random directions

// specular reflection
// smooth surface
// 

// I = diffuse + specular + ambient

int main(int argc, char** argv)
{
	if(argc < 5)
	{
		fprintf(stderr, "Usage: width height input.json output.ppm\n");
		exit(1);
	}
	
	PPMmeta fileinfo;
	fileinfo.width = atoi(argv[1]);
	fileinfo.height = atoi(argv[2]);
	fileinfo.max = 255;
	fileinfo.type = 6;
	
	Scene scene = read_scene(argv[3]);

	printf("Read in %d objects\n", scene.num_objects);

	scene.background_color[0] = 0.7;
	scene.background_color[1] = 0.51;
	scene.background_color[2] = 0.6;
	
	raycast(scene, argv[4], fileinfo);
	
	return 0;
}



