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
//! @file   SignalSquareWave.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a square wave signal generator
//!
//$Id$

///////////////////////////////////////////////////////////
//                â  XXXXX   XXXXX   XXXXX               //
//      Amplitude |  X   X   X   X   X   X               //
//  BaseValue XXXXXXXX   X   X   X   X   XXX             //
//                       X   X   X   X                   //
//                       XXXXX   XXXXX                   //
//                                                       //
//                   â                                   //
//              StartTime                                //
///////////////////////////////////////////////////////////

#ifndef SIGNALSQUAREWAVE_HPP_INCLUDED
#define SIGNALSQUAREWAVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities/AuxiliarySimulationFunctions.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSquareWave : public ComponentSignal
    {

    private:
        double *mpStartTime;
        double *mpFrequency;
        double *mpAmplitude;
        double *mpBaseValue;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSquareWave();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "-", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "-", 1.0, &mpAmplitude);
            addInputVariable("f", "Frequencty", "Hz", 1.0, &mpFrequency);
            addInputVariable("t_start", "Start Time", "s", 0.0, &mpStartTime);


            addOutputVariable("out", "Square wave output", "", &mpOut);
        }


        void initialize()
        {
            // Write basevalue value to node
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            // Step Equations
            if (mTime < (*mpStartTime))
            {
                (*mpOut) = (*mpBaseValue);
            }
            else
            {
                if ( sin( (mTime-(*mpStartTime))*2.0*pi*(*mpFrequency) ) >= 0.0 )
                {
                    (*mpOut) = (*mpBaseValue) + (*mpAmplitude);
                }
                else
                {
                    (*mpOut) = (*mpBaseValue) - (*mpAmplitude);
                }
            }
        }
    };
}

#endif // SIGNALSQUAREWAVE_HPP_INCLUDED
