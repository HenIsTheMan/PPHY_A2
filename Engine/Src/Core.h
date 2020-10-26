#pragma once

#define NOMINMAX

#include <Windows.h>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <IRRKLANG/irrKlang.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <functional>
#include <fstream>
#include <map>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace irrklang;

constexpr auto KEY_2 = 50;
constexpr auto KEY_I = 73;
constexpr auto KEY_L = 76;
constexpr auto KEY_O = 79;
constexpr auto KEY_P = 80;

#define STR(text) #text

#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "irrKlang.lib")

typedef const char* cstr;
typedef unsigned int uint;
typedef std::string str;