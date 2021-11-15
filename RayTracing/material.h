#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "ray.h"
#include "vec3.h"
#include "hittable.h"
#include "texture.h"
#include "onb.h"

class material
{
public:
    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf, bool& is_reflected) const {
        return false;
    }
    virtual color emitted(const hit_record& rec, double u, double v, const point3& p) const
    {
        return color(0, 0, 0);
    }
    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
        return 0;
    }
    virtual bool use_monte_carlo() const
    {
        return false;
    }
    virtual vec3 albedo_color(const hit_record& rec) const
    {
        return color(1, 1, 1);
    }
};

class lambertian : public material
{
public:
    shared_ptr<texture> albedo;

    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) :albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf, bool& is_reflected) const override
    {
        onb uvw;
        uvw.build_from_w(rec.normal);
        auto direction = uvw.local(random_cosine_direction());
        //auto scatter_direction = rec.normal + random_unit_vector();

        //if (scatter_direction.near_zero())
        //    scatter_direction = rec.normal;

        //scattered = ray(rec.pt, scatter_direction, r_in.time());
        //attenuation = albedo->value(rec.u, rec.v, rec.pt);

        scattered = ray(rec.pt, unit_vector(direction), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.pt);
        pdf = dot(uvw.w(), scattered.direction()) / pi;
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
    {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

    virtual bool use_monte_carlo() const
    {
        return true;
    }

    virtual vec3 albedo_color(const hit_record& rec) const
    {
        //cerr << albedo->value(rec.u, rec.v, rec.pt) << endl;
        return albedo->value(rec.u, rec.v, rec.pt);
    }
};

class metal : public material
{
public:
    color albedo;
    double fuzz;

    metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf, bool& is_reflected) const override
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.pt, reflected + fuzz * random_in_unit_sphere(), r_in.time());
        attenuation = albedo;
        is_reflected = true;
        return (dot(scattered.direction(), rec.normal) > 0);
    }
};

class dielectric : public material
{
private:
    static double reflectance(double cosine, double ref_idx)
    {
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
public:
    double ir; // Index of refraction

    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf, bool& is_reflected) const override
    {
        attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
        {
            direction = reflect(unit_direction, rec.normal);
            is_reflected = true;
        }
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        scattered = ray(rec.pt, direction, r_in.time());
        return true;
    }
};

class diffuse_light : public material
{
public:
    shared_ptr<texture> emit;

    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf, bool& is_reflected) const override
    {
        return false;
    }

    virtual color emitted(const hit_record& rec, double u, double v, const point3& p) const override
    {
        if (rec.front_face)
            return emit->value(u, v, p);
        else
            return color(0, 0, 0);
    }
};

class isotropic : public material
{
public:
    shared_ptr<texture> albedo;

    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf, bool& is_reflected) const override
    {
        scattered = ray(rec.pt, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.pt);
        return true;
    }
};
#endif // MATERIAL_H

