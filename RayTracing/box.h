//
//  box.h
//  RayTracing
//
//  Created by 刘雅新 on 2021/10/12.
//

#ifndef box_h
#define box_h

#include "aarect.h"
#include "hittable_list.h"

class box : public hittable
{
public:
    point3 box_min;
    point3 box_max;
    hittable_list sides;
    
    box() {}
    box(const point3 & p0, const point3 & p1, shared_ptr<material> ptr);
    
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
    {
        return sides.hit(r, t_min, t_max, rec);
    }
    
    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
    {
        output_box = aabb(box_min, box_max);
        return true;
    }
    
};

box::box(const point3 & p0, const point3 & p1, shared_ptr<material> ptr)
{
    box_min = p0;
    box_max = p1;
    
    sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
    sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));
    
    sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
    sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));

    sides.add(make_shared<yz_rect>(p0.z(), p1.z(), p0.y(), p1.y(), p1.x(), ptr));
    sides.add(make_shared<yz_rect>(p0.z(), p1.z(), p0.y(), p1.y(), p0.x(), ptr));
}

#endif /* box_h */
