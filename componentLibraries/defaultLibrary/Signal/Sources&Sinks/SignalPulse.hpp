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
//! @file   SignalPulse.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-14
//!
//! @brief Contains a pulse signal generator
//!
//$Id$

////////////////////////////////////////////////
//                    XXXXX       â           //
//                    X   X       | Amplitude //
//                    X   X       |           //
// BaseValue â  XXXXXXX   XXXXXXX â           //
//                    â   â                   //
//            StartTime   StopTime            //
////////////////////////////////////////////////

#ifndef SIGNALPULSE_HPP_INCLUDED
#define SIGNALPULSE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPulse : public ComponentSignal
    {

    private:
        double *mpOut, *mpY0, *mpTstart, *mpTend, *mpYa;

    public:
        static Component *Creator()
        {
            return new SignalPulse();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "", 0.0, &mpY0);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpYa);
            addInputVariable("t_start", "Start Time", "Time", 1.0, &mpTstart);
            addInputVariable("t_end", "Stop Time", "Time", 2.0, &mpTend);

            addOutputVariable("out", "Pulse", "", &mpOut);
        }


        void initialize()
        {
            // Write initial value
            (*mpOut) = (*mpY0);
        }


        void simulateOneTimestep()
        {
                //Step Equations
            const double time = mTime+0.5*mTimestep;
            if ( time >= (*mpTstart) && time < (*mpTend))
            {
                (*mpOut) = (*mpY0) + (*mpYa);     //During pulse
            }
            else
            {
                (*mpOut) = (*mpY0);               //Not during pulse
            }
        }
    };
}

#endif // SIGNALPULSE_HPP_INCLUDED
