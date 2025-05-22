@echo off
setlocal enabledelayedexpansion

:: Get script directory
set "SCRIPT_PATH=%~dp0"
set "SCRIPT_DIR=%SCRIPT_PATH:~0,-1%"
set "DEBUG_DIR=%SCRIPT_DIR%\bin\debug"

:: Create output directory
if not exist "%DEBUG_DIR%" (
    mkdir "%DEBUG_DIR%"
)

:: Compiler settings
set "CXX=clang++"
set "CXXFLAGS=-std=c++17 -g -O0 -Wall -Wextra -Werror -Wno-error=deprecated-declarations -Wno-error=unused-function -Wvla -Wgnu-folding-constant -Wno-missing-braces -fdeclspec -Wstrict-prototypes -Wno-unused-parameter -Wno-missing-field-initializers -DWARPUNK_EXPORT=1 -DVK_DISABLE_VIDEO"
set "INCLUDES=-I%SCRIPT_DIR% -I%SCRIPT_DIR%\warpunk.core -I%SCRIPT_DIR%\warpunk.runtime"

:: Vulkan SDK Setup
set "VULKAN_SDK=C:\VulkanSDK\1.4.313.0"
set "INCLUDES=%INCLUDES% -I%VULKAN_SDK%\Include"
set "LIBS=-L%VULKAN_SDK%\Lib -lvulkan-1"

echo ---------------------------------------------------
echo Building warpunk.core (DLL)
echo ---------------------------------------------------
set "CORE_OBJS="
for /R "%SCRIPT_DIR%\warpunk.core" %%F in (*.cpp) do (
    set "OBJ=%DEBUG_DIR%\warpunk.core\%%~nF.o"
    if not exist "%DEBUG_DIR%\warpunk.core" mkdir "%DEBUG_DIR%\warpunk.core"
    echo Compiling %%F...
    %CXX% %CXXFLAGS% %INCLUDES% -c "%%F" -o "!OBJ!"
    if errorlevel 1 exit /b 1
    set "CORE_OBJS=!CORE_OBJS! !OBJ!"
)
%CXX% -g -shared -o %DEBUG_DIR%\warpunk.core.dll !CORE_OBJS! %LIBS% -luser32 -lgdi32 -lwinmm
if errorlevel 1 exit /b 1

echo.
echo ---------------------------------------------------
echo Building warpunk.runtime (DLL)
echo ---------------------------------------------------
set "RUNTIME_OBJS="
for /R "%SCRIPT_DIR%\warpunk.runtime" %%F in (*.cpp) do (
    set "OBJ=%DEBUG_DIR%\warpunk.runtime\%%~nF.o"
    if not exist "%DEBUG_DIR%\warpunk.runtime" mkdir "%DEBUG_DIR%\warpunk.runtime"
    echo Compiling %%F...
    %CXX% %CXXFLAGS% %INCLUDES% -c "%%F" -o "!OBJ!"
    if errorlevel 1 exit /b 1
    set "RUNTIME_OBJS=!RUNTIME_OBJS! !OBJ!"
)
%CXX% -g -shared -o %DEBUG_DIR%\warpunk.runtime.dll !RUNTIME_OBJS! ^
    -Wl,/implib:%DEBUG_DIR%\warpunk.runtime.lib ^
    -L%DEBUG_DIR% -lwarpunk.core %LIBS% -luser32 -lgdi32
if errorlevel 1 exit /b 1

echo.
echo ---------------------------------------------------
echo Building magicians_misfits (EXE)
echo ---------------------------------------------------
set "GAME_SRC=%SCRIPT_DIR%\magicians_misfits\magicians_misfits.cpp"
set "GAME_OBJ=%DEBUG_DIR%\magicians_misfits.o"
%CXX% %CXXFLAGS% -DWARPUNK_IMPORT=1 %INCLUDES% -c "%GAME_SRC%" -o "%GAME_OBJ%"
if errorlevel 1 exit /b 1

%CXX% -g -o %DEBUG_DIR%\magicians_misfits.exe "%GAME_OBJ%" ^
    -L%DEBUG_DIR% -lwarpunk.core -lwarpunk.runtime %LIBS% -luser32 -lgdi32
if errorlevel 1 exit /b 1

echo.
echo ---------------------------------------------------
echo Build erfolgreich!
echo Engine DLLs in: %DEBUG_DIR%
echo Game EXE in:    %DEBUG_DIR%\magicians_misfits.exe
echo ---------------------------------------------------
