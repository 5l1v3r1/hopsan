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
//! @file   SignalMultiply.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical multiplication function
//!
//$Id$

#ifndef SIGNALMULTIPLY_HPP_INCLUDED
#define SIGNALMULTIPLY_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMultiply : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;
        Port *mpIn1Port, *mpIn2Port;

    public:
        static Component *Creator()
        {
            return new SignalMultiply();
        }

        void configure()
        {
            mpIn1Port = addInputVariable("in1", "", "", 1.0, &mpND_in1);
            mpIn2Port = addInputVariable("in2", "", "", 1.0, &mpND_in2);
            addOutputVariable("out", "in1*in2", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Multiplication equation
            (*mpND_out) = (*mpND_in1) * (*mpND_in2);
        }
    };
}

#endif // SIGNALMULTIPLY_HPP_INCLUDED
