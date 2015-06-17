/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef SIGNALFFBAND_HPP_INCLUDED
#define SIGNALFFBAND_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file SignalFFBand.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Fri 28 Jun 2013 13:01:39
//! @brief FFBD AND
//! @ingroup SignalComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/SignalFFBDcomponents.nb*/

using namespace hopsan;

class SignalFFBand : public ComponentSignal
{
private:
     int mNstep;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double in0;
     double in1;
     //outputVariables
     double state;
     double exiting;
     //InitialExpressions variables
     double oldIn0;
     double oldIn1;
     double oldExiting;
     //Expressions variables
     double state0;
     double state1;
     double oldState0;
     double oldState1;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpin0;
     double *mpin1;
     //inputParameters pointers
     //outputVariables pointers
     double *mpstate;
     double *mpexiting;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new SignalFFBand();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        //Add inputVariables to the component
            addInputVariable("in0","Input 0","",0.,&mpin0);
            addInputVariable("in1","Input 1","",0.,&mpin1);

        //Add inputParammeters to the component
        //Add outputVariables to the component
            addOutputVariable("state","State activated","",0.,&mpstate);
            addOutputVariable("exiting","exiting to alt 0","",0.,&mpexiting);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes

        //Read variables from nodes

        //Read inputVariables from nodes
        in0 = (*mpin0);
        in1 = (*mpin1);

        //Read inputParameters from nodes

        //Read outputVariables from nodes
        state = (*mpstate);
        exiting = (*mpexiting);

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        oldIn0 = in0;
        oldIn1 = in1;
        oldExiting = exiting;


        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes

        //Read inputVariables from nodes
        in0 = (*mpin0);
        in1 = (*mpin1);

        //LocalExpressions

          //Expressions
          state0 = onPositive(-0.5 - onPositive(-0.5 + oldExiting) + \
2*onPositive(-0.5 + in0 - oldIn0) + onPositive(-0.5 + oldState0));
          state1 = onPositive(-0.5 - onPositive(-0.5 + oldExiting) + \
2*onPositive(-0.5 + in1 - oldIn1) + onPositive(-0.5 + oldState1));
          exiting = oldState0*oldState1;
          state = onPositive(-0.5 + onPositive(-0.5 + state0) + \
onPositive(-0.5 + state1));
          oldState0 = state0;
          oldState1 = state1;
          oldIn0 = in0;
          oldIn1 = in1;
          oldExiting = exiting;

        //Calculate the delayed parts


        //Write new values to nodes
        //outputVariables
        (*mpstate)=state;
        (*mpexiting)=exiting;

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // SIGNALFFBAND_HPP_INCLUDED
