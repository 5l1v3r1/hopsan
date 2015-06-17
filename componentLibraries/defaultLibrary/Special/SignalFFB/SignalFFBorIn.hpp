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

#ifndef SIGNALFFBORIN_HPP_INCLUDED
#define SIGNALFFBORIN_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file SignalFFBorIn.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Fri 28 Jun 2013 13:01:39
//! @brief FFBD or in
//! @ingroup SignalComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/SignalFFBDcomponents.nb*/

using namespace hopsan;

class SignalFFBorIn : public ComponentSignal
{
private:
     int mNstep;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double in0;
     double divert;
     //outputVariables
     double state;
     double out0;
     double out1;
     //InitialExpressions variables
     double oldState;
     double oldIn0;
     double oldOut0;
     double oldOut1;
     //Expressions variables
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpin0;
     double *mpdivert;
     //inputParameters pointers
     //outputVariables pointers
     double *mpstate;
     double *mpout0;
     double *mpout1;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new SignalFFBorIn();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        //Add inputVariables to the component
            addInputVariable("in0","Input 0","",0.,&mpin0);
            addInputVariable("divert","Input 0","",0.,&mpdivert);

        //Add inputParammeters to the component
        //Add outputVariables to the component
            addOutputVariable("state","State activated","",0.,&mpstate);
            addOutputVariable("out0","exiting to alt 0","",0.,&mpout0);
            addOutputVariable("out1","exiting to alt 0","",0.,&mpout1);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes

        //Read variables from nodes

        //Read inputVariables from nodes
        in0 = (*mpin0);
        divert = (*mpdivert);

        //Read inputParameters from nodes

        //Read outputVariables from nodes
        state = (*mpstate);
        out0 = (*mpout0);
        out1 = (*mpout1);

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        oldState = state;
        oldIn0 = in0;
        oldOut0 = out0;
        oldOut1 = out1;


        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes

        //Read inputVariables from nodes
        in0 = (*mpin0);
        divert = (*mpdivert);

        //LocalExpressions

          //Expressions
          state = onPositive(-0.5 + 2*onPositive(-0.5 + in0 - oldIn0) - \
onPositive(-0.5 + oldOut0 + oldOut1) + onPositive(-0.5 + oldState));
          out0 = state*onPositive(0.5 - divert);
          out1 = state*onPositive(-0.5 + divert);
          oldState = state;
          oldIn0 = in0;
          oldOut0 = out0;
          oldOut1 = out1;

        //Calculate the delayed parts


        //Write new values to nodes
        //outputVariables
        (*mpstate)=state;
        (*mpout0)=out0;
        (*mpout1)=out1;

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // SIGNALFFBORIN_HPP_INCLUDED
