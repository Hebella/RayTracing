#pragma once
#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray
{
public:
	point3 _origin;
	vec3 _direction;
	double _time;

	ray() {}
	ray(const point3 & origin, const vec3 & direction, double time = 0.0): _origin(origin), _direction(direction), _time(time) {}

	point3 origin() const { return _origin; }
	vec3 direction() const { return _direction; }
	double time() const { return _time; }

	point3 at(double t) const
	{
		return _origin + t * _direction;
	}

};

#endif

