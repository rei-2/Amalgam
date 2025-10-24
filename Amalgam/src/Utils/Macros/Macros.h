#pragma once

#define VA_LIST(...) __VA_ARGS__

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x
#define __LINESTRING__ STRINGIFY(__LINE__)

#define ADD_FEATURE_CUSTOM(type, name, scope) namespace scope { inline type name; }
#define ADD_FEATURE(type, name) ADD_FEATURE_CUSTOM(type, name, F)

#define Assert(cond) if (!cond) { MessageBox(0, #cond "\n\n" __FUNCSIG__ " Line " __LINESTRING__, "Error", MB_OK | MB_ICONERROR); }
#define AssertFatal(cond) if (!cond) { MessageBox(0, #cond "\n\n" __FUNCSIG__ " Line " __LINESTRING__, "Error", MB_OK | MB_ICONERROR); exit(EXIT_FAILURE); }
#define AssertCustom(cond, message) if (!cond) { MessageBox(0, message, "Error", MB_OK | MB_ICONERROR); }