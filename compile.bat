@echo off
setlocal EnableDelayedExpansion

echo ===========================================
echo FastAudioPlayer Native Build
echo ===========================================
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Try to find Java automatically
if "%JAVA_HOME%"=="" (
    if exist "C:\Program Files\Java\jdk-25" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-25"
    ) else if exist "C:\Program Files\Java\jdk-21" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-21"
    ) else if exist "C:\Program Files\Java\jdk-17" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-17"
    ) else if exist "C:\Program Files\Eclipse Adoptium\jdk-21*" (
        for /d %%i in ("C:\Program Files\Eclipse Adoptium\jdk-21*") do set "JAVA_HOME=%%i"
    )
)

if "%JAVA_HOME%"=="" (
    echo ERROR: JAVA_HOME not set and could not find Java automatically
    echo Please set JAVA_HOME manually
    exit /b 1
)

echo Using JAVA_HOME: %JAVA_HOME%

REM Setup Visual Studio environment
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo Setting up Visual Studio environment...
    
    REM Try to find VS installation
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -property installationPath`) do (
            set "VSPATH=%%i"
        )
    )
    
    if "!VSPATH!"=="" (
        REM Fallback to common paths
        if exist "C:\Program Files\Microsoft Visual Studio\2022\Community" (
            set "VSPATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
        ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional" (
            set "VSPATH=C:\Program Files\Microsoft Visual Studio\2022\Professional"
        ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise" (
            set "VSPATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
        ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" (
            set "VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
        )
    )
    
    if not "!VSPATH!"=="" (
        call "!VSPATH!\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        echo ERROR: Could not find Visual Studio 2022
        echo Please run this from a Developer Command Prompt
        exit /b 1
    )
)

REM Get Windows SDK version
for /f "tokens=*" %%i in ('dir /b /ad "C:\Program Files (x86)\Windows Kits\10\Include\" 2^>nul ^| sort /r ^| findstr /r "^[0-9]"') do (
    set "WindowsSDKVersion=%%i"
    goto :found_sdk
)
:found_sdk

set "WindowsSdkDir=C:\Program Files (x86)\Windows Kits\10\"

REM Get MSVC version
for /f "tokens=*" %%i in ('dir /b /ad "%VSPATH%\VC\Tools\MSVC\" 2^>nul ^| sort /r') do (
    set "VCToolsVersion=%%i"
    goto :found_vc
)
:found_vc

echo Building with MSVC %VCToolsVersion%...

REM Compile
cl.exe ^
    /nologo ^
    /W3 ^
    /O2 ^
    /MD ^
    /EHsc ^
    /std:c++17 ^
    /wd4005 ^
    /I"%JAVA_HOME%\include" ^
    /I"%JAVA_HOME%\include\win32" ^
    /I"%WindowsSdkDir%Include\%WindowsSDKVersion%\um" ^
    /I"%WindowsSdkDir%Include\%WindowsSDKVersion%\shared" ^
    /I"%WindowsSdkDir%Include\%WindowsSDKVersion%\ucrt" ^
    /I"%VSPATH%\VC\Tools\MSVC\%VCToolsVersion%\include" ^
    ..\native\src\fastaudioplayer.cpp ^
    /link ^
    /DLL ^
    /OUT:fastaudioplayer.dll ^
    /DEF:..\native\fastaudioplayer.def ^
    kernel32.lib ^
    user32.lib ^
    winmm.lib ^
    ole32.lib

if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo ===========================================
echo === Build successful ===
echo Output: build\fastaudioplayer.dll
echo ===========================================

cd ..
