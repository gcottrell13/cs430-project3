#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "3dmath.c"
#include "imageread.c"
#include "jsonread.c"
#include "raycast.c"

// snell's law

// I = (1 - K) I + K I

typedef struct {
	int retain;
} CounterObject;

CounterObject* create_object()
{
	CounterObject* value = malloc(sizeof(CounterObject));
	value->retain = 1;
}

void object_retain(CounterObject* value) {
	value-> retain ++;
}
void object_release(CounterObject* value) {
	value-> retain --;
	if(value -> retain <= 0)
		free(value);
}

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



