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

#ifndef HYDRAULICFUELTANKG_HPP_INCLUDED
#define HYDRAULICFUELTANKG_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicFuelTankG.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Sun 6 Apr 2014 13:18:16
//! @brief Calulates the mass of remaining fuel in tank
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, CompgenModels}/HydraulicComponents.nb*/

using namespace hopsan;

class HydraulicFuelTankG : public ComponentC
{
private:
     double rhofuel;
     double p0;
     double hf;
     double massfuel0;
     double massfuelmax;
     Port *mpPT;
     double delayParts1[9];
     double delayParts2[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[1];
     int mNstep;
     //Port PT variable
     double pT;
     double qT;
     double TT;
     double dET;
     double cT;
     double ZcT;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double gx;
     //outputVariables
     double massfuel;
     double consfuel;
     //Expressions variables
     double hx;
     //Port PT pointer
     double *mpND_pT;
     double *mpND_qT;
     double *mpND_TT;
     double *mpND_dET;
     double *mpND_cT;
     double *mpND_ZcT;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpgx;
     //inputParameters pointers
     double *mprhofuel;
     double *mpp0;
     double *mphf;
     double *mpmassfuel0;
     double *mpmassfuelmax;
     //outputVariables pointers
     double *mpmassfuel;
     double *mpconsfuel;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new HydraulicFuelTankG();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(1,1);
        systemEquations.create(1);
        delayedPart.create(2,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPT=addPowerPort("PT","NodeHydraulic");
        //Add inputVariables to the component
            addInputVariable("gx","acceleration","m/s2",9.82,&mpgx);

        //Add inputParammeters to the component
            addInputVariable("rhofuel", "Fuel density", "kg/m3", \
700.,&mprhofuel);
            addInputVariable("p0", "tank pressure", "Pa", 100000.,&mpp0);
            addInputVariable("hf", "fuel in tank height", "m/s2", 5.,&mphf);
            addInputVariable("massfuel0", "The intitial fuel mass", "kg/s", \
1000.,&mpmassfuel0);
            addInputVariable("massfuelmax", "fuelmass at full tank", "kg", \
1000.,&mpmassfuelmax);
        //Add outputVariables to the component
            addOutputVariable("massfuel","Fuel mass","kg",0.,&mpmassfuel);
            addOutputVariable("consfuel","Consumed fuel \
mass","kg",0.,&mpconsfuel);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,1);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port PT
        mpND_pT=getSafeNodeDataPtr(mpPT, NodeHydraulic::Pressure);
        mpND_qT=getSafeNodeDataPtr(mpPT, NodeHydraulic::Flow);
        mpND_TT=getSafeNodeDataPtr(mpPT, NodeHydraulic::Temperature);
        mpND_dET=getSafeNodeDataPtr(mpPT, NodeHydraulic::HeatFlow);
        mpND_cT=getSafeNodeDataPtr(mpPT, NodeHydraulic::WaveVariable);
        mpND_ZcT=getSafeNodeDataPtr(mpPT, NodeHydraulic::CharImpedance);

        //Read variables from nodes
        //Port PT
        pT = (*mpND_pT);
        qT = (*mpND_qT);
        TT = (*mpND_TT);
        dET = (*mpND_dET);
        cT = (*mpND_cT);
        ZcT = (*mpND_ZcT);

        //Read inputVariables from nodes
        gx = (*mpgx);

        //Read inputParameters from nodes
        rhofuel = (*mprhofuel);
        p0 = (*mpp0);
        hf = (*mphf);
        massfuel0 = (*mpmassfuel0);
        massfuelmax = (*mpmassfuelmax);

        //Read outputVariables from nodes
        massfuel = (*mpmassfuel);
        consfuel = (*mpconsfuel);

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (-2*consfuel + mTimestep*qT*rhofuel)/2.;
        mDelayedPart11.initialize(mNstep,delayParts1[1]);

        delayedPart[1][1] = delayParts1[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(1);
        Vec stateVark(1);
        Vec deltaStateVar(1);

        //Read variables from nodes
        //Port PT
        pT = (*mpND_pT);
        qT = (*mpND_qT);
        dET = (*mpND_dET);

        //Read inputVariables from nodes
        gx = (*mpgx);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = consfuel;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //FuelTankG
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =consfuel - limit(-(mTimestep*qT*rhofuel)/2. - \
delayedPart[1][1],0.,0.0001 + massfuel0);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          consfuel=stateVark[0];
          //Expressions
          massfuel = -consfuel + massfuel0;
          hx = (hf*massfuel)/massfuelmax;
          cT = (p0 + gx*hx*rhofuel)*onPositive(massfuel);
          ZcT = 0.;
        }

        //Calculate the delayed parts
        delayParts1[1] = (-2*consfuel + mTimestep*qT*rhofuel)/2.;

        delayedPart[1][1] = delayParts1[1];

        //Write new values to nodes
        //Port PT
        (*mpND_TT)=TT;
        (*mpND_cT)=cT;
        (*mpND_ZcT)=ZcT;
        //outputVariables
        (*mpmassfuel)=massfuel;
        (*mpconsfuel)=consfuel;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // HYDRAULICFUELTANKG_HPP_INCLUDED
