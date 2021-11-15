//
//  raytracing_stb_image.h
//  RayTracing
//
//  Created by 刘雅新 on 2021/10/11.
//

#ifndef raytracing_stb_image_h
#define raytracing_stb_image_h

// Disable pedantic warnings for this external library.
#ifdef _MSC_VER
    // Microsoft Visual C++ Compiler
    #pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Restore warning levels.
#ifdef _MSC_VER
    // Microsoft Visual C++ Compiler
    #pragma warning (pop)
#endif


#endif /* raytracing_stb_image_h */
