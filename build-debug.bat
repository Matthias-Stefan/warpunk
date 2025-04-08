@echo off
setlocal enabledelayedexpansion

:: Get script directory
set "SCRIPT_PATH=%~dp0"
set "SCRIPT_DIR=%SCRIPT_PATH:~0,-1%"

:: Output directories
set "DEBUG_DIR=%SCRIPT_DIR%\bin\debug"

:: Ensure output directories exist
if not exist "%DEBUG_DIR%" (
    mkdir "%DEBUG_DIR%"
)

:: Compiler and flags
set "CXX=clang++"
set "CXXFLAGS=-std=c++17 -Werror -Wunused-function -g -O0 -DWARPUNK_EXPORT=1 -fvisibility=hidden -DVK_DISABLE_VIDEO"
set "INCLUDES=-I%SCRIPT_DIR% -I%SCRIPT_DIR%\warpunk.core -IC:\VulkanSDK\1.4.309.0\Include"

:: Compile main.cpp
echo Compiling main.cpp...
%CXX% %CXXFLAGS% %INCLUDES% -c main.cpp -o bin\debug\main.o
if errorlevel 1 (
    echo Compilation failed for main.cpp
    exit /b 1
)

:: Compile all .cpp files in warpunk.core
set "WARPUNK_O_FILES="
for /R "%SCRIPT_DIR%\warpunk.core" %%F in (*.cpp) do (
    set "SRC_FILE=%%F"
    set "RELATIVE_PATH=%%~pnxF"
    set "RELATIVE_PATH=!RELATIVE_PATH:%SCRIPT_DIR%\=!"
    set "OBJECT_FILE=%DEBUG_DIR%\!RELATIVE_PATH:.cpp=.o!"

    echo Compiling !RELATIVE_PATH!...
    if not exist "!OBJECT_FILE!\.." (
        mkdir "!OBJECT_FILE!\.."
    )
    %CXX% %CXXFLAGS% %INCLUDES% -c "%%F" -o "!OBJECT_FILE!"
    if errorlevel 1 (
        echo Compilation failed for !RELATIVE_PATH!
        exit /b 1
    )
    set "WARPUNK_O_FILES=!WARPUNK_O_FILES! !OBJECT_FILE!"
)

:: Compile shared library
echo Compiling magicians_misfits into a shared library...
%CXX% -std=c++17 -g -O0 -I./ -shared magicians_misfits\magicians_misfits.cpp -o bin\debug\magicians_misfits.dll
if errorlevel 1 (
    echo Compilation failed for magicians_misfits
    exit /b 1
)

:: Link the final executable
echo Linking object files into the final executable...
%CXX% -std=c++17 -g -o bin\debug\warpunk.exe bin\debug\main.o %WARPUNK_O_FILES% -lvulkan-1 -ldl -luser32 -lgdi32

:: Check build result
if %errorlevel%==0 (
    echo Build successful!
    echo Executable is located at: bin\debug\warpunk.exe
    echo Shared library is located at: bin\debug\magicians_misfits.dll
) else (
    echo Build failed.
    exit /b 1
)
