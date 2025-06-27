# Setting Up OBS Development Environment

## Quick Solution (5 minutes):

### Step 1: Download OBS Development Package
1. Go to: https://github.com/obsproject/obs-studio/releases/latest
2. Download: `obs-studio-30.x.x-Full-x64.zip` (get the latest version)
3. Extract the zip file to a temporary folder

### Step 2: Set Up Development Headers
```bash
# Create OBS development directory
mkdir C:\obs-dev
mkdir C:\obs-dev\include
mkdir C:\obs-dev\lib

# Copy from the extracted OBS package:
# - Copy the 'include' folder to C:\obs-dev\include
# - Copy contents of 'bin\64bit' to C:\obs-dev\lib
```

### Step 3: Update Our Project
Update the paths in `obs-multistream.vcxproj`:

**Change this line:**
```xml
<AdditionalIncludeDirectories>C:\Program Files\obs-studio\include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
```

**To this:**
```xml
<AdditionalIncludeDirectories>C:\obs-dev\include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
```

**And change this line:**
```xml
<AdditionalLibraryDirectories>C:\Program Files\obs-studio\bin\64bit;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

**To this:**
```xml
<AdditionalLibraryDirectories>C:\obs-dev\lib;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

### Step 4: Test Build
```bash
./build.bat
```

## Alternative: Use GitHub Actions (No Local Setup Needed)

If you want to avoid local setup completely:

1. **Push code to GitHub repository**
2. **Use GitHub Actions** for building (OBS provides cloud build environment)
3. **Download compiled DLL** from GitHub releases

This is actually how most OBS plugin developers work!

## Production Workflow Recommendation

For serious plugin development, most developers use:
1. **Local development** with proper OBS dev environment
2. **GitHub Actions** for CI/CD and releases
3. **OBS Plugin Template** as starting point

The template at https://github.com/obsproject/obs-plugintemplate provides everything pre-configured. 