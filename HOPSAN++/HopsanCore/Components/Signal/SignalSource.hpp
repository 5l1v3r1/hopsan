//!
//! @file   SignalSource.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Source Component
//!
//$Id$

#ifndef SIGNALSOURCE_HPP_INCLUDED
#define SIGNALSOURCE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSource : public ComponentSignal
    {

    private:
        double mValue;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSource("Source");
        }

        SignalSource(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSource";
            mValue = 1.0;

            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("Value", "Source Value", "-", mValue);
        }


        void initialize()
        {
            //Initialize value to the node
            mpOut->writeNode(NodeSignal::VALUE, mValue);
        }


        void simulateOneTimestep()
        {
            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, mValue);
        }
    };





    class SignalOptimizedSource : public ComponentSignal
    {

    private:
        double mValue;
        Port *mpOut;
        double *out;

    public:
        static Component *Creator()
        {
            return new SignalOptimizedSource("OptimizedSource");
        }

        SignalOptimizedSource(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalOptimizedSource";
            mValue = 1.0;

            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("Value", "Source Value", "-", mValue);
        }


        void initialize()
        {
            //Initialize value to the node
            out = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            *out = mValue;
        }


        void simulateOneTimestep()
        {
            //Nothing changes because it's a constant
        }
    };
}

#endif // SIGNALSOURCE_HPP_INCLUDED
