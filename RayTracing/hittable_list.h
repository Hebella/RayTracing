#pragma once
#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

#include <memory>
#include <vector>

using namespace std;

class hittable_list : public hittable
{
public:
	vector<shared_ptr<hittable>> objects;

	hittable_list() {}
	hittable_list(shared_ptr<hittable> object)
	{
		add(object);
	}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

	void clear()
	{
		objects.clear();
	}

	void add(shared_ptr<hittable> object)
	{
		objects.push_back(object);
	}
};

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	hit_record temp_rec;
	bool hit_anything = false;
	auto closest_so_far = t_max;

	for (const auto & object : objects)
	{
		if (object->hit(r, t_min, closest_so_far, temp_rec)) // find the closest hit point 
		{
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}
	return hit_anything;
}

bool hittable_list::bounding_box(double time0, double time1, aabb& output_box) const
{
	if (objects.empty())
		return false;

	aabb temp_box;
	bool is_first_box = true;

	for (const auto& object : objects)
	{
		if (!object->bounding_box(time0, time1, temp_box))
			return false;
		output_box = is_first_box ? temp_box : surrounding_box(output_box, temp_box);
		is_first_box = false;
	}

	return true;
}
#endif