#ifndef SIGNALFFBLOOP_HPP_INCLUDED
#define SIGNALFFBLOOP_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file SignalFFBloop.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Thu 2 May 2013 10:54:02
//! @brief FFBD loop out
//! @ingroup SignalComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/SignalFFBDcomponents.nb*/

using namespace hopsan;

class SignalFFBloop : public ComponentSignal
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
     //outputVariables pointers
     double *mpstate;
     double *mpout0;
     double *mpout1;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new SignalFFBloop();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        //Add inputVariables to the component
            addInputVariable("in0","Input 0","",0.,&mpin0);
            addInputVariable("divert","Input 0","",0.,&mpdivert);

        //Add outputVariables to the component
            addOutputVariable("state","State activated","",0.,&mpstate);
            addOutputVariable("out0","exiting to alt 0","",0.,&mpout0);
            addOutputVariable("out1","exiting to alt 0","",0.,&mpout1);

//==This code has been autogenerated using Compgen==
        //Add constants/parameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes

        //Read variables from nodes

        //Read inputVariables from nodes
        in0 = (*mpin0);
        divert = (*mpdivert);

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
#endif // SIGNALFFBLOOP_HPP_INCLUDED
