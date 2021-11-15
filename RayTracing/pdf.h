//
//  pdf.h
//  RayTracing
//
//  Created by 刘雅新 on 2021/11/9.
//

#ifndef pdf_h
#define pdf_h

#include "vec3.h"
#include "onb.h"
#include "hittable.h"

using namespace std;

class pdf
{
public:
    virtual ~pdf() {}
    virtual double value(const vec3& direction) const = 0;
    virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf
{
public:
    onb uvw;
    cosine_pdf(const vec3& w)
    {
        uvw.build_from_w(w); // w: normal vector
    }
    
    virtual double value(const vec3& direction) const override
    {
        auto cosine = dot(unit_vector(direction), uvw.w());
        return (cosine <= 0) ? 0 : cosine / pi;
    }
    
    virtual vec3 generate() const override
    {
        return uvw.local(random_cosine_direction());
    }
};

class hittable_pdf : public pdf
{
public:
    point3 o;
    shared_ptr<hittable> ptr;
    
    hittable_pdf(shared_ptr<hittable> p, const point3& origin) : ptr(p), o(origin) {}
    
    virtual double value(const vec3& direction) const override
    {
        return ptr->pdf_value(o, direction);
    }
    
    virtual vec3 generate() const override
    {
        return ptr->random(o);
    }
};

class mixture_pdf : public pdf
{
public:
    shared_ptr<pdf> p[2];

    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1)
    {
        p[0] = p0;
        p[1] = p1;
    }

    virtual double value(const vec3& direction) const override
    {
        return 0.7 * p[0]->value(direction) + 0.3 * p[1]->value(direction);
    }

    virtual vec3 generate() const override
    {
        if (random_double() < 0.7)
            return p[0]->generate();
        else
            return p[1]->generate();
    }
};
#endif /* pdf_h */
