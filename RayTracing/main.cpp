#include <iostream>
#include "utils.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "camera.h"
#include "material.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "bvh.h"
#include "pdf.h"
#include "photon_map.h"
#include "nearest_photons.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include <typeinfo>

using namespace std;

shared_ptr<PhotonMap> photon_map = make_shared<PhotonMap>(50000);

// calculate the intersection of the ray and the hittable objects
color ray_color(const ray& r, const color& background, const hittable& world, shared_ptr<hittable>& lights, int depth)
{
    hit_record rec;

    if (depth <= 0)
        return color(0, 0, 0);

    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec, rec.u, rec.v, rec.pt);
    double pdf;
    color albedo;
    bool is_reflected = false;


    if (!rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf, is_reflected)) // light emitting material: false
        return emitted;
    //auto on_light = point3(random_double(213, 343), 554, random_double(-332, -227));
    //auto to_light = on_light - rec.pt;
    //auto distance_squared = to_light.length_squared();
    //to_light = unit_vector(to_light);

    //if (dot(to_light, rec.normal) < 0)
        //return emitted;

    //double light_area = (double)(343 - 213) * (-227 + 332);
    //auto light_cosine = fabs(to_light.y());
    //if (light_cosine < 0.000001)
        //return emitted;

    //pdf = distance_squared / (light_cosine * light_area);
    //scattered = ray(rec.pt, to_light, r.time());
    //cosine_pdf p(rec.normal);
    //scattered = ray(rec.pt, p.generate(), r.time());
    //pdf = p.value(scattered.direction());
    if (!is_reflected)
        return photon_map->getIrradiance(rec.pt, rec.normal, 20, 100);

    if (!rec.mat_ptr->use_monte_carlo())
        return emitted + albedo * ray_color(scattered, background, world, lights, depth - 1);

    auto p0 = make_shared<hittable_pdf>(lights, rec.pt);
    auto p1 = make_shared<cosine_pdf>(rec.normal);
    mixture_pdf mixed_pdf(p0, p1);

    //hittable_pdf light_pdf(lights, rec.pt);
    scattered = ray(rec.pt, mixed_pdf.generate(), r.time());
    pdf = mixed_pdf.value(scattered.direction());


    return emitted + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered) * ray_color(scattered, background, world, lights, depth - 1) / pdf;
    //return emitted + albedo * ray_color(scattered, background, world, lights, depth - 1);
}

void trace_photon(const ray& r, const hittable& world, int depth, float power_scale, shared_ptr<PhotonMap> photon_map)
{
    hit_record rec;

    if (depth >= 10)
        return;

    
    if (!world.hit(r, 0.001, infinity, rec))
        return;

   

    bool is_reflected = false;
    color attenuation;
    double pdf;
    color albedo;
    ray scattered;
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered, pdf, is_reflected))
        return;


    if (is_reflected)
        trace_photon(scattered, world, depth + 1, power_scale, photon_map);
    else
    {
        if (depth == 0)
            return;

        Photon p;
        p.origin = rec.pt;
        p.dir = r.direction();
        p.power = power_scale * rec.mat_ptr->albedo_color(rec);
        photon_map->store(p);
    }
}

hittable_list random_scene()
{
    hittable_list world;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    // auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -5; a < 5; a += 2)
    {
        for (int b = -5; b < 5; b += 2)
        {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9)
            {
                shared_ptr<material> sphere_material;
                if (choose_mat < 0.8)
                {
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0, 0.5), 0); // �����ƶ�

                    world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.7, 0.3, 0.3));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

hittable_list two_spheres()
{
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list two_perlin_spheres()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth()
{
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

hittable_list simple_light()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list cornell_box()
{
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    objects.add(make_shared<yz_rect>(-555, 0, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(-555, 0, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, -332, -227, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, -555, 0, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, -555, 0, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, -555, white));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    //auto metalball = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(300, 100, -300), 50, material3));

    

    /*shared_ptr<hittable> box1 = make_shared<box>(point3(0.0, 0.0, 0), point3(165.0, 330.0, 165.0), ground);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(130, 0, -500));
    //objects.add(make_shared<constant_medium>(box1, 0.01, color(7, 7, 7)));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0.0, 0.0, 0), point3(165.0, 165, 165.0), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(300, 0, -300));
    objects.add(box2);*/
    return objects;
}

hittable_list final_scene()
{
    hittable_list objects;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    auto light = make_shared<diffuse_light>(color(20, 20, 20));
    objects.add(make_shared<xz_rect>(123, 423, -412, -147, 554, light));

    const int boxes_per_side = 10;
    for (int i = 0; i < boxes_per_side; ++i)
    {
        for (int j = 0; j < boxes_per_side; ++j)
        {
            auto w = 100.0;
            auto x0 = -200.0 + i * w;
            auto z0 = -800.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            //objects.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
        }
    }


    auto center1 = point3(400, 400, -200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    auto glassball = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(150, 150, -50), 50, glassball));

    auto metalball = make_shared<metal>(color(0.8, 0.8, 0.9), 0.0);
    objects.add(make_shared<sphere>(point3(600, 150, -150), 50, metalball));

    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(400, 200, -200), 100, earth_surface);
    objects.add(globe);

    auto pertext = make_shared<noise_texture>(0.2);
    objects.add(make_shared<sphere>(point3(250, 300, -400), 100, make_shared<lambertian>(pertext)));

    return objects;
}

void Initial()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  //清屏颜色
    glViewport(0, 0, 600, 600);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

vector<vec3> points, points_red;

static const float vertex_list[][3] =
{
    -1.0f, -1.0f, -3.0f,
    1.0f, -1.0f, -3.0f,
    -1.0f, 1.0f, -3.0f,
    1.0f, 1.0f, -3.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
};

static const GLint index_list[][2] =
{
    {0, 1},
    {2, 3},
    {4, 5},
    {6, 7},
    {0, 2},
    {1, 3},
    {4, 6},
    {5, 7},
    {0, 4},
    {1, 5},
    {7, 3},
    {2, 6}
};


void RenderScene(int w, int h, int z)
{
    glPointSize(3);

    glBegin(GL_POINTS);
    for (int i = 0; i < points.size(); ++i)
    {
        float x = points[i].x() / (float)w * 2 - 1;
        float y = points[i].y() / (float)h * 2 - 1;
        float z = points[i].z() / 600 - 1;
        //cout << x << " " << y << " " << z << endl;
        glVertex3f(x, y, z);
    }
    glEnd();


    glBegin(GL_LINES);
    for (int i = 0; i < 12; ++i) // 12 条线段
    {
        for (int j = 0; j < 2; ++j) // 每条线段 2个顶点
        {
            glVertex3fv(vertex_list[index_list[i][j]]);
        }
    }
    glEnd();

    /*glPointSize(8);
    glColor3f(1, 0, 0);
    glBegin(GL_POINTS);
    for (int i = 0; i < points_red.size(); ++i)
    {
        float x = points_red[i].x() / (float)w * 2 - 1;
        float y = points_red[i].y() / (float)h * 2 - 1;
        float z = points_red[i].z() / 600 - 1;
        //cout << x << " " << y << " " << z << endl;
        glVertex3f(x, y, z);
    }
    glEnd();

    glColor3f(0, 1, 0);
    glBegin(GL_POINTS);
    float x = points_red[0].x() / (float)w * 2 - 1;
    float y = points_red[0].y() / (float)h * 2 - 1;
    float k = points_red[0].z() / 600 - 1;
    glVertex3f(x, y, k);
    glEnd();*/
}


void display(void)
{
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0, 0, 0); //画笔蓝色   
    glLoadIdentity();  //加载单位矩阵   
    gluLookAt(2.0, 0.0, 2.0, 1, 0, 0, 0.0, 1.0, 0.0);

    RenderScene(600, 600, 600);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, (GLfloat)w / (GLfloat)h, 2.9, 10000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv)
{
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 800;
    int samples_per_pixel = 50;
    const int max_depth = 5;

    // World
    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0, 0, 0);

    switch (6) {
    case 1:
        world = random_scene();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        break;

    case 2:
        world = two_spheres();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;

    case 3:
        world = two_perlin_spheres();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;

    case 4:
        world = earth();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;

    case 5:
        world = simple_light();
        samples_per_pixel = 200;
        background = color(0, 0, 0);
        lookfrom = point3(26, 3, 6);
        lookat = point3(0, 2, 0);
        vfov = 20.0;
        break;

    case 6:
        world = cornell_box();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 20;
        background = color(0, 0, 0);
        lookfrom = point3(278, 278, 800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;

    case 8:
        world = final_scene();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 200;
        background = color(0, 0, 0);
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(78, 278, 800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;
    }
    shared_ptr<hittable> lights = make_shared<xz_rect>(213, 343, -332, -227, 554, shared_ptr<material>());

    // Camera

    int image_height = static_cast<int>(image_width / aspect_ratio);

    camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, aspect_ratio, aperture, 10.0, 0.0, 1.0);

    // Render

    cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    xz_rect photon_lights(213, 343, -332, -227, 554, shared_ptr<material>());
    vec3 origin, dir, power = vec3(27.0f, 27.0f, 27.0f);
    float power_scale;
    while (photon_map->photons.size() < 50000)
    {
        photon_lights.generate_photon(origin, dir, power_scale);
        ray r(origin, dir);
        trace_photon(r, world, 0, power_scale, photon_map);
    }

    photon_map->balance();

    //cerr << photon_map->photons.size() << endl;

    for (int j = image_height - 1; j >= 0; --j)
    {
        cerr << "\rScanlines remaining: " << j << ' ' << flush;
        for (int i = 0; i < image_width; ++i)
        {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s)
            {
                auto u = double(i + random_double()) / (image_width - 1);
                auto v = double(j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, lights, max_depth);
            }
            write_color(cout, pixel_color, samples_per_pixel);
        }
    }

    //shared_ptr<PhotonMap> photon_map = make_shared<PhotonMap>(10000);

    
    for (int i = 0; i < photon_map->photons.size(); ++i)
    {
        points.push_back(vec3(photon_map->photons[i].origin.x(), photon_map->photons[i].origin.y(), photon_map->photons[i].origin.z()));
    }
    cerr << "\nDone.\n";

    

    /*nearest_photons_map test_map(photon_map->photons[0].origin, 10000, 15);
    test_map.get_nearest_photons(photon_map->photons, 0);

    while (!test_map.nearest_photons.empty())
    {
        Photon p = test_map.nearest_photons.top().p;
        test_map.nearest_photons.pop();
        points_red.push_back(vec3(p.origin.x(), p.origin.y(), p.origin.z()));
        cerr << p.origin << endl;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Create Dot");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    

    for (int i = 0; i < photon_map->photons.size(); ++i)
    {
        cerr << vec3(photon_map->photons[i].origin.x(), photon_map->photons[i].origin.y(), photon_map->photons[i].origin.z()) << " " << photon_map->photons[i].divide_axis << endl;
    }*/


    
    return 0;
}
