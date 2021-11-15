#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"

class camera
{
private:
	point3 origin;
	point3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	double lens_radius;
	double time0; // shutter open/close times
	double time1;

public:
	camera(point3 lookfrom, point3 lookat, vec3 vup, double vfov, double aspect_ratio, double aperture, double focus_dist, double _time0 = 0, double _time1 = 0) // vertical field-of-view in degrees
	{
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2); // equals to 1 when theta = pi / 2
		auto viewport_height = 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;
		auto focal_length = 1.0; // distance between eye and scene

		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		origin = lookfrom; // eye
		horizontal = focus_dist * viewport_width * u;
		vertical = focus_dist * viewport_height * v;
		lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

		lens_radius = aperture / 2;

		time0 = _time0;
		time1 = _time1;
	}

	ray get_ray(double s, double t) const
	{
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();
		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset, random_double(time0, time1));
	}
};
#endif

color pixel_color(0, 0, 0);
