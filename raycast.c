

#define SPEC_FALL 15
#define SPEC_K 0.4
#define DIFFUSE_K 1.0

#define AMBIENT_R 0.15
#define AMBIENT_G 0.15
#define AMBIENT_B 0.15

float intersect_sphere(float* c, float R, float* r0, float* rd)
{
	// A = xd^2 + yd^2 + zd^2
	// B = 2 * (xd * (x0 - xc) + yd * (y0 - yc) + zd * (z0 - zc))
	// C = (x0 - xc)^2 + (y0 - yc)^2 + (z0 - zc)^2 - r^2
	
	float A = sqr(rd[0]) + sqr(rd[1]) + sqr(rd[2]);
	float B = 2 * (rd[0] * (r0[0] - c[0]) + rd[1] * (r0[1] - c[1]) + rd[2] * (r0[2] - c[2]));
	float C = sqr(r0[0] - c[0]) + sqr(r0[1] - c[1]) + sqr(r0[2] - c[2]) - sqr(R);
	
	float* zeroes = quadratic_formula(A, B, C);
	
	if(isnan(zeroes[0]))
		return -1;
	
	if(zeroes[0] > 0) return zeroes[0];
	if(zeroes[1] > 0) return zeroes[1];
	return -1;
}

float intersect_plane(float a, float b, float c, float d, float* r0, float* rd)
{
	// t = -(a*x0 + b*y0 + c*z0) / (a*xd + b*yd + c*zd)
	return (a*r0[0] + b*r0[1] + c*r0[2] + d) / (a*rd[0] + b*rd[1] + c*rd[2]);
}

// returns the scene's object id that intersects the ray
// the basic object finding loop now lives here
void send_ray(Intersection* i, Scene scene, float* r0, float* rd, int avoid)
{
	float best_t = INFINITY;
	int closest_id = -1;
	int k;
	
	for(k = 0; k < scene.num_objects; k ++)
	{
		if(k == avoid) {
			continue;
		}

		float t = -1;
		Object o = scene.objects[k];
		
		if(o.kind == T_SPHERE)
		{
			t = intersect_sphere(o.position, o.d, r0, rd);
		}
		else if(o.kind == T_PLANE)
		{
			t = intersect_plane(o.a, o.b, o.c, o.d, r0, rd);
		}
		else if(o.kind == 0)
			break;
		
		if(t > 0 && t < best_t)
		{
			best_t = t;
			closest_id = k;
		}
	}
	
	scale(rd, best_t, i->point); // scale rd by best_t
	add(i->point, r0, i->point); // then add that to r0
	
	i->object = &(scene.objects[closest_id]);
	i->object_id = closest_id;
}


void get_color_ray(float* color, Scene scene, float* r0, float* rd, float opacity_left, int avoid)
{
	Intersection intersection;
	send_ray(&intersection, scene, r0, rd, avoid);

	if(intersection.object_id == -1)
	{
		color[0] = scene.background_color[0];
		color[1] = scene.background_color[1];
		color[2] = scene.background_color[2];
		return;
	}

	Object* closest = intersection.object;
	// do lighting on the object
	float lighting[3];
	lighting[0] = AMBIENT_R;
	lighting[1] = AMBIENT_G;
	lighting[2] = AMBIENT_B;

	int k;
	for(k = 0; k < scene.num_lights; k ++)
	{
		Object light = scene.lights[k];

		float light_dir[3];
		float dir_to_light[3];
		
		// distance to light
		float dist[3];
		subtract(light.position, intersection.point, dist);
		float distance_to_light = length(dist);
		
		subtract(intersection.point, light.position, light_dir);
		normalize(light_dir);
		scale(light_dir, -1, dir_to_light);

		// test for a shadow intersection
		Intersection shadow;
		send_ray(&shadow, scene, intersection.point, dir_to_light, intersection.object_id);

		// if there is an object between this object and the light, don't light it
		if(shadow.object_id != -1) {
			// make sure that this object isn't actually behind the light
			
			subtract(shadow.point, intersection.point, dist);
			float distance_to_object = length(dist);

			if(distance_to_light > distance_to_object)
				continue;
		}

		float normal[3];

		if(closest->kind == T_SPHERE)
		{
			subtract(intersection.point, closest->position, normal);
			normalize(normal);
		}
		else if(closest->kind == T_PLANE)
		{
			normal[0] = closest->a;
			normal[1] = closest->b;
			normal[2] = closest->c;
		}

		// (Xs - Xl) / ||Xs-Xl||

		float incident_light_level = -dot(normal, light_dir);
		if(incident_light_level > 0)
		{
			float Ic[3];
			Ic[0] = 0;
			Ic[1] = 0;
			Ic[2] = 0;

			// calculate attenuation
				float attenuation = 1.0;
				

			// do specular highlight
				float spec[3];

				// reflect the light normal across the surface normal
				// r = d - 2(d*n)n
				float r[3];
				scale(normal, dot(light_dir, normal) * 2, r);
				subtract(light_dir, r, r);

				float v[3];
				scale(rd, -1, v);

				float speck = powf(dot(r, v), SPEC_FALL) * SPEC_K;
				scale(light.color, speck, spec);
				multiply(closest->specular, spec, spec);
				
			// do diffuse lighting
				float diffuse[3];
				scale(closest->color, incident_light_level * DIFFUSE_K, diffuse);
			

			if(speck > 0)
			{
				add(spec, diffuse, Ic);
				scale(Ic, attenuation, Ic);
			}
			else
				scale(diffuse, attenuation, Ic);

			add(Ic, lighting, lighting);
		}
	}
	scale(lighting, 1, color);
	//scale(lighting, 1 - closest->transparency, color);
}

void raycast(Scene scene, char* outfile, PPMmeta fileinfo)
{
	Pixel* data = malloc(sizeof(Pixel) * fileinfo.width * fileinfo.height);
	
	// raycasting here
	
	int N = fileinfo.width;
	int M = fileinfo.height;
	float w = scene.camera_width;
	float h = scene.camera_height;
	
	float pixel_height = h / M;
	float pixel_width = w / N;
	
	float p_z = 0;
	
	float c_x = 0;
	float c_y = 0;
	float c_z = 0;
	
	float r0[3];
	r0[0] = c_x;
	r0[1] = c_y;
	r0[2] = c_z;
	
	float rd[3];
	rd[2] = 1;
	
	int i;
	int j;
	int k;
	
	for(i = 0; i < M; i ++)
	{
		rd[1] = -r0[1] + h/2.0 - pixel_height * (i + 0.5);
		
		for(j = 0; j < N; j ++)
		{
			rd[0] = r0[0] - w/2.0 + pixel_width * (j + 0.5);
			
			float colors[3];

			get_color_ray(colors, scene, r0, rd, 1.0, -1);

			colors[0] = clamp(colors[0], 0.0, 1.0);
			colors[1] = clamp(colors[1], 0.0, 1.0);
			colors[2] = clamp(colors[2], 0.0, 1.0);

			Pixel pixel = data[i * N + j];
			pixel.r = (unsigned char) (colors[0] * 255);
			pixel.g = (unsigned char) (colors[1] * 255);
			pixel.b = (unsigned char) (colors[2] * 255);
			data[i * N + j] = pixel;
			
		}
		
	}
	
	WritePPM(data, outfile, fileinfo);
}