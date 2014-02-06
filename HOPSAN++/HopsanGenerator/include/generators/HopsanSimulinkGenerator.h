#ifndef HOPSANSIMULINKGENERATOR_H
#define HOPSANSIMULINKGENERATOR_H

#include "HopsanGenerator.h"

class HopsanSimulinkGenerator : public HopsanGenerator
{
public:
    HopsanSimulinkGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void generateToSimulink(QString savePath, QString modelFile, hopsan::ComponentSystem *pSystem, bool disablePortLabels);
    void generateToSimulinkCoSim(QString savePath, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler);
};

#endif // HOPSANSIMULINKGENERATOR_H
