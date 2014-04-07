#ifndef ELECTRICBATTERY_HPP_INCLUDED
#define ELECTRICBATTERY_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file ElectricBattery.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Mon 7 Apr 2014 13:06:59
//! @brief Battery with static behaviour
//! @ingroup ElectricComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, ComponentLibraries, defaultLibrary, \
Electric}/ElectricBattery.nb*/

using namespace hopsan;

class ElectricBattery : public ComponentQ
{
private:
     double cond;
     double unom;
     double capacity;
     double kappa;
     double e;
     Port *mpPel1;
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
     //Port Pel1 variable
     double uel1;
     double iel1;
     double cel1;
     double Zcel1;
//==This code has been autogenerated using Compgen==
     //inputVariables
     //outputVariables
     double soc;
     double ubatt;
     //Port Pel1 pointer
     double *mpND_uel1;
     double *mpND_iel1;
     double *mpND_cel1;
     double *mpND_Zcel1;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     //inputParameters pointers
     double *mpcond;
     double *mpunom;
     double *mpcapacity;
     double *mpkappa;
     double *mpe;
     //outputVariables pointers
     double *mpsoc;
     double *mpubatt;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new ElectricBattery();
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
        mpPel1=addPowerPort("Pel1","NodeElectric");
        //Add inputVariables to the component

        //Add inputParammeters to the component
            addInputVariable("cond", "conductance (at 1)", "1/ohm", \
1000.,&mpcond);
            addInputVariable("unom", "nominal voltage of battery", "V", \
12.,&mpunom);
            addInputVariable("capacity", "capacity", "Ah", 41.,&mpcapacity);
            addInputVariable("kappa", "exponent of discharge function", "", \
0.1,&mpkappa);
            addInputVariable("e", "e", "", 2.71828,&mpe);
        //Add outputVariables to the component
            addOutputVariable("soc","soc","",1.,&mpsoc);
            addOutputVariable("ubatt","battery voltage","V",0.,&mpubatt);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,4);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pel1
        mpND_uel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Voltage);
        mpND_iel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Current);
        mpND_cel1=getSafeNodeDataPtr(mpPel1, NodeElectric::WaveVariable);
        mpND_Zcel1=getSafeNodeDataPtr(mpPel1, NodeElectric::CharImpedance);

        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpND_uel1);
        iel1 = (*mpND_iel1);
        cel1 = (*mpND_cel1);
        Zcel1 = (*mpND_Zcel1);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        cond = (*mpcond);
        unom = (*mpunom);
        capacity = (*mpcapacity);
        kappa = (*mpkappa);
        e = (*mpe);

        //Read outputVariables from nodes
        soc = (*mpsoc);
        ubatt = (*mpubatt);

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (iel1*mTimestep - \
7200*capacity*soc)/(7200.*capacity);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);

        delayedPart[1][1] = delayParts1[1];
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
        //Port Pel1
        cel1 = (*mpND_cel1);
        Zcel1 = (*mpND_Zcel1);

        //Read inputVariables from nodes

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = soc;
        stateVark[1] = iel1;
        stateVark[2] = ubatt;
        stateVark[3] = uel1;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //Battery
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =soc - limit(-(iel1*mTimestep)/(7200.*capacity) \
- delayedPart[1][1],0.001,0.999);
          systemEquations[1] =iel1 + cond*(-ubatt + uel1);
          systemEquations[2] =ubatt - (Power(2,kappa)*(-1 + \
Power(e,(10*soc)/kappa))*unom*Power(asin(limit(soc,1.e-9,0.9999)),kappa))/(1 \
+ Power(e,(10*soc)/kappa));
          systemEquations[3] =-cel1 + uel1 - iel1*Zcel1;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = \
(mTimestep*dxLimit(-(iel1*mTimestep)/(7200.*capacity) - \
delayedPart[1][1],0.001,0.999))/(7200.*capacity);
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = -cond;
          jacobianMatrix[1][3] = cond;
          jacobianMatrix[2][0] = -((Power(2,kappa)*(-1 + \
Power(e,(10*soc)/kappa))*kappa*unom*Power(asin(limit(soc,1.e-9,0.9999)),-1 + \
kappa)*dxLimit(soc,1.e-9,0.9999))/((1 + Power(e,(10*soc)/kappa))*Sqrt(1 - \
Power(limit(soc,1.e-9,0.9999),2)))) + (5*Power(2,1 + \
kappa)*Power(e,(10*soc)/kappa)*(-1 + \
Power(e,(10*soc)/kappa))*unom*Power(asin(limit(soc,1.e-9,0.9999)),kappa)*log(\
e))/(Power(1 + Power(e,(10*soc)/kappa),2)*kappa) - (5*Power(2,1 + \
kappa)*Power(e,(10*soc)/kappa)*unom*Power(asin(limit(soc,1.e-9,0.9999)),kappa\
)*log(e))/((1 + Power(e,(10*soc)/kappa))*kappa);
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = -Zcel1;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          soc=stateVark[0];
          iel1=stateVark[1];
          ubatt=stateVark[2];
          uel1=stateVark[3];
        }

        //Calculate the delayed parts
        delayParts1[1] = (iel1*mTimestep - \
7200*capacity*soc)/(7200.*capacity);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        //Write new values to nodes
        //Port Pel1
        (*mpND_uel1)=uel1;
        (*mpND_iel1)=iel1;
        //outputVariables
        (*mpsoc)=soc;
        (*mpubatt)=ubatt;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // ELECTRICBATTERY_HPP_INCLUDED
