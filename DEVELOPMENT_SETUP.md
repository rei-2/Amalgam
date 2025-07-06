# Development Setup Guide

## Quick Setup (Recommended)

1. **Run the automated setup:**
   ```cmd
   setup.bat
   ```

2. **Open the project:**
   - Open `Amalgam.sln` in Visual Studio 2022
   - Select configuration: `Release` and platform: `x64`
   - Build the solution

## Manual Setup

If you prefer to set up manually or the automated script fails:

### Prerequisites

- **Windows 10/11**
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Install with "Desktop development with C++" workload
- **Git** (for cloning dependencies)

### Steps

1. **Clone and bootstrap vcpkg:**
   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   bootstrap-vcpkg.bat
   ```

2. **Install dependencies:**
   ```cmd
   vcpkg install cpr:x64-windows-static-md
   vcpkg install nlohmann-json:x64-windows-static-md
   vcpkg integrate install
   ```

3. **Restore NuGet packages:**
   ```cmd
   nuget restore Amalgam.sln
   ```

## Build Configurations

- **Release** - Standard release build
- **ReleaseAVX2** - Optimized for AVX2 instruction set
- **ReleaseFreetype** - With FreeType font rendering
- **ReleaseFreetypeAVX2** - FreeType + AVX2 optimizations

## Dependencies

- **cpr** - C++ Requests library for HTTP functionality
- **nlohmann-json** - JSON parsing library
- **boost** - Boost libraries (via NuGet)
- **libolm** - Matrix encryption library (embedded in source)

## Output

Built DLLs will be located in:
```
output/x64/[Configuration]/Amalgam[Platform][Configuration].dll
```

## Troubleshooting

### vcpkg Integration Issues
If vcpkg integration fails, try running as administrator:
```cmd
vcpkg integrate install
```

### Build Errors
1. Ensure all dependencies are installed
2. Clean and rebuild the solution
3. Check that vcpkg integration is working:
   ```cmd
   vcpkg integrate project
   ```

### Missing NuGet
The setup script will automatically download NuGet if not found in PATH.