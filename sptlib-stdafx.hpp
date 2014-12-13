#pragma once

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Psapi.h>
#include <detours.h>
#pragma comment( lib, "psapi.lib" )
#pragma comment (lib, "detours.lib")
#endif

#include <clocale>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <future>
#include <limits>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

using std::uintptr_t;
using std::size_t;
