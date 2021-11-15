//
//  photon.h
//  RayTracing
//
//  Created by 刘雅新 on 2021/11/10.
//

#ifndef photon_h
#define photon_h

#include "vec3.h"
#include "photon.h"
#include "nearest_photons.h"

#include <vector>
#include <cmath>

using namespace std;

inline int calculate_mid(int start, int end)
{
    int num = end - start + 1;
    int smaller_num = pow(2, int(log2(num))) - 1;
    return start + min((num - smaller_num), (smaller_num + 1) / 2) + smaller_num / 2;
}


class PhotonMap
{
public:
    vec3 box_min, box_max; // bounding box
    int maxPhotonNum;
    vector<Photon> photons;
    
    PhotonMap(int _maxPhotonNum = 10000);
    ~PhotonMap()
    {
        
    }
    void store(Photon p);
    float get_photon_origin_axis(int index, int axis);
    void split(vector<Photon>& photons_temp, int start, int end, int mid, int axis);
    void balance();
    void balance(vector<Photon>& photons_temp, int index, int start, int end);
    vec3 getIrradiance(vec3 origin, vec3 norm, float max_dist, int max_num);
};

PhotonMap::PhotonMap(int _maxPhotonNum) : maxPhotonNum(_maxPhotonNum)
{
    box_min = vec3(1000000.0, 1000000.0, 1000000.0);
    box_max = vec3(-1000000.0, -1000000.0, -1000000.0);
}

void PhotonMap::store(Photon p)
{
    if (photons.size() >= maxPhotonNum)
        return;
    photons.push_back(p);
    
    box_min = vec3(min(box_min.x(), p.origin.x()), min(box_min.y(), p.origin.y()), min(box_min.z(), p.origin.z()));
    box_max = vec3(max(box_max.x(), p.origin.x()), max(box_max.y(), p.origin.y()), max(box_max.z(), p.origin.z()));
}

float PhotonMap::get_photon_origin_axis(int index, int axis)
{
    return photons[index].origin[axis];
}

void PhotonMap::split(vector<Photon>& photons_temp, int start, int end, int mid, int axis)
{
    int l = start, r = end;
    while (l < r)
    {
        double pivot = photons_temp[r].origin[axis];
        int i = l - 1, j = r;
        while (true)
        {
            while (photons_temp[++i].origin[axis] < pivot);
            while (photons_temp[--j].origin[axis] > pivot && j > l);
            
            if (i >= j)
                break;
            swap(photons_temp[i], photons_temp[j]);
        }
        swap(photons_temp[i], photons_temp[r]);
        if (i >= mid)
            r = i - 1;
        if (i <= mid)
            l = i + 1;
    }
}

void PhotonMap::balance()
{
    vector<Photon> phptons_temp = photons;
    balance(phptons_temp, 0, 0, photons.size() - 1);
}

void PhotonMap::balance(vector<Photon>& photons_temp, int index, int start, int end)
{
    if (index >= photons_temp.size())
        return;
    if (start == end)
    {
        photons[index] = photons_temp[start];
        return;
    }
    int mid = calculate_mid(start, end);

    int axis = 2;
    float x_boundary = box_max.x() - box_min.x(), y_boundary = box_max.y() - box_min.y(), z_boundary = box_max.z() - box_min.z();
    if (x_boundary > max(y_boundary, z_boundary))
        axis = 0;
    if (y_boundary > max(x_boundary, z_boundary))
        axis = 1;
    split(photons_temp, start, end, mid, axis);
    photons[index] = photons_temp[mid];
    photons[index].divide_axis = axis;

    if (start < mid)
    {
        float tmp = box_max[axis];
        box_max[axis] = photons[index].origin[axis];
        balance(photons_temp, index * 2 + 1, start, mid - 1); // left child
        box_max[axis] = tmp;
    }

    if (mid < end)
    {
        float tmp = box_min[axis];
        box_min[axis] = photons[index].origin[axis];
        balance(photons_temp, index * 2 + 2, mid + 1, end); // right child
        box_max[axis] = tmp;
    }
}

vec3 PhotonMap::getIrradiance(vec3 origin, vec3 norm, float max_dist, int max_num)
{
    vec3 res;
    nearest_photons_map local_map(origin, max_dist * max_dist, max_num);
    local_map.get_nearest_photons(photons, 0);
    if (local_map.nearest_photons.size() <= 15)
        return res;


    while (!local_map.nearest_photons.empty())
    {
        vec3 dir = local_map.nearest_photons.top().p.dir;
        if (dot(norm, dir) < 0)
            res += local_map.nearest_photons.top().p.power;
        local_map.nearest_photons.pop();
    }

    res *= (1.0 / (pi * max_dist * max_dist)) * 30;
    //cerr << res << endl;
    return res;
}

#endif /* photon_h */
