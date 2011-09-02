#ifndef MECHANICJLINK_HPP_INCLUDED
#define MECHANICJLINK_HPP_INCLUDED

#include <iostream>
//#include <Qt/QDebug.h>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"
#include <math.h>
#include "matrix.h"

//!
//! @file MechanicJLink.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 10 Aug 2011 18:22:02
//! @brief Link with inertia
//! @ingroup MechanicComponents
//!
//This component is generated by COMPGEN for HOPSAN-NG simulation 
//from 
/*{, C:, Documents and Settings, petkr14, My Documents, \
CompgenNG}/Mechanic1DNG.nb*/

using namespace hopsan;

class MechanicJLink : public ComponentQ
{
private:
     double mJL;
     double mBL;
     double mlink;
     double mx0;
     double mtheta0;
     double mthetamin;
     double mthetamax;
     Port *mpPm1;
     Port *mpPmr2;
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
     int jsyseqnweight[4];
     int order[4];
     int mNstep;
     //Port Pm1 variable
     double fm1;
     double xm1;
     double vm1;
     double cm1;
     double Zcm1;
     double eqMassm1;
     //Port Pmr2 variable
     double tormr2;
     double thetamr2;
     double wmr2;
     double cmr2;
     double Zcmr2;
     double eqInertiamr2;
     //inputVariables
     //outputVariables
     //Port Pm1 pointer
     double *mpND_fm1;
     double *mpND_xm1;
     double *mpND_vm1;
     double *mpND_cm1;
     double *mpND_Zcm1;
     double *mpND_eqMassm1;
     //Port Pmr2 pointer
     double *mpND_tormr2;
     double *mpND_thetamr2;
     double *mpND_wmr2;
     double *mpND_cmr2;
     double *mpND_Zcmr2;
     double *mpND_eqInertiamr2;
     //Delay declarations
     //inputVariables pointers
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart22;

public:
     static Component *Creator()
     {
        std::cout << "running MechanicJLink creator" << std::endl;
        return new MechanicJLink("JLink");
     }

     MechanicJLink(const std::string name = "JLink"
                             ,const double JL = 1.
                             ,const double BL = 1.
                             ,const double link = 0.1
                             ,const double x0 = 0.1
                             ,const double theta0 = 0.1
                             ,const double thetamin = -1.05
                             ,const double thetamax = 1.05
                             )
        : ComponentQ(name)
     {
        mNstep=9;
        jacobianMatrix.create(4,4);
        systemEquations.create(4);
        delayedPart.create(5,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;

        mJL = JL;
        mBL = BL;
        mlink = link;
        mx0 = x0;
        mtheta0 = theta0;
        mthetamin = thetamin;
        mthetamax = thetamax;

        //Add ports to the component
        mpPm1=addPowerPort("Pm1","NodeMechanic");
        mpPmr2=addPowerPort("Pmr2","NodeMechanicRotational");

        //Add inputVariables ports to the component

        //Add outputVariables ports to the component

        //Register changable parameters to the HOPSAN++ core
        registerParameter("J_L", "Equivalent inertia at node 2", "kgm2", mJL);
        registerParameter("B_L", "Visc friction coeff. at node 2", "Ns/rad", \
mBL);
        registerParameter("L_link", "Link length x1/sin(thetarot2)", "", \
mlink);
        registerParameter("x_0", "x position for zero angle", "", mx0);
        registerParameter("theta_0", "link angle for zero angle", "", \
mtheta0);
        registerParameter("theta_min", "Min angle", "rad", mthetamin);
        registerParameter("theta_max", "Max angle", "rad", mthetamax);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pm1
        mpND_fm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::FORCE);
        mpND_xm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::POSITION);
        mpND_vm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::VELOCITY);
        mpND_cm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::WAVEVARIABLE);
        mpND_Zcm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::CHARIMP);
        mpND_eqMassm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::EQMASS);
        //Port Pmr2
        mpND_tormr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::TORQUE);
        mpND_thetamr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::ANGLE);
        mpND_wmr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::ANGULARVELOCITY);
        mpND_cmr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::WAVEVARIABLE);
        mpND_Zcmr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::CHARIMP);
        mpND_eqInertiamr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::EQINERTIA);
        //Read inputVariables pointers from nodes
        //Read outputVariable pointers from nodes

        //Read variables from nodes
        //Port Pm1
        fm1 = (*mpND_fm1);
        xm1 = (*mpND_xm1);
        vm1 = (*mpND_vm1);
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        eqMassm1 = (*mpND_eqMassm1);
        //Port Pmr2
        tormr2 = (*mpND_tormr2);
        thetamr2 = (*mpND_thetamr2);
        wmr2 = (*mpND_wmr2);
        cmr2 = (*mpND_cmr2);
        Zcmr2 = (*mpND_Zcmr2);
        eqInertiamr2 = (*mpND_eqInertiamr2);

        //Read inputVariables from nodes

        //Read outputVariables from nodes



        //Initialize delays
        delayParts1[1] = (mTimestep*tormr2 - 2*mJL*wmr2 + mBL*mTimestep*wmr2 \
- fm1*mlink*mTimestep*Cos(mtheta0 + thetamr2))/(2*mJL + mBL*mTimestep);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts2[1] = (-8*mJL*mtheta0 - 8*mJL*thetamr2 + \
2*Power(mTimestep,2)*tormr2 + 2*mBL*Power(mTimestep,2)*wmr2 - \
2*fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL);
        mDelayedPart21.initialize(mNstep,delayParts2[1]);
        delayParts2[2] = (4*mJL*mtheta0 + 4*mJL*thetamr2 + \
Power(mTimestep,2)*tormr2 + mBL*Power(mTimestep,2)*wmr2 - \
fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL);
        mDelayedPart22.initialize(mNstep,delayParts2[2]);
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
        eqMassm1 = (*mpND_eqMassm1);
        //Port Pmr2
        cmr2 = (*mpND_cmr2);
        Zcmr2 = (*mpND_Zcmr2);

        //Read inputVariables from nodes

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = wmr2;
        stateVark[1] = thetamr2;
        stateVark[2] = fm1;
        stateVark[3] = tormr2;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //JLink
         //Differential-algebraic system of equation parts
          delayParts1[1] = (mTimestep*tormr2 - 2*mJL*wmr2 + \
mBL*mTimestep*wmr2 - fm1*mlink*mTimestep*Cos(mtheta0 + thetamr2))/(2*mJL + \
mBL*mTimestep);
          delayParts2[1] = (-8*mJL*mtheta0 - 8*mJL*thetamr2 + \
2*Power(mTimestep,2)*tormr2 + 2*mBL*Power(mTimestep,2)*wmr2 - \
2*fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL);
          delayParts2[2] = (4*mJL*mtheta0 + 4*mJL*thetamr2 + \
Power(mTimestep,2)*tormr2 + mBL*Power(mTimestep,2)*wmr2 - \
fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL);

          delayedPart[1][1] = delayParts1[1];
          delayedPart[2][1] = delayParts2[1];
          delayedPart[2][2] = mDelayedPart22.getIdx(1);
          delayedPart[3][1] = delayParts3[1];
          delayedPart[4][1] = delayParts4[1];

          //Assemble differential-algebraic equations
          systemEquations[0] =wmr2 - \
dxLimit(thetamr2,mthetamin,mthetamax)*(-((mTimestep*(tormr2 - \
fm1*mlink*Cos(mtheta0 + thetamr2)))/(2*mJL + mBL*mTimestep)) - \
delayedPart[1][1]);
          systemEquations[1] =thetamr2 - limit(-(4*mJL*mtheta0 + \
Power(mTimestep,2)*(tormr2 + mBL*wmr2) - \
fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL) - \
delayedPart[2][1] - delayedPart[2][2],mthetamin,mthetamax);
          systemEquations[2] =-cm1 + fm1 + mlink*wmr2*Zcm1*Cos(thetamr2);
          systemEquations[3] =-cmr2 + tormr2 - wmr2*Zcmr2;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = \
(fm1*mlink*mTimestep*dxLimit(thetamr2,mthetamin,mthetamax)*Sin(mtheta0 + \
thetamr2))/(2*mJL + mBL*mTimestep);
          jacobianMatrix[0][2] = -((mlink*mTimestep*Cos(mtheta0 + \
thetamr2)*dxLimit(thetamr2,mthetamin,mthetamax))/(2*mJL + mBL*mTimestep));
          jacobianMatrix[0][3] = \
(mTimestep*dxLimit(thetamr2,mthetamin,mthetamax))/(2*mJL + mBL*mTimestep);
          jacobianMatrix[1][0] = \
(mBL*Power(mTimestep,2)*dxLimit(-(4*mJL*mtheta0 + Power(mTimestep,2)*(tormr2 \
+ mBL*wmr2) - fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL) \
- delayedPart[2][1] - delayedPart[2][2],mthetamin,mthetamax))/(4.*mJL);
          jacobianMatrix[1][1] = 1 + \
(fm1*mlink*Power(mTimestep,2)*dxLimit(-(4*mJL*mtheta0 + \
Power(mTimestep,2)*(tormr2 + mBL*wmr2) - \
fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL) - \
delayedPart[2][1] - delayedPart[2][2],mthetamin,mthetamax)*Sin(mtheta0 + \
thetamr2))/(4.*mJL);
          jacobianMatrix[1][2] = -(mlink*Power(mTimestep,2)*Cos(mtheta0 + \
thetamr2)*dxLimit(-(4*mJL*mtheta0 + Power(mTimestep,2)*(tormr2 + mBL*wmr2) - \
fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL) - \
delayedPart[2][1] - delayedPart[2][2],mthetamin,mthetamax))/(4.*mJL);
          jacobianMatrix[1][3] = (Power(mTimestep,2)*dxLimit(-(4*mJL*mtheta0 \
+ Power(mTimestep,2)*(tormr2 + mBL*wmr2) - \
fm1*mlink*Power(mTimestep,2)*Cos(mtheta0 + thetamr2))/(4.*mJL) - \
delayedPart[2][1] - delayedPart[2][2],mthetamin,mthetamax))/(4.*mJL);
          jacobianMatrix[2][0] = mlink*Zcm1*Cos(thetamr2);
          jacobianMatrix[2][1] = -(mlink*wmr2*Zcm1*Sin(thetamr2));
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = -Zcmr2;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;

          //Solving equation using LU-faktorisation
          ludcmp(jacobianMatrix, order);
          solvlu(jacobianMatrix,systemEquations,deltaStateVar,order);

        for(i=0;i<4;i++)
          {
          stateVar[i] = stateVark[i] - 
          jsyseqnweight[iter - 1] * deltaStateVar[i];
          }
        for(i=0;i<4;i++)
          {
          stateVark[i] = stateVar[i];
          }
        }
        wmr2=stateVark[0];
        thetamr2=stateVark[1];
        fm1=stateVark[2];
        tormr2=stateVark[3];
        //Expressions
        double vm1 = -(mlink*wmr2*Cos(thetamr2));
        double xm1 = mx0 - mlink*Sin(thetamr2);
        double eqMassm1 = (mJL*Power(Sec(thetamr2),2))/Power(mlink,2);
        double eqInertiamr2 = mJL;

        //Write new values to nodes
        //Port Pm1
        (*mpND_fm1)=fm1;
        (*mpND_xm1)=xm1;
        (*mpND_vm1)=vm1;
        //Port Pmr2
        (*mpND_tormr2)=tormr2;
        (*mpND_thetamr2)=thetamr2;
        (*mpND_wmr2)=wmr2;
        (*mpND_eqInertiamr2)=eqInertiamr2;
        //outputVariables

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart21.update(delayParts2[1]);
        mDelayedPart22.update(delayParts2[2]);

     }
};
#endif // MECHANICJLINK_HPP_INCLUDED
