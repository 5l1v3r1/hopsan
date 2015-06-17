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

//!
//! @file   Hydraulic42Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-05-04
//!
//! @brief Contains a hydraulic 4/2-valve of Q-type
//$Id$

#ifndef HYDRAULIC42VALVE2_HPP_INCLUDED
#define HYDRAULIC42VALVE2_HPP_INCLUDED


#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/2-valve (no closed position) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic42Valve2 : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mSpoolPosTF;
        TurbulentFlowFunction mQTurb_pa;
        TurbulentFlowFunction mQTurb_pb;
        TurbulentFlowFunction mQTurb_at;
        TurbulentFlowFunction mQTurb_bt;

        // Port and node data pointers
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn, *mpOut;
        double *mpPP_p, *mpPP_q, *mpPT_p, *mpPT_q, *mpPA_p, *mpPA_q, *mpPB_p, *mpPB_q;
        double *mpPP_c, *mpPP_Zc, *mpPT_c, *mpPT_Zc, *mpPA_c, *mpPA_Zc, *mpPB_c, *mpPB_Zc;

        double *mpXvIn, *mpXv, *mpCq, *mpD, *mpF_pa, *mpF_pb, *mpF_at, *mpF_bt, *mpXvmax, *mpRho;

        // Constants
        double mOmegah, mDeltah;

    public:
        static Component *Creator()
        {
            return new Hydraulic42Valve2();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);

            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);
            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil Density", "kg/m^3", 890, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_pa);
            addInputVariable("f_pb", "Fraction of spool circumference that is opening P-B", "-", 1.0, &mpF_pb);
            addInputVariable("f_at", "Fraction of spool circumference that is opening A-T", "-", 1.0, &mpF_at);
            addInputVariable("f_bt", "Fraction of spool circumference that is opening B-T", "-", 1.0, &mpF_bt);
            addInputVariable("x_vmax", "Maximum Spool Displacement", "m", 0.01, &mpXvmax);

            addConstant("omega_h", "Resonance frequency", "Frequency", 100.0, mOmegah);
            addConstant("delta_h", "Damping factor", "-", 1.0, mDeltah);
        }


        void initialize()
        {
            mpPP_p = getSafeNodeDataPtr(mpPP, NodeHydraulic::Pressure);
            mpPP_q = getSafeNodeDataPtr(mpPP, NodeHydraulic::Flow);
            mpPP_c = getSafeNodeDataPtr(mpPP, NodeHydraulic::WaveVariable);
            mpPP_Zc = getSafeNodeDataPtr(mpPP, NodeHydraulic::CharImpedance);

            mpPT_p = getSafeNodeDataPtr(mpPT, NodeHydraulic::Pressure);
            mpPT_q = getSafeNodeDataPtr(mpPT, NodeHydraulic::Flow);
            mpPT_c = getSafeNodeDataPtr(mpPT, NodeHydraulic::WaveVariable);
            mpPT_Zc = getSafeNodeDataPtr(mpPT, NodeHydraulic::CharImpedance);

            mpPA_p = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpPA_q = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpPA_c = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpPA_Zc = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            mpPB_p = getSafeNodeDataPtr(mpPB, NodeHydraulic::Pressure);
            mpPB_q = getSafeNodeDataPtr(mpPB, NodeHydraulic::Flow);
            mpPB_c = getSafeNodeDataPtr(mpPB, NodeHydraulic::WaveVariable);
            mpPB_Zc = getSafeNodeDataPtr(mpPB, NodeHydraulic::CharImpedance);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDeltah/mOmegah, 1.0/(mOmegah*mOmegah)};
            double initXv = limit(*mpXv,-(*mpXvmax),(*mpXvmax));
            mSpoolPosTF.initialize(mTimestep, num, den, initXv, initXv, -(*mpXvmax), (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double cp, Zcp, ct, Zct, ca, Zca, cb, Zcb, xvin, xv, xpanom, xpbnom, xatnom, xbtnom, Kcpa, Kcpb, Kcat, Kcbt, qpa, qpb, qat, qbt, qp, qa, qb, qt, pa, pb, pt, pp;
            double Cq, rho, xvmax, d, f_pa, f_pb, f_at, f_bt;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ct  = (*mpPT_c);
            Zct = (*mpPT_Zc);
            ca  = (*mpPA_c);
            Zca = (*mpPA_Zc);
            cb  = (*mpPB_c);
            Zcb = (*mpPB_Zc);

            xvin  = (*mpXvIn);
            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_pb = (*mpF_pb);
            f_at = (*mpF_at);
            f_bt = (*mpF_bt);

            limitValue(xvin, 0.0, xvmax);
            mSpoolPosTF.update(xvin);
            xv = mSpoolPosTF.value();

            //Valve equations
            xpanom = xv;
            xpbnom = xvmax-xv;
            xatnom = xvmax-xv;
            xbtnom = xv;

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcpb = Cq*f_pb*pi*d*xpbnom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);
            Kcbt = Cq*f_bt*pi*d*xbtnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            mQTurb_pa.setFlowCoefficient(Kcpa);
            mQTurb_pb.setFlowCoefficient(Kcpb);
            mQTurb_at.setFlowCoefficient(Kcat);
            mQTurb_bt.setFlowCoefficient(Kcbt);

            qpa = mQTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qpb = mQTurb_pb.getFlow(cp, cb, Zcp, Zcb);
            qat = mQTurb_at.getFlow(ca, ct, Zca, Zct);
            qbt = mQTurb_bt.getFlow(cb, ct, Zcb, Zct);

            qp = -qpa-qpb;
            qa = qpa-qat;
            qb = -qbt+qpb;
            qt = qat+qbt;

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;
            pb = cb + qb*Zcb;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
                cav = true;
            }
            if(pb < 0.0)
            {
                cb = 0.0;
                Zcb = 0;
                cav = true;
            }
            if(pp < 0.0)
            {
                cp = 0.0;
                Zcp = 0;
                cav = true;
            }
            if(pt < 0.0)
            {
                ct = 0.0;
                Zct = 0;
                cav = true;
            }

            if(cav)
            {
                qpa = mQTurb_pa.getFlow(cp, ca, Zcp, Zca);
                qpb = mQTurb_pb.getFlow(cp, cb, Zcp, Zcb);
                qat = mQTurb_at.getFlow(ca, ct, Zca, Zct);
                qbt = mQTurb_bt.getFlow(cb, ct, Zcb, Zct);

                qp = -qpa-qpb;
                qa = qpa-qat;
                qb = -qbt+qpb;
                qt = qat+qbt;

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
                pb = cb + qb*Zcb;
            }

            //Write new values to nodes

            (*mpPP_p) = cp + qp*Zcp;
            (*mpPP_q) = qp;
            (*mpPT_p) = ct + qt*Zct;
            (*mpPT_q) = qt;
            (*mpPA_p) = ca + qa*Zca;
            (*mpPA_q) = qa;
            (*mpPB_p) = cb + qb*Zcb;
            (*mpPB_q) = qb;
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULIC42VALVE2_HPP_INCLUDED
