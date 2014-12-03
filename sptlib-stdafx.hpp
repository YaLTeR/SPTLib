#pragma once

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <clocale>

#include <future>
#include <limits>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using std::uintptr_t;
using std::size_t;
