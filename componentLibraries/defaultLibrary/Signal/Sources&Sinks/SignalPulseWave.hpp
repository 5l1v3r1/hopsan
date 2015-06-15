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
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-03-29
//!
//! @brief Contains a pulse wave (train) signal generator
//!
//$Id$

#ifndef SIGNALPULSEWAVE_HPP
#define SIGNALPULSEWAVE_HPP

#include "ComponentEssentials.h"
#include <cmath>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalPulseWave : public ComponentSignal
    {

    private:
        double *mpBaseValue;
        double *mpStartTime;
        double *mpPeriodT;
        double *mpDutyCycle;
        double *mpAmplitude;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalPulseWave();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);
            addInputVariable("t_start", "Start Time", "Time", 0.0, &mpStartTime);
            addInputVariable("dT", "Time Period", "Time", 1.0, &mpPeriodT);
            addInputVariable("D", "Duty Cycle, (ratio 0<=x<=1)", "", 0.5, &mpDutyCycle);

            addOutputVariable("out", "PulseWave", "", &mpOut);
        }


        void initialize()
        {
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            // +0.5*mTimestep to avoid ronding issues
            const double time = (mTime-(*mpStartTime)+0.5*mTimestep);
            const double periodT = (*mpPeriodT);
            const bool high = (time - std::floor(time/periodT)*periodT) < (*mpDutyCycle)*periodT;

            if ( (time > 0) && high)
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude);     //During pulse
            }
            else
            {
                (*mpOut) = (*mpBaseValue);                  //Not during pulse
            }
        }
    };
}

#endif // SIGNALPULSEWAVE_HPP
