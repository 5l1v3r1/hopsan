/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicPlugQ.hpp
//! @author FluMeS
//! @date   2013-05-02
//!
//! @brief Contains a Hydraulic Plug of Q-type
//!
//$Id$

#ifndef HYDRAULICPLUG_HPP_INCLUDED
#define HYDRAULICPLUG_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPlugQ : public ComponentQ
    {
    private:
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicPlugQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p, q, c, Zc;

            //Read variables from nodes
            c = (*mpP1_c);
            Zc = (*mpP1_Zc);

            //Flow source equations
            q = 0.0;
            p = c + q*Zc;

            if(p<0)
            {
                p=0;
            }

            (*mpP1_p) = p;
            (*mpP1_q) = q;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICPLUG_HPP_INCLUDED
