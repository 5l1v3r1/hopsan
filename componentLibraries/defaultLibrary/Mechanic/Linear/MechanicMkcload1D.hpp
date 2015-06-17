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

#ifndef MECHANICMKCLOAD1D_HPP_INCLUDED
#define MECHANICMKCLOAD1D_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file MechanicMkcload1D.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Sat 14 Feb 2015 22:46:57
//! @brief An inertia load with spring, damper and coulomb friction
//! @ingroup MechanicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Mechanic, \
Linear}/MechanicMkcload1D.nb*/

using namespace hopsan;

class MechanicMkcload1D : public ComponentQ
{
private:
     double mL;
     double bL;
     double kL;
     double fc;
     double bfc;
     double xmin;
     double xmax;
     Port *mpPm1;
     Port *mpPm2;
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
     int order[4];
     int mNstep;
     //Port Pm1 variable
     double fm1;
     double xm1;
     double vm1;
     double cm1;
     double Zcm1;
     double eqMassm1;
     //Port Pm2 variable
     double fm2;
     double xm2;
     double vm2;
     double cm2;
     double Zcm2;
     double eqMassm2;
//==This code has been autogenerated using Compgen==
     //inputVariables
     //outputVariables
     double fs;
     //InitialExpressions variables
     //Expressions variables
     //Port Pm1 pointer
     double *mpND_fm1;
     double *mpND_xm1;
     double *mpND_vm1;
     double *mpND_cm1;
     double *mpND_Zcm1;
     double *mpND_eqMassm1;
     //Port Pm2 pointer
     double *mpND_fm2;
     double *mpND_xm2;
     double *mpND_vm2;
     double *mpND_cm2;
     double *mpND_Zcm2;
     double *mpND_eqMassm2;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     //inputParameters pointers
     double *mpmL;
     double *mpbL;
     double *mpkL;
     double *mpfc;
     double *mpbfc;
     double *mpxmin;
     double *mpxmax;
     //outputVariables pointers
     double *mpfs;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart12;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new MechanicMkcload1D();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(4,4);
        systemEquations.create(4);
        delayedPart.create(5,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPm1=addPowerPort("Pm1","NodeMechanic");
        mpPm2=addPowerPort("Pm2","NodeMechanic");
        //Add inputVariables to the component

        //Add inputParammeters to the component
            addInputVariable("mL", "Inertia", "kg", 1000.,&mpmL);
            addInputVariable("bL", "Visc. friction coeff.", "Ns/m", \
10.,&mpbL);
            addInputVariable("kL", "Spring constant", "N/m", 10.,&mpkL);
            addInputVariable("fc", "Dry friction (+/-)", "N", 10.,&mpfc);
            addInputVariable("bfc", "Numerical friction factor.", "", \
1.,&mpbfc);
            addInputVariable("xmin", "Limitation on stroke", "m", \
0.,&mpxmin);
            addInputVariable("xmax", "Limitation on stroke", "m", \
1.,&mpxmax);
        //Add outputVariables to the component
            addOutputVariable("fs","Spring force","N",1000.,&mpfs);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,4);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pm1
        mpND_fm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
        mpND_xm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
        mpND_vm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
        mpND_cm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
        mpND_Zcm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::CharImpedance);
        mpND_eqMassm1=getSafeNodeDataPtr(mpPm1, \
NodeMechanic::EquivalentMass);
        //Port Pm2
        mpND_fm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::Force);
        mpND_xm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::Position);
        mpND_vm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::Velocity);
        mpND_cm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::WaveVariable);
        mpND_Zcm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::CharImpedance);
        mpND_eqMassm2=getSafeNodeDataPtr(mpPm2, \
NodeMechanic::EquivalentMass);

        //Read variables from nodes
        //Port Pm1
        fm1 = (*mpND_fm1);
        xm1 = (*mpND_xm1);
        vm1 = (*mpND_vm1);
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        eqMassm1 = (*mpND_eqMassm1);
        //Port Pm2
        fm2 = (*mpND_fm2);
        xm2 = (*mpND_xm2);
        vm2 = (*mpND_vm2);
        cm2 = (*mpND_cm2);
        Zcm2 = (*mpND_Zcm2);
        eqMassm2 = (*mpND_eqMassm2);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        mL = (*mpmL);
        bL = (*mpbL);
        kL = (*mpkL);
        fc = (*mpfc);
        bfc = (*mpbfc);
        xmin = (*mpxmin);
        xmax = (*mpxmax);

        //Read outputVariables from nodes
        fs = (*mpfs);

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        xm1 = -xm2;


        //Initialize delays
        delayParts1[1] = (-2*fm1*Power(mTimestep,2) + \
2*fm2*Power(mTimestep,2) - 8*mL*xm2 + 2*kL*Power(mTimestep,2)*xm2 + \
2*Power(mTimestep,2)*limit((bfc*mL*vm2)/mTimestep,-fc,fc))/(4*mL + \
2*bL*mTimestep + kL*Power(mTimestep,2));
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts1[2] = (-(fm1*Power(mTimestep,2)) + fm2*Power(mTimestep,2) \
+ 4*mL*xm2 - 2*bL*mTimestep*xm2 + kL*Power(mTimestep,2)*xm2 + \
Power(mTimestep,2)*limit((bfc*mL*vm2)/mTimestep,-fc,fc))/(4*mL + \
2*bL*mTimestep + kL*Power(mTimestep,2));
        mDelayedPart12.initialize(mNstep,delayParts1[2]);
        delayParts2[1] = (-(fm1*mTimestep) + fm2*mTimestep - 2*mL*vm2 + \
bL*mTimestep*vm2 + kL*mTimestep*xm2 + \
mTimestep*limit((bfc*mL*vm2)/mTimestep,-fc,fc))/(2*mL + bL*mTimestep);
        mDelayedPart21.initialize(mNstep,delayParts2[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(1);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(4);
        Vec stateVark(4);
        Vec deltaStateVar(4);

        //Read variables from nodes
        //Port Pm1
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        //Port Pm2
        cm2 = (*mpND_cm2);
        Zcm2 = (*mpND_Zcm2);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        mL = (*mpmL);
        bL = (*mpbL);
        kL = (*mpkL);
        fc = (*mpfc);
        bfc = (*mpbfc);
        xmin = (*mpxmin);
        xmax = (*mpxmax);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = xm2;
        stateVark[1] = vm2;
        stateVark[2] = fm1;
        stateVark[3] = fm2;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //Mkcload1D
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =xm2 - limit(-((Power(mTimestep,2)*(-fm1 + fm2 + \
limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + mTimestep*(2*bL + \
kL*mTimestep))) - delayedPart[1][1] - delayedPart[1][2],xmin,xmax);
          systemEquations[1] =vm2 - dxLimit(limit(-((Power(mTimestep,2)*(-fm1 \
+ fm2 + limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + mTimestep*(2*bL + \
kL*mTimestep))) - delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax),xmin,xmax)*(-((mTimestep*(-fm1 + fm2 + kL*xm2 + \
limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(2*mL + bL*mTimestep)) - \
delayedPart[2][1]);
          systemEquations[2] =-cm1 + fm1 + vm2*Zcm1;
          systemEquations[3] =-cm2 + fm2 - vm2*Zcm2;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = \
(bfc*mL*mTimestep*dxLimit((bfc*mL*vm2)/mTimestep,-fc,fc)*dxLimit(-((Power(mTi\
mestep,2)*(-fm1 + fm2 + limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + \
mTimestep*(2*bL + kL*mTimestep))) - delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax))/(4*mL + mTimestep*(2*bL + kL*mTimestep));
          jacobianMatrix[0][2] = \
-((Power(mTimestep,2)*dxLimit(-((Power(mTimestep,2)*(-fm1 + fm2 + \
limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + mTimestep*(2*bL + \
kL*mTimestep))) - delayedPart[1][1] - delayedPart[1][2],xmin,xmax))/(4*mL + \
mTimestep*(2*bL + kL*mTimestep)));
          jacobianMatrix[0][3] = \
(Power(mTimestep,2)*dxLimit(-((Power(mTimestep,2)*(-fm1 + fm2 + \
limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + mTimestep*(2*bL + \
kL*mTimestep))) - delayedPart[1][1] - delayedPart[1][2],xmin,xmax))/(4*mL + \
mTimestep*(2*bL + kL*mTimestep));
          jacobianMatrix[1][0] = \
(kL*mTimestep*dxLimit(limit(-((Power(mTimestep,2)*(-fm1 + fm2 + \
limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + mTimestep*(2*bL + \
kL*mTimestep))) - delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax),xmin,xmax))/(2*mL + bL*mTimestep);
          jacobianMatrix[1][1] = 1 + \
(bfc*mL*dxLimit((bfc*mL*vm2)/mTimestep,-fc,fc)*dxLimit(limit(-((Power(mTimest\
ep,2)*(-fm1 + fm2 + limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + \
mTimestep*(2*bL + kL*mTimestep))) - delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax),xmin,xmax))/(2*mL + bL*mTimestep);
          jacobianMatrix[1][2] = \
-((mTimestep*dxLimit(limit(-((Power(mTimestep,2)*(-fm1 + fm2 + \
limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + mTimestep*(2*bL + \
kL*mTimestep))) - delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax),xmin,xmax))/(2*mL + bL*mTimestep));
          jacobianMatrix[1][3] = \
(mTimestep*dxLimit(limit(-((Power(mTimestep,2)*(-fm1 + fm2 + \
limit((bfc*mL*vm2)/mTimestep,-fc,fc)))/(4*mL + mTimestep*(2*bL + \
kL*mTimestep))) - delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax),xmin,xmax))/(2*mL + bL*mTimestep);
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = Zcm1;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = -Zcm2;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          xm2=stateVark[0];
          vm2=stateVark[1];
          fm1=stateVark[2];
          fm2=stateVark[3];
          //Expressions
          vm1 = -vm2;
          xm1 = -xm2;
          fs = kL*xm2;
          eqMassm1 = mL;
          eqMassm2 = mL;
        }

        //Calculate the delayed parts
        delayParts1[1] = (-2*fm1*Power(mTimestep,2) + \
2*fm2*Power(mTimestep,2) - 8*mL*xm2 + 2*kL*Power(mTimestep,2)*xm2 + \
2*Power(mTimestep,2)*limit((bfc*mL*vm2)/mTimestep,-fc,fc))/(4*mL + \
2*bL*mTimestep + kL*Power(mTimestep,2));
        delayParts1[2] = (-(fm1*Power(mTimestep,2)) + fm2*Power(mTimestep,2) \
+ 4*mL*xm2 - 2*bL*mTimestep*xm2 + kL*Power(mTimestep,2)*xm2 + \
Power(mTimestep,2)*limit((bfc*mL*vm2)/mTimestep,-fc,fc))/(4*mL + \
2*bL*mTimestep + kL*Power(mTimestep,2));
        delayParts2[1] = (-(fm1*mTimestep) + fm2*mTimestep - 2*mL*vm2 + \
bL*mTimestep*vm2 + kL*mTimestep*xm2 + \
mTimestep*limit((bfc*mL*vm2)/mTimestep,-fc,fc))/(2*mL + bL*mTimestep);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(0);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        //Write new values to nodes
        //Port Pm1
        (*mpND_fm1)=fm1;
        (*mpND_xm1)=xm1;
        (*mpND_vm1)=vm1;
        (*mpND_eqMassm1)=eqMassm1;
        //Port Pm2
        (*mpND_fm2)=fm2;
        (*mpND_xm2)=xm2;
        (*mpND_vm2)=vm2;
        (*mpND_eqMassm2)=eqMassm2;
        //outputVariables
        (*mpfs)=fs;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart12.update(delayParts1[2]);
        mDelayedPart21.update(delayParts2[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // MECHANICMKCLOAD1D_HPP_INCLUDED
