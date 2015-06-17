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
//! @file   SignalLP2Filter.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2010-06-10
//!
//! @brief Contains a Signal Second Order Low Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALLP2FILTER_HPP_INCLUDED
#define SIGNALLP2FILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalLP2Filter : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mTF2;
        double mW, mD, mMin, mMax;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalLP2Filter();
        }

        void configure()
        {
            addInputVariable("in","","", 0.0, &mpND_in);
            addOutputVariable("out", "","",0.0, &mpND_out);

            addConstant("omega", "Break frequency", "rad/s", 1000.0, mW);
            addConstant("delta", "Damp coefficient", "-", 1.0, mD);
            addConstant("y_min", "Lower output limit", "-", -1.5E+300, mMin);
            addConstant("y_max", "Upper output limit", "-", 1.5E+300, mMax);
        }


        void initialize()
        {
            double num[3];
            double den[3];

            num[2] = 0.0;
            num[1] = 0.0;
            num[0] = 1.0;
            den[2] = 1.0/(mW*mW);
            den[1] = 2.0*mD/mW;
            den[0] = 1.0;

            mTF2.initialize(mTimestep, num, den, (*mpND_in), (*mpND_out), mMin, mMax);

            // Do not write initial value to out port, its startvalue is used to initialize the filter
        }


        void simulateOneTimestep()
        {
            //Write new values to nodes
            (*mpND_out) = mTF2.update((*mpND_in));
        }
    };
}

#endif // SIGNALLP2FILTER_HPP_INCLUDED


