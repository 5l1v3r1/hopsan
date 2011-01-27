//!
//! @file   SignalIntegratorLimited.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-17
//!
//! @brief Contains a Signal Integrator Component with Limited Output
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegratorLimited : public ComponentSignal
    {

    private:
        double mMin, mMax;
        double mPrevU, mPrevY;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited("IntegratorLimited");
        }

        SignalIntegratorLimited(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegratorLimited";

            mMin = -1.5E+300;   //! @todo Shouldn't these be registered parameters?
            mMax = 1.5E+300;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            mPrevU = startY;
            limitValue(startY, mMin, mMax);

            limitValue((*mpND_in), mMin, mMax);
        }


        void simulateOneTimestep()
        {
            //Filter equations
            //Bilinear transform is used
            (*mpND_out) = mPrevY + mTimestep/2.0*((*mpND_in) + mPrevU);

            if ((*mpND_out) >= mMax)
            {
                (*mpND_out) = mMax;
            }
            else if ((*mpND_out) <= mMin)
            {
                (*mpND_out) = mMin;
            }

            //Update filter:
            mPrevU = (*mpND_in);
            mPrevY = (*mpND_out);
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED_HPP_INCLUDED


