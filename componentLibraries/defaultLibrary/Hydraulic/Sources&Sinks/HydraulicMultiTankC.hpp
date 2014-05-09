/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicMultiTankC.hpp
//! @author Robert Braun
//! @date   2011-09-02
//!
//! @brief Contains a Hydraulic Multi Port Tank Component of C-type
//!
//$Id$

#ifndef HYDRAULICMULTITANKC_HPP_INCLUDED
#define HYDRAULICMULTITANKC_HPP_INCLUDED

#include <vector>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicMultiTankC : public ComponentC
    {
    private:
        double mP;

        Port *mpMP;
        size_t mNumPorts;
        std::vector<double*> mvpMP_p; //!< @todo Do we really need this, also in non multiport example
        std::vector<double*> mvpMP_q;
        std::vector<double*> mvpMP_c;
        std::vector<double*> mvpMP_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicMultiTankC();
        }

        void configure()
        {
            mpMP = addPowerMultiPort("MP", "NodeHydraulic");
            addConstant("p", "Default pressure", "Pa", 1.0e5, mP);

            disableStartValue(mpMP, NodeHydraulic::Pressure);
            disableStartValue(mpMP, NodeHydraulic::WaveVariable);
            disableStartValue(mpMP, NodeHydraulic::CharImpedance);
        }


        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();
            //! @todo write help function to set the size and contents of a these vectors automatically
            mvpMP_p.resize(mNumPorts);
            mvpMP_q.resize(mNumPorts);
            mvpMP_c.resize(mNumPorts);
            mvpMP_Zc.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpMP_p[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Pressure);
                mvpMP_q[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Flow);
                mvpMP_c[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::WaveVariable);
                mvpMP_Zc[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::CharImpedance);

                *(mvpMP_p[i]) = mP;    //Override the startvalue for the pressure
                *(mvpMP_q[i]) = getDefaultStartValue(mpMP, NodeHydraulic::Flow);
                *(mvpMP_c[i]) = mP;
                *(mvpMP_Zc[i]) = 0.0;
            }
        }


        void simulateOneTimestep()
        {
            //Nothing will change
        }
    };
}

#endif // HYDRAULICTANKC_HPP_INCLUDED
