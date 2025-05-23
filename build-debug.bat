@echo off
setlocal enabledelayedexpansion

:: ===================================================
:: Project Paths
:: ===================================================
set "SCRIPT_PATH=%~dp0"
set "SCRIPT_DIR=%SCRIPT_PATH:~0,-1%"
set "DEBUG_DIR=%SCRIPT_DIR%\bin\debug"

:: Create output directory if missing
if not exist "%DEBUG_DIR%" mkdir "%DEBUG_DIR%"

:: ===================================================
:: Compiler and Flags
:: ===================================================
set "CXX=clang++"
::Werror
set "CXXFLAGS=-std=c++17 -g -gcodeview -O0 -Wall -Wextra ^
-Wno-error=deprecated-declarations -Wno-error=unused-function ^
-Wvla -Wgnu-folding-constant -Wno-missing-braces -fdeclspec ^
-Wstrict-prototypes -Wno-unused-parameter -Wno-missing-field-initializers ^
-DVK_DISABLE_VIDEO -D_CRT_SECURE_NO_WARNINGS -DWARPUNK_DEBUG"

set "INCLUDES=-I%SCRIPT_DIR% -I%SCRIPT_DIR%\warpunk.core -I%SCRIPT_DIR%\warpunk.runtime"

:: Vulkan SDK configuration
set "VULKAN_SDK=C:\VulkanSDK\1.4.313.0"
set "INCLUDES=%INCLUDES% -I%VULKAN_SDK%\Include"
set "LIBS=-L%VULKAN_SDK%\Lib -lvulkan-1 -luser32 -lgdi32 -lwinmm"

:: ===================================================
:: Build warpunk.core DLL
:: ===================================================
echo.
echo ---------------------------------------------------
echo Building warpunk.core (DLL)
echo ---------------------------------------------------

set "CORE_OBJS="
if not exist "%DEBUG_DIR%\warpunk.core" mkdir "%DEBUG_DIR%\warpunk.core"

for /R "%SCRIPT_DIR%\warpunk.core" %%F in (*.cpp) do (
    set "OBJ=%DEBUG_DIR%\warpunk.core\%%~nF.o"
    echo Compiling %%F...
    %CXX% %CXXFLAGS% -DWARPUNK_EXPORT=1 %INCLUDES% -c "%%F" -o "!OBJ!"
    if errorlevel 1 exit /b 1
    set "CORE_OBJS=!CORE_OBJS! !OBJ!"
)

%CXX% -g -gcodeview -shared -o %DEBUG_DIR%\warpunk.core.dll !CORE_OBJS! ^
    -Wl,/implib:%DEBUG_DIR%\warpunk.core.lib ^
    -Wl,/pdb:%DEBUG_DIR%\warpunk.core.pdb ^
    %LIBS%
if errorlevel 1 exit /b 1

:: ===================================================
:: Build warpunk.runtime DLL
:: ===================================================
echo.
echo ---------------------------------------------------
echo Building warpunk.runtime (DLL)
echo ---------------------------------------------------

set "RUNTIME_OBJS="
if not exist "%DEBUG_DIR%\warpunk.runtime" mkdir "%DEBUG_DIR%\warpunk.runtime"

for /R "%SCRIPT_DIR%\warpunk.runtime" %%F in (*.cpp) do (
    set "OBJ=%DEBUG_DIR%\warpunk.runtime\%%~nF.o"
    echo Compiling %%F...
    %CXX% %CXXFLAGS% -DWARPUNK_EXPORT=1 %INCLUDES% -c "%%F" -o "!OBJ!"
    if errorlevel 1 exit /b 1
    set "RUNTIME_OBJS=!RUNTIME_OBJS! !OBJ!"
)

%CXX% -g -gcodeview -shared -o %DEBUG_DIR%\warpunk.runtime.dll !RUNTIME_OBJS! ^
    -Wl,/implib:%DEBUG_DIR%\warpunk.runtime.lib ^
    -Wl,/pdb:%DEBUG_DIR%\warpunk.runtime.pdb ^
    -L%DEBUG_DIR% %LIBS% -lwarpunk.core
if errorlevel 1 exit /b 1

:: ===================================================
:: Build magicians_misfits executable
:: ===================================================
echo.
echo ---------------------------------------------------
echo Building magicians_misfits (EXE)
echo ---------------------------------------------------

set "GAME_SRC=%SCRIPT_DIR%\magicians_misfits\magicians_misfits.cpp"
set "GAME_OBJ=%DEBUG_DIR%\magicians_misfits.o"

%CXX% %CXXFLAGS% -DWARPUNK_IMPORT=1 %INCLUDES% -c "%GAME_SRC%" -o "%GAME_OBJ%"
if errorlevel 1 exit /b 1

%CXX% -g -gcodeview -o %DEBUG_DIR%\magicians_misfits.exe "%GAME_OBJ%" ^
    -Wl,/pdb:%DEBUG_DIR%\magicians_misfits.pdb ^
    -L%DEBUG_DIR% -lwarpunk.core -lwarpunk.runtime %LIBS%
if errorlevel 1 exit /b 1

:: ===================================================
:: Build Summary
:: ===================================================
echo.
echo ---------------------------------------------------
echo Build successful.
echo Engine DLLs and PDBs: %DEBUG_DIR%
echo Game executable:      %DEBUG_DIR%\magicians_misfits.exe
echo ---------------------------------------------------
