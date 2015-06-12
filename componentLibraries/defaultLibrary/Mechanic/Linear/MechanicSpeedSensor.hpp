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

//!
//! @file   MechanicSpeedSensor.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a Mechanic Speed Sensor Component
//!
//$Id$

#ifndef MECHANICSPEEDSENSOR_HPP_INCLUDED
#define MECHANICSPEEDSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicSpeedSensor : public ComponentSignal
    {
    private:
        double *mpND_v, *mpND_out;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicSpeedSensor();
        }

        void configure()
        {

            mpP1 = addReadPort("P1", "NodeMechanic", "", Port::NotRequired);
            addOutputVariable("out","Velocity","m/s", &mpND_out);
        }


        void initialize()
        {
            mpND_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            simulateOneTimestep(); //Set initial output node value
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_v);
        }
    };
}

#endif // MECHANICSPEEDSENSOR_HPP_INCLUDED
