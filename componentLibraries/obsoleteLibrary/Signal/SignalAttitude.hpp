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

#ifndef SIGNALATTITUDE_HPP_INCLUDED
#define SIGNALATTITUDE_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"


//!
//! @file SignalAttitude.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 21 Sep 2011 22:09:17
//! @brief Attitude control unit for an aircraft
//! @ingroup SignalComponents
//!
//This component is generated by COMPGEN for HOPSAN-NG simulation 
//from 
/*{, C:, Documents and Settings, petkr14, My Documents, \
CompgenNG}/FlightControlNG1.nb*/

using namespace hopsan;

class SignalAttitude : public ComponentSignal
{
private:
     double mKphi;
     double mKphipsi;
     double mphimin;
     double mphimax;
     double mKelev;
     double mKrud;
     double mu1min;
     double mu1max;
     double mu2min;
     double mu2max;
     double mu3min;
     double mu3max;
     double mU0;
     double mphiref;
     double mthetaref;
     double mpsiref;
     double mphi;
     double mtheta;
     double mpsi;
     double mbeta;
     double mUb;
     Port *mpPphiref;
     Port *mpPthetaref;
     Port *mpPpsiref;
     Port *mpPphi;
     Port *mpPtheta;
     Port *mpPpsi;
     Port *mpPbeta;
     Port *mpPUb;
     Port *mpPuaerL;
     Port *mpPuaerR;
     Port *mpPuelev;
     Port *mpPurud;
     int mNstep;
     //inputVariables
     double phiref;
     double thetaref;
     double psiref;
     double phi;
     double theta;
     double psi;
     double beta;
     double Ub;
     //outputVariables
     double uaerL;
     double uaerR;
     double uelev;
     double urud;
     //Delay declarations
     //inputVariables pointers
     double *mpND_phiref;
     double *mpND_thetaref;
     double *mpND_psiref;
     double *mpND_phi;
     double *mpND_theta;
     double *mpND_psi;
     double *mpND_beta;
     double *mpND_Ub;
     //outputVariables pointers
     double *mpND_uaerL;
     double *mpND_uaerR;
     double *mpND_uelev;
     double *mpND_urud;

public:
     static Component *Creator()
     {
        return new SignalAttitude();
     }

     SignalAttitude(const double Kphi = 0.1
                             ,const double Kphipsi = 1.
                             ,const double phimin = -1.
                             ,const double phimax = 1.
                             ,const double Kelev = 0.03
                             ,const double Krud = 0.5
                             ,const double u1min = -0.6
                             ,const double u1max = 0.6
                             ,const double u2min = 0.6
                             ,const double u2max = 0.6
                             ,const double u3min = -0.6
                             ,const double u3max = 0.6
                             ,const double U0 = 100.
                             ,const double phiref = 0.
                             ,const double thetaref = 0.
                             ,const double psiref = 0.
                             ,const double phi = 0.
                             ,const double theta = 0.
                             ,const double psi = 0.
                             ,const double beta = 0.
                             ,const double Ub = 0.
                             )
        : ComponentSignal()
     {
        mNstep=9;
        mKphi = Kphi;
        mKphipsi = Kphipsi;
        mphimin = phimin;
        mphimax = phimax;
        mKelev = Kelev;
        mKrud = Krud;
        mu1min = u1min;
        mu1max = u1max;
        mu2min = u2min;
        mu2max = u2max;
        mu3min = u3min;
        mu3max = u3max;
        mU0 = U0;
        mphiref = phiref;
        mthetaref = thetaref;
        mpsiref = psiref;
        mphi = phi;
        mtheta = theta;
        mpsi = psi;
        mbeta = beta;
        mUb = Ub;

        //Add ports to the component

        //Add inputVariables ports to the component
        mpPphiref=addReadPort("Pphiref","NodeSignal", Port::NOTREQUIRED);
        mpPthetaref=addReadPort("Pthetaref","NodeSignal", Port::NOTREQUIRED);
        mpPpsiref=addReadPort("Ppsiref","NodeSignal", Port::NOTREQUIRED);
        mpPphi=addReadPort("Pphi","NodeSignal", Port::NOTREQUIRED);
        mpPtheta=addReadPort("Ptheta","NodeSignal", Port::NOTREQUIRED);
        mpPpsi=addReadPort("Ppsi","NodeSignal", Port::NOTREQUIRED);
        mpPbeta=addReadPort("Pbeta","NodeSignal", Port::NOTREQUIRED);
        mpPUb=addReadPort("PUb","NodeSignal", Port::NOTREQUIRED);

        //Add outputVariables ports to the component
        mpPuaerL=addWritePort("PuaerL","NodeSignal", Port::NOTREQUIRED);
        mpPuaerR=addWritePort("PuaerR","NodeSignal", Port::NOTREQUIRED);
        mpPuelev=addWritePort("Puelev","NodeSignal", Port::NOTREQUIRED);
        mpPurud=addWritePort("Purud","NodeSignal", Port::NOTREQUIRED);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("Kphi", "Gain roll", "rad", mKphi);
        registerParameter("Kphipsi", "Gain yaw/roll", "rad", mKphipsi);
        registerParameter("phimin", "Minium bank angle for turn", "rad", \
mphimin);
        registerParameter("phimax", "Maximum bank angle for turn", "rad", \
mphimax);
        registerParameter("Kelev", "Gain tip, default", "rad", mKelev);
        registerParameter("Krud", "Gain yaw, default", "rad", mKrud);
        registerParameter("u1min", "Minium output signal roll", "rad", \
mu1min);
        registerParameter("u1max", "Maximum output signal roll", "rad", \
mu1max);
        registerParameter("u2min", "Minium output signal tip", "rad", \
mu2min);
        registerParameter("u2max", "Maximum output signal tip", "rad", \
mu2max);
        registerParameter("u3min", "Minium output signal yaw", "rad", \
mu3min);
        registerParameter("u3max", "Maximum output signal yaw", "rad", \
mu3max);
        registerParameter("U0", "Reference movement for compensation", "m/s", \
mU0);
        registerParameter("phiref", "Reference signal roll", "rad", mphiref);
        registerParameter("thetaref", "Reference signal tip", "rad", \
mthetaref);
        registerParameter("psiref", "Reference signal yaw", "rad", mpsiref);
        registerParameter("phi", "roll angle", "rad", mphi);
        registerParameter("theta", "tipp angle", "rad", mtheta);
        registerParameter("psi", "yaw angle", "rad", mpsi);
        registerParameter("beta", "side slip angle", "rad", mbeta);
        registerParameter("Ub", "actual movement", "m/s", mUb);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Read inputVariables pointers from nodes
        mpND_phiref=getSafeNodeDataPtr(mpPphiref, NodeSignal::VALUE,mphiref);
        mpND_thetaref=getSafeNodeDataPtr(mpPthetaref, \
NodeSignal::VALUE,mthetaref);
        mpND_psiref=getSafeNodeDataPtr(mpPpsiref, NodeSignal::VALUE,mpsiref);
        mpND_phi=getSafeNodeDataPtr(mpPphi, NodeSignal::VALUE,mphi);
        mpND_theta=getSafeNodeDataPtr(mpPtheta, NodeSignal::VALUE,mtheta);
        mpND_psi=getSafeNodeDataPtr(mpPpsi, NodeSignal::VALUE,mpsi);
        mpND_beta=getSafeNodeDataPtr(mpPbeta, NodeSignal::VALUE,mbeta);
        mpND_Ub=getSafeNodeDataPtr(mpPUb, NodeSignal::VALUE,mUb);
        //Read outputVariable pointers from nodes
        mpND_uaerL=getSafeNodeDataPtr(mpPuaerL, NodeSignal::VALUE);
        mpND_uaerR=getSafeNodeDataPtr(mpPuaerR, NodeSignal::VALUE);
        mpND_uelev=getSafeNodeDataPtr(mpPuelev, NodeSignal::VALUE);
        mpND_urud=getSafeNodeDataPtr(mpPurud, NodeSignal::VALUE);

        //Read variables from nodes

        //Read inputVariables from nodes
        phiref = (*mpND_phiref);
        thetaref = (*mpND_thetaref);
        psiref = (*mpND_psiref);
        phi = (*mpND_phi);
        theta = (*mpND_theta);
        psi = (*mpND_psi);
        beta = (*mpND_beta);
        Ub = (*mpND_Ub);

        //Read outputVariables from nodes
        uaerL = mpPuaerL->getStartValue(NodeSignal::VALUE);
        uaerR = mpPuaerR->getStartValue(NodeSignal::VALUE);
        uelev = mpPuelev->getStartValue(NodeSignal::VALUE);
        urud = mpPurud->getStartValue(NodeSignal::VALUE);


        //LocalExpressions
//        double Kv = Power(mU0,2)/(Power(mU0,2) + Power(Abs(Ub),2));
//        double u1cmin = Kv*mu1min;
//        double u1cmax = Kv*mu1max;
//        double u2cmin = Kv*mu2min;
//        double u2cmax = Kv*mu2max;
//        double u3cmin = Kv*mu3min;
//        double u3cmax = Kv*mu3max;

        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes

        //Read inputVariables from nodes
        phiref = (*mpND_phiref);
        thetaref = (*mpND_thetaref);
        psiref = (*mpND_psiref);
        phi = (*mpND_phi);
        theta = (*mpND_theta);
        psi = (*mpND_psi);
        beta = (*mpND_beta);
        Ub = (*mpND_Ub);

        //LocalExpressions
        double Kv = Power(mU0,2)/(Power(mU0,2) + Power(Abs(Ub),2));
        double u1cmin = Kv*mu1min;
        double u1cmax = Kv*mu1max;
        double u2cmin = Kv*mu2min;
        double u2cmax = Kv*mu2max;
        double u3cmin = Kv*mu3min;
        double u3cmax = Kv*mu3max;

        //Expressions
        double uaerL = limit(-(Kv*mKphi*(diffAngle(phiref,phi) + \
limit(mKphipsi*diffAngle(psiref,psi),mphimin,mphimax))),u1cmin,u1cmax);
        double uaerR = limit(Kv*mKphi*(diffAngle(phiref,phi) + \
limit(mKphipsi*diffAngle(psiref,psi),mphimin,mphimax)),u1cmin,u1cmax);
        double uelev = \
limit(Kv*mKelev*diffAngle(thetaref,theta),u2cmin,u2cmax);
        double urud = limit(-(beta*Kv*mKrud),u3cmin,u3cmax);

        //Calculate the delayed parts


        //Write new values to nodes
        //outputVariables
        (*mpND_uaerL)=uaerL;
        (*mpND_uaerR)=uaerR;
        (*mpND_uelev)=uelev;
        (*mpND_urud)=urud;

        //Update the delayed variabels

     }
};
#endif // SIGNALATTITUDE_HPP_INCLUDED
