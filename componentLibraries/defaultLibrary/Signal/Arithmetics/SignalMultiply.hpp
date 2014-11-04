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
//! @file   SignalMultiply.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical multiplication function
//!
//$Id$

#ifndef SIGNALMULTIPLY_HPP_INCLUDED
#define SIGNALMULTIPLY_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMultiply : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1Port, *mpIn2Port;

    public:
        static Component *Creator()
        {
            return new SignalMultiply();
        }

        void configure()
        {
            mpIn1Port = addInputVariable("in1", "", "", 1.0, &mpND_in1);
            mpIn2Port = addInputVariable("in2", "", "", 1.0, &mpND_in2);
            addOutputVariable("out", "in1*in2", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Multiplication equation
            (*mpND_out) = (*mpND_in1) * (*mpND_in2);
        }
    };
}

#endif // SIGNALMULTIPLY_HPP_INCLUDED
