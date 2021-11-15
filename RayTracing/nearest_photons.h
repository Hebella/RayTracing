#pragma once
#include "photon.h"
#include "vec3.h"

#include <queue>
#include <vector>

struct photon_dist
{
	Photon p;
	float dist_square;
	photon_dist(Photon _p = Photon(), float _dist_square = 0) : p(_p), dist_square(_dist_square) {}
};

bool operator<(photon_dist p1, photon_dist p2)
{
	return p1.dist_square < p2.dist_square;
}

class nearest_photons_map
{
public:
	vec3 origin;
	int max_num;
	float max_dist_square;
	priority_queue<photon_dist, vector<photon_dist>, less<photon_dist>> nearest_photons;
	nearest_photons_map(vec3 _origin, float _max_dist_square, int _max_num = 0) : origin(_origin), max_dist_square(_max_dist_square), max_num(_max_num) {}

	void get_nearest_photons(const vector<Photon>& photons, int index);
};

void nearest_photons_map::get_nearest_photons(const vector<Photon>& photons, int index)
{
	Photon cur_photon = photons[index];
	if (2 * index + 1 < photons.size())
	{
		double dist_axis = origin[cur_photon.divide_axis] - cur_photon.origin[cur_photon.divide_axis];
		if (dist_axis < 0 || dist_axis * dist_axis < max_dist_square)
			get_nearest_photons(photons, 2 * index + 1);
		if ((2 * index + 2 < photons.size()) && (dist_axis >= 0 || dist_axis * dist_axis < max_dist_square))
			get_nearest_photons(photons, 2 * index + 2);
	}

	float cur_dist_square = (cur_photon.origin - origin).length_squared();
	if (cur_dist_square > max_dist_square)
		return;
	nearest_photons.push(photon_dist(cur_photon, cur_dist_square));

	if (nearest_photons.size() > max_num)
		nearest_photons.pop();
}