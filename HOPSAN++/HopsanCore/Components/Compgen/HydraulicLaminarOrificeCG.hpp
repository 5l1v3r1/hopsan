/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, BjÃ¶rn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at LinkÃ¶ping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

#ifndef HYDRAULICLAMINARORIFICECG_HPP_INCLUDED
#define HYDRAULICLAMINARORIFICECG_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @author <>author
//! @date Fri 16 Apr 2010 16:11:09
//! @brief A hydraulic laminar orifice component
//! @ingroup HydraulicLaminarOrificeCG
//!
//This component is generated by COMPGEN for HOPSAN simulation 
//from 
//<>noteBookFile

Class HydraulicLaminarOrificeCG : public ComponentQ
{
private:
     double mKcv
     double mKcv2
     Port mP1
     Port mP2

public:
     static Component *Creator()
     {
        std::cout << "running HydraulicLaminarOrificeCG creator" << \
std::endl;
        return new HydraulicLaminarOrifice("LaminarOrifice");
     }

     HydraulicLaminarOrificeCG(const string name = "LaminarOrifice",
                             const double Kcv = 1.e-11,
                             const double Kcv2 = -11 + 1.*e,
                             const double timestep = 0.001)
        : ComponentQ(name,timestep)
     {
                             mKcv = Kcv,
                             mKcv2 = Kcv2,

		//Add ports to the component
        mP1=addPowerPort("P1","NodeHydraulic");
        mP2=addPowerPort("P2","NodeHydraulic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("Kcv", "Pressure-flow koeff.", "m5/(N s)", mKcv);
        registerParameter("Kcv2", "Pressure-flow koeff.", "m5/(N s)", mKcv2);
     }

    void initialize()
     {
        //Set external parameters
        //Get variable values from nodes
        double p1 = mP1->readNode(NodeHydraulic::PRESSURE);
        double q1 = mP1->readNode(NodeHydraulic::FLOW);
        double p2 = mP2->readNode(NodeHydraulic::PRESSURE);
        double q2 = mP2->readNode(NodeHydraulic::FLOW);

        //LaminarOrifice equations

     }
    void simulateOneTimestep()
     {
        double delayedValue[3][10][10];
        double statevar[3];
        double statevark[3];

        //Get variable values from nodes
        double p1 = mP1->readNode(NodeHydraulic::PRESSURE);
        double q1 = mP1->readNode(NodeHydraulic::FLOW);
        double p2 = mP2->readNode(NodeHydraulic::PRESSURE);
        double q2 = mP2->readNode(NodeHydraulic::FLOW);

        //LaminarOrifice equations
        //Iterative solution using Newton-Rapshson
        statevark[1] = q2;
        statevark[2] = q1;

        for(iter=1,iter<mNoIter,iter++)
        {
          //Algebraic system of equations


          systemEquations[1] =({}[[1]]);
          systemEquations[2] =({}[[2]]);

          //Jacobian matrix

        //Solving equation using LU-faktorisation
        ludcmpw(jacobianMatrix, &Norder, &Norder, indxludcmp, \
&dludcmp,"HydraulicLaminarOrificeCG");
        lubksb(jacobianMatrix, &Norder, &Norder, indxludcmp, systemeqn);

        for(i=1,i<1,i++)
          {
          statevar[i] = statevark[i] - 
          jsyseqnweight[iter - 1] * systemeqn[i];
          }
        for(i=1,i<1,i++)
          {
          statevark[i] = statevar[i];
          }
        }
        double q2 = ((c1 - c2)*mKcv)/(1. + mKcv*(Zc1 + Zc2));
        double q1 = -q2;
        double p1 = c1 + q1*Zc1;
        double p2 = c2 + q2*Zc2;

        //Write new values to nodes
        mP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mP1->writeNode(NodeHydraulic::FLOW, q1);
        mP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mP2->writeNode(NodeHydraulic::FLOW, q2);

        //Update the delayed variabels
        mDelayedEq10.update({}[[1]]);
        mDelayedEq20.update({}[[2]]);

     }
};
endif // HYDRAULICLAMINARORIFICECG_HPP_INCLUDED
