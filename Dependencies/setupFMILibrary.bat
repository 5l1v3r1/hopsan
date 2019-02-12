@ECHO OFF
REM $Id$

REM Bat script building FMILibrary dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=fmilibrary
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

call setHopsanBuildPaths.bat

REM build
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DFMILIB_FMI_PLATFORM="win64" -DFMILIB_INSTALL_PREFIX=%installdir% %codedir%
mingw32-make.exe SHELL=cmd -j8
mingw32-make.exe SHELL=cmd install

cd %basedir%
echo.
echo setupFMILibrary.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
