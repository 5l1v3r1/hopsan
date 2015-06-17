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

#ifndef SIGNALTIMEACCELERATOR_HPP_INCLUDED
#define SIGNALTIMEACCELERATOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file SignalTimeAccelerator.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Tue 14 Apr 2015 16:48:37
//! @brief Accelerate time in mission simulation
//! @ingroup SignalComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Signal, \
Control}/SignalControlAero.nb*/

using namespace hopsan;

class SignalTimeAccelerator : public ComponentSignal
{
private:
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[3];
     int mNstep;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double timecomp;
     double massflow;
     double vxcg;
     double vycg;
     //outputVariables
     double timeE;
     double massflowE;
     double xcgE;
     double ycgE;
     //Expressions variables
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mptimecomp;
     double *mpmassflow;
     double *mpvxcg;
     double *mpvycg;
     //inputParameters pointers
     //outputVariables pointers
     double *mptimeE;
     double *mpmassflowE;
     double *mpxcgE;
     double *mpycgE;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart30;
     Delay mDelayedPart31;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new SignalTimeAccelerator();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(3,3);
        systemEquations.create(3);
        delayedPart.create(4,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        //Add inputVariables to the component
            addInputVariable("timecomp","time compression \
rate","",1.,&mptimecomp);
            addInputVariable("massflow","Mass flow \
rate","kg/s",0.,&mpmassflow);
            addInputVariable("vxcg","x-position","m",0.,&mpvxcg);
            addInputVariable("vycg","y-position","m",0.,&mpvycg);

        //Add inputParammeters to the component
        //Add outputVariables to the component
            addOutputVariable("timeE","effective time","sec",0.,&mptimeE);
            addOutputVariable("massflowE","Effective Mass flow \
rate","kg",0.,&mpmassflowE);
            addOutputVariable("xcgE","Effective x-position","m",0.,&mpxcgE);
            addOutputVariable("ycgE","Effective y-position","m",0.,&mpycgE);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,3);
     }

    void initialize()
     {
        //Read port variable pointers from nodes

        //Read variables from nodes

        //Read inputVariables from nodes
        timecomp = (*mptimecomp);
        massflow = (*mpmassflow);
        vxcg = (*mpvxcg);
        vycg = (*mpvycg);

        //Read inputParameters from nodes

        //Read outputVariables from nodes
        timeE = (*mptimeE);
        massflowE = (*mpmassflowE);
        xcgE = (*mpxcgE);
        ycgE = (*mpycgE);

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (-(mTimestep*timecomp*vxcg) - 2*xcgE)/2.;
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts2[1] = (-(mTimestep*timecomp*vycg) - 2*ycgE)/2.;
        mDelayedPart21.initialize(mNstep,delayParts2[1]);
        delayParts3[1] = (-(mTimestep*timecomp) - 2*timeE)/2.;
        mDelayedPart31.initialize(mNstep,delayParts3[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];

        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        Vec stateVar(3);
        Vec stateVark(3);
        Vec deltaStateVar(3);

        //Read variables from nodes

        //Read inputVariables from nodes
        timecomp = (*mptimecomp);
        massflow = (*mpmassflow);
        vxcg = (*mpvxcg);
        vycg = (*mpvycg);

        //Read inputParameters from nodes

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = xcgE;
        stateVark[1] = ycgE;
        stateVark[2] = timeE;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //TimeAccelerator
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =-(mTimestep*timecomp*vxcg)/2. + xcgE + \
delayedPart[1][1];
          systemEquations[1] =-(mTimestep*timecomp*vycg)/2. + ycgE + \
delayedPart[2][1];
          systemEquations[2] =-(mTimestep*timecomp)/2. + timeE + \
delayedPart[3][1];

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          xcgE=stateVark[0];
          ycgE=stateVark[1];
          timeE=stateVark[2];
          //Expressions
          massflowE = massflow*timecomp;
        }

        //Calculate the delayed parts
        delayParts1[1] = (-(mTimestep*timecomp*vxcg) - 2*xcgE)/2.;
        delayParts2[1] = (-(mTimestep*timecomp*vycg) - 2*ycgE)/2.;
        delayParts3[1] = (-(mTimestep*timecomp) - 2*timeE)/2.;

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];

        //Write new values to nodes
        //outputVariables
        (*mptimeE)=timeE;
        (*mpmassflowE)=massflowE;
        (*mpxcgE)=xcgE;
        (*mpycgE)=ycgE;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart21.update(delayParts2[1]);
        mDelayedPart31.update(delayParts3[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // SIGNALTIMEACCELERATOR_HPP_INCLUDED
