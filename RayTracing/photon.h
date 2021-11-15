#pragma once

#include "vec3.h"

using namespace std;

struct Photon
{
    vec3 origin;
    vec3 dir;
    vec3 power;
    int divide_axis = 100;
};