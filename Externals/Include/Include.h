#pragma once

#ifdef _MSC_VER
    #include "GLEW/glew.h"
    #include "FreeGLUT/freeglut.h"
    #include <direct.h>
#else
    #include <OpenGL/gl3.h>
    #include <GLUT/glut.h>
    #include <unistd.h>
#endif

#include "TinyOBJ/tiny_obj_loader.h"
#include "STB/stb_image.h"
// #include "utils/glutils.hpp"
// #include "utils/timer.hpp"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#ifdef _MSC_VER
    #pragma comment (lib, "glew32.lib")
	#pragma comment(lib, "freeglut.lib")
    #pragma comment(lib, "assimp.lib")
#endif

#define GLM_SWIZZLE
#include "GLM/glm/glm.hpp"
#include "GLM/glm/gtc/matrix_transform.hpp"
#include "GLM/glm/gtc/type_ptr.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>

#ifdef _MSC_VER
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
    #define __FILEPATH__(x) ((std::string(__FILE__).substr(0, std::string(__FILE__).rfind('\\'))+(x)).c_str())
#else
	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
    #define __FILEPATH__(x) ((std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/'))+(x)).c_str())
#endif

#ifndef _DEG2RAD
#define _DEG2RAD
    #define deg2rad(x) ((x)*((M_PI)/(180.0f)))
#endif
