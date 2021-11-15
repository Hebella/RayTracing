#pragma once
#ifndef  MOVING_SPHERE_H
#define MOVING_SPHERE_H

#include "hittable.h"
#include "vec3.h"

class moving_sphere : public hittable
{
public:
	point3 _center0, _center1;
	double _radius;
	double _time0, _time1;
	shared_ptr<material> mat_ptr;

	moving_sphere() {}
	moving_sphere(point3 center0, point3 center1, double time0, double time1, double radius, shared_ptr<material> m) : _center0(center0), _center1(center1), _time0(time0), _time1(time1), _radius(radius), mat_ptr(m) {};
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;
	point3 center(double time) const;
};

point3 moving_sphere::center(double time) const
{
	return _center0 + ((time - _time0) / (_time1 - _time0)) * (_center1 - _center0);
}

/// <summary>
/// calculate the hit point
/// </summary>
/// <param name="r"></param>
/// <param name="t_min"></param>
/// <param name="t_max"></param>
/// <param name="rec"></param>
/// <returns></returns>
bool moving_sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	vec3 oc = r.origin() - center(r.time());
	auto a = dot(r.direction(), r.direction());
	auto half_b = dot(oc, r.direction());
	auto c = dot(oc, oc) - _radius * _radius;
	auto delta = half_b * half_b - a * c;
	if (delta < 0)
		return false;
	auto sqrt_delta = sqrt(delta);

	auto root = (-half_b - sqrt_delta) / a;
	if (root < t_min || t_max < root)
	{
		root = (-half_b + sqrt_delta) / a;
		if (root < t_min || t_max < root)
			return false;
	}

	rec.t = root;
	rec.pt = r.at(rec.t);
	vec3 outward_normal = (rec.pt - center(r.time())) / _radius;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat_ptr;

	return true;
}

bool moving_sphere::bounding_box(double time0, double time1, aabb& output_box) const
{
	aabb box0(center(time0) - vec3(_radius, _radius, _radius), center(time0) + vec3(_radius, _radius, _radius));
	aabb box1(center(time1) - vec3(_radius, _radius, _radius), center(time1) + vec3(_radius, _radius, _radius));
	output_box = surrounding_box(box0, box1);
	return true;
}
#endif // ! MOVING_SPHERE_H