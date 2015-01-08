#include <iostream>
#include <sstream>
#include <assert.h>
#include "HopsanCore.h"
#include "Port.h"
#include "HopsanFMU.h"
#include "model.hpp"

static double fmu_time=0;
static hopsan::ComponentSystem *spCoreComponentSystem = 0;
static std::vector<std::string> sComponentNames;
std::map<std::string, double> parametersMap;
hopsan::HopsanEssentials gHopsanCore;

hopsan::Port *ports[<<<nports>>>];

void initializeHopsanWrapper(char* filename)
{
    double startT;      //Dummy variable
    double stopT;       //Dummy variable
    gHopsanCore.loadHMFModel("defaultComponentLibrary.dll");    //Only used for debugging, since components are not included in HopsanCore.dll during development
    spCoreComponentSystem = gHopsanCore.loadHMFModel(filename, startT, stopT);

    assert(spCoreComponentSystem);
    spCoreComponentSystem->setDesiredTimestep(0.001);
    spCoreComponentSystem->disableLog();
    spCoreComponentSystem->initialize(0,10);

    fmu_time = 0;
}

void initializeHopsanWrapperFromBuiltInModel()
{
    spCoreComponentSystem = gHopsanCore.loadHMFModel(getModelString().c_str());

    //Assert that model was successfully loaded
    assert(spCoreComponentSystem);

    //Set parameters
    std::map<std::string,double>::iterator it;
    for(it=parametersMap.begin(); it!=parametersMap.end(); ++it)
    {
        std::stringstream ss;
        ss << it->second;
        spCoreComponentSystem->setSystemParameter(it->first.c_str(), ss.str().c_str(), "double");
    }

    //Initialize systsem
    spCoreComponentSystem->setDesiredTimestep(0.001);
    spCoreComponentSystem->disableLog();
    spCoreComponentSystem->initialize(0,10);

    fmu_time = 0;

    >>>assignportpointers>>>ports[<<<idx>>>] = spCoreComponentSystem->getSubComponent("<<<comp>>>")->getPort("<<<port>>>");
    <<<assignportpointers<<<
}

void simulate(double stopTime)
{
    spCoreComponentSystem->simulate(stopTime);
    fmu_time = stopTime;
}

void simulateOneStep()
{
    double timestep = getTimeStep();
    spCoreComponentSystem->simulate(fmu_time+timestep);
    fmu_time = fmu_time+timestep;
}

double getTimeStep()
{
    return spCoreComponentSystem->getDesiredTimeStep();
}

double getFmuTime()
{
    return fmu_time;
}

double getVariable(size_t ref, size_t idx)
{
    if(spCoreComponentSystem)
        return ports[ref]->readNode(idx);
    else
        return 0;
}

void setVariable(size_t ref, size_t idx, double value)
{
    if(spCoreComponentSystem)
        return ports[ref]->writeNode(idx, value);
}

void setParameter(char* name, double value)
{
    if(spCoreComponentSystem)
    {
        parametersMap.erase(name);
        parametersMap.insert (std::pair<std::string,double>(name,value));
    }
}
