#pragma once

#define FUNCSIG __FUNCSIG__
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define Assert(cond) if (!cond) { MessageBox(0, #cond "\n\n" FUNCSIG " Line " LINE_STRING, "Error", MB_OK | MB_ICONERROR); }
#define AssertFatal(cond) if (!cond) { MessageBox(0, #cond "\n\n" FUNCSIG " Line " LINE_STRING, "Error", MB_OK | MB_ICONERROR); exit(EXIT_FAILURE); }
#define AssertCustom(cond, message) if (!cond) { MessageBox(0, message, "Error", MB_OK | MB_ICONERROR); }