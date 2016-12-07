

#define SPEC_FALL 20
#define SPEC_K 0.4
#define DIFFUSE_K 1.0

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

float intersect_cylinder(Object cyl, float* r0, float* rd)
{
	float basis2[3];
	basis2[0] = cyl.a;
	basis2[1] = cyl.b;
	basis2[2] = cyl.c;

	float basis1[3];
	vector_copy(cyl.direction, basis1);

	float c[3];
	vector_copy(cyl.position, c);

	float r0_dot_b1 = dot(r0, basis1);
	float r0_dot_b2 = dot(r0, basis2);
	float rd_dot_b1 = dot(rd, basis1);
	float rd_dot_b2 = dot(rd, basis2);
	float c_dot_b1 = dot(c, basis1);
	float c_dot_b2 = dot(c, basis2);

	float A = sqr(rd_dot_b1) + sqr(rd_dot_b2);
	float B = 2 * (rd_dot_b1*r0_dot_b1 + r0_dot_b2*r0_dot_b2 - rd_dot_b1*c_dot_b1 - rd_dot_b2*c_dot_b2);
	float C = sqr(r0_dot_b2 - c_dot_b2) + sqr(r0_dot_b1 - c_dot_b1) - sqr(cyl.e);

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
	int k;
	i->object_id = -1;
	
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
			t = intersect_plane(o.direction[0], o.direction[1], o.direction[2], o.d, r0, rd);
		}
		else if(o.kind == T_CYLINDER)
		{
			t = intersect_cylinder(o, r0, rd);
		}
		else if(o.kind == 0)
			break;
		
		if(t > 0 && t < best_t)
		{
			best_t = t;
			i->object_id = k;
		}
	}

	if(i->object_id < -1) printf("Strange object id: %d. Avoid: %d\n", i->object_id, avoid);
	
	scale(rd, best_t, i->point); // scale rd by best_t
	add(i->point, r0, i->point); // then add that to r0
	
	i->object = &(scene.objects[i->object_id]);
}


void get_color_ray(float* color, Scene scene, float* r0, float* rd, int recursion)
{
	if(recursion <= 0){
		vector_copy(scene.ambient_color, color);
		return;
	} 

	Intersection intersection;
	send_ray(&intersection, scene, r0, rd, -1);

	if(intersection.object_id == -1)
	{
		vector_copy(scene.ambient_color, color);
		return;
	}

	// do reflections here

	// keep a reference to the intersected object
	Object* closest = intersection.object;

	// do lighting on the object
	float lighting[3];
	vector_copy(scene.ambient_color, lighting);
	int number_contributors = 1;

	float normal[3];
	// a test to see how we need to calculate the normal
	if(closest->kind == T_SPHERE)
	{
		subtract(intersection.point, closest->position, normal);
		normalize(normal);
	}
	else if(closest->kind == T_PLANE)
	{
		vector_copy(closest->direction, normal);
	}

	float added_color[3]; // from any transparency that might happen
	// transparency stuff goes here
	if(closest->e > 0)
	{
		// do some refraction
		
		float refracted_ray[3];
		float new_point[3];
		float n1 = 1;
		float n2 = 1;
		
		if(dot(normal, rd) < 0) n2 = closest->b; else n1 = closest->b;
		smellit(rd, normal, n1, n2, refracted_ray);
		
		// continue on through the object
		add(intersection.point, refracted_ray, new_point);
		get_color_ray(added_color, scene, new_point, refracted_ray, recursion - 1);
		scale(added_color, closest->e, added_color);
		added_color[0] = clamp(added_color[0], 0.0, 1.0);
		added_color[1] = clamp(added_color[1], 0.0, 1.0);
		added_color[2] = clamp(added_color[2], 0.0, 1.0);
		number_contributors ++;
	}

	// loop through the lights
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

		// (Xs - Xl) / ||Xs-Xl||
		
		float incident_light_level = dot(normal, dir_to_light);

		if(incident_light_level > 0)
		{
			float Ic[3];
			Ic[0] = 0;
			Ic[1] = 0;
			Ic[2] = 0;

			// calculate attenuation
				float ang_att = 1;
				if(light.e != 0) // is spotlight
				{
					float att_dot = dot(light_dir, light.direction);
					if(att_dot < light.e)
						ang_att = 0;
					else
						ang_att = powf(att_dot, light.d);
				}

				float rad_att = 1 / 
							(light.a * sqr(distance_to_light) + 
								light.b * distance_to_light + light.c);

				float attenuation = clamp(ang_att * rad_att, 0.0, 1.0);

				float spec[3];
				
				// reflect the light normal across the surface normal
				// r = d - 2(d*n)n
				float r[3];
				scale(normal, dot(light_dir, normal) * 2, r);
				subtract(light_dir, r, r);

				float v[3];
				scale(rd, -1, v);

				float speck = powf(dot(r, v), closest->a) * SPEC_K;
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

	float reflect_color[3];
	scale(reflect_color,0,reflect_color);
	// do reflection here
	if(closest->c > 0)
	{
		float reflect_r[3];
		//scale(rd, -1, reflect_r);
		vector_copy(rd, reflect_r);
		scale(normal, dot(reflect_r, normal) * 2, reflect_r);
		subtract(rd, reflect_r, reflect_r);
		
		float reflect_new_point[3];
		add(intersection.point, reflect_r, reflect_new_point);
		get_color_ray(reflect_color, scene, reflect_new_point, reflect_r, recursion - 1);
		
		scale(reflect_color, closest->c, reflect_color);
		reflect_color[0] = clamp(reflect_color[0], 0.0, 1.0);
		reflect_color[1] = clamp(reflect_color[1], 0.0, 1.0);
		reflect_color[2] = clamp(reflect_color[2], 0.0, 1.0);
		//number_contributors ++;
	}

	//scale(lighting, 1 - (closest->e), lighting);

	vector_copy(lighting, color);
	add(added_color, color, color);
	add(reflect_color, color, color);

	scale(color, 1/number_contributors, color);
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

			get_color_ray(colors, scene, r0, rd, 7);

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