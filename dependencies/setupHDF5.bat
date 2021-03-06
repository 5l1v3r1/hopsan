@ECHO OFF
REM Bat script building libHDF5 dependency automatically

setlocal
set basedir=%~dp0
set name=hdf5
set version=1.8.21
set codedir=%basedir%\%name%-code\%name%-%version%
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

call setHopsanBuildPaths.bat

mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
cmake --build . --parallel 8
cmake --build . --target install

cd %basedir%
echo.
echo setupHDF5.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
