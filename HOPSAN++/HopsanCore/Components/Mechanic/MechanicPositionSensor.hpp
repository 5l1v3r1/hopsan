//!
//! @file   MechanicPositionSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Position Sensor Component
//!
//$Id$

#ifndef MECHANICPOSITIONSENSOR_HPP_INCLUDED
#define MECHANICPOSITIONSENSOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicPositionSensor : public ComponentSignal
{
private:
    enum {P1,out};

public:
    static Component *Creator()
    {
        std::cout << "running MechanicPositionSensor creator" << std::endl;
        return new MechanicPositionSensor("PositionSensor");
    }

    MechanicPositionSensor(const string name,
                           const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "MechanicPositionSensor";

        addReadPort("P1", "NodeMechanic", P1);
        addWritePort("out", "NodeSignal", out);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double x = mPortPtrs[P1]->readNode(NodeMechanic::POSITION);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, x);
    }
};

#endif // MECHANICPOSITIONSENSOR_HPP_INCLUDED
