# Cross-compile for Windows (64-bit) using Mingw-w64

# Build using:
#   cmake . -Bbuild -DCMAKE_TOOLCHAIN_FILE=Windows64.cmake
#   cmake --build build

# from @ticho:cyberdi.sk
# https://paste.debian.net/1201338/

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# static-link against the standard libraries
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++")
