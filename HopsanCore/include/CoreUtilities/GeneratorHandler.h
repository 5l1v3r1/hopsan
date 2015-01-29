/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

#ifndef GENERATORHANDLER_H
#define GENERATORHANDLER_H

#ifdef _WIN32
#define _WIN32_WINNT 0x0502
#include "Windows.h"
#else
#include "dlfcn.h"
#endif
#include <vector>
#include "win32dll.h"
#include "HopsanTypes.h"

namespace hopsan {

class ComponentSystem;

class DLLIMPORTEXPORT GeneratorHandler
{
public:
    GeneratorHandler();
    ~GeneratorHandler();

    bool isLoadedSuccessfully();

    typedef void (*call_modelica_generator_t)(HString path, hopsan::HString gccPath, bool showDialog, int solver, bool compile, hopsan::HString coreIncludePath, hopsan::HString binPath);
    typedef void (*call_cpp_generator_t)(HString hppPath, hopsan::HString gccPath, bool compile, hopsan::HString coreIncludePath, hopsan::HString binPath);
    typedef void (*call_fmu_import_generator_t)(HString path, HString targetPath, HString coreIncludePath, HString binPath, HString gccPath, bool showDialog);
    typedef void (*call_fmu_export_generator_t)(HString path, hopsan::ComponentSystem *pSystem, HString coreIncludePath, HString binPath, HString gccPath, bool me, bool x64, bool showDialog);
    typedef void (*call_simulink_export_generator_t)(HString path, HString modelName, hopsan::ComponentSystem *pSystem, bool disablePortLabels, HString coreIncludePath, HString binPath, bool showDialog);
    typedef void (*call_simulink_cosim_export_generator_t)(HString path, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler, HString coreIncludePath, HString binPath, bool showDialog);
    typedef void (*call_lvsit_export_generator_t)(HString path, hopsan::ComponentSystem *pSystem, HString coreIncludePath, HString binPath, bool showDialog);
    typedef void (*call_complib_compiler_t)(HString path, HString extraLinks, HString coreIncludePath, HString binPath, HString gccPath, bool showDialog);
    typedef void (*call_library_generator_t)(HString path, std::vector<HString> hppFiles, bool showDialog);

    call_modelica_generator_t callModelicaGenerator;
    call_cpp_generator_t callCppGenerator;
    call_fmu_import_generator_t callFmuImportGenerator;
    call_fmu_export_generator_t callFmuExportGenerator;
    call_simulink_export_generator_t callSimulinkExportGenerator;
    call_simulink_cosim_export_generator_t callSimulinkCoSimExportGenerator;
    call_lvsit_export_generator_t callLabViewSITGenerator;
    call_complib_compiler_t callComponentLibraryCompiler;
    call_library_generator_t callLibraryGenerator;

private:
    bool mLoadedSuccessfully;

#ifdef _WIN32
    HINSTANCE lib_ptr;
#else
    void* lib_ptr;
#endif
};

}

#endif // GENERATORHANDLER_H
