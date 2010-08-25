#include "myLib.h"
#include "../../HopsanCore/Component.h"
#include "../../HopsanCore/win32dll.h"
using namespace hopsan;

extern "C" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr)
{
    std::cout << "Running register function in myLib dll" << std::endl;
    //Register Components
    cfact_ptr->RegisterCreatorFunction("MyWickedOrifice", MyWickedOrifice::Creator);

    //Register custom nodes (if any)
}
