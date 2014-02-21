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
//! @file   SecondOrderTransferFunction.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-23
//!
//! @brief Contains the Core Second Transfer Function class
//!
//$Id$

//#include <iostream>
//#include <cassert>
#include <algorithm>
#include "ComponentUtilities/SecondOrderTransferFunction.h"

using namespace hopsan;

//! @class hopsan::SecondOrderTransferFunction
//! @ingroup ComponentUtilityClasses
//! @brief The SecondOrderTransferFunction class implements a second order transfer function using bilinear transform
//!
//! To declare a filter like \f[G=\frac{a_2 s^2 + a_1 s + a_0}{b_2 s^2 + b_1 s + b_0}\f]
//! the syntax is filter.setNumDen(num, den)
//! where \f$num[0]=a_0\f$, \f$num[1]=a_1\f$, \f$num[2]=a_2\f$
//! and \f$den[0]=b_0\f$, \f$den[1]=b_1\f$, \f$den[2]=b_2\f$,
//!

void SecondOrderTransferFunction::initialize(double timestep, double num[3], double den[3], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mDelayU[0] = u0;
    mDelayU[1] = u0;
    mDelayY[0] = std::max(std::min(y0, mMax), mMin);
    mDelayY[1] = mDelayY[0];
    mTimeStep = timestep;
    setNumDen(num, den);
}


void SecondOrderTransferFunction::setNum(double num[3])
{
    mCoeffU[0] = num[0]*mTimeStep*mTimeStep + 2.0*num[1]*mTimeStep + 4.0*num[2];
    mCoeffU[1] = 2.0*num[0]*mTimeStep*mTimeStep - 8.0*num[2];
    mCoeffU[2] = num[0]*mTimeStep*mTimeStep - 2.0*num[1]*mTimeStep + 4.0*num[2];
}


void SecondOrderTransferFunction::setDen(double den[3])
{
    mCoeffY[0] = den[0]*mTimeStep*mTimeStep + 2.0*den[1]*mTimeStep + 4.0*den[2];
    mCoeffY[1] = 2.0*den[0]*mTimeStep*mTimeStep - 8.0*den[2];
    mCoeffY[2] = den[0]*mTimeStep*mTimeStep - 2.0*den[1]*mTimeStep + 4.0*den[2];
}


void SecondOrderTransferFunction::setNumDen(double num[3], double den[3])
{
//num =
//(c*T^2*q^2 + 2*c*T^2*q + c*T^2 - 2*b*T*q^2 + 2*b*T + 4*a*q^2 - 8*a*q + 4*a)
//den =
//(C*T^2*q^2 + 2*C*T^2*q + C*T^2 - 2*B*T*q^2 + 2*B*T + 4*A*q^2 - 8*A*q + 4*A)

    mCoeffU[0] = num[0]*mTimeStep*mTimeStep + 2.0*num[1]*mTimeStep + 4.0*num[2];
    mCoeffU[1] = 2.0*num[0]*mTimeStep*mTimeStep - 8.0*num[2];
    mCoeffU[2] = num[0]*mTimeStep*mTimeStep - 2.0*num[1]*mTimeStep + 4.0*num[2];

    mCoeffY[0] = den[0]*mTimeStep*mTimeStep + 2.0*den[1]*mTimeStep + 4.0*den[2];
    mCoeffY[1] = 2.0*den[0]*mTimeStep*mTimeStep - 8.0*den[2];
    mCoeffY[2] = den[0]*mTimeStep*mTimeStep - 2.0*den[1]*mTimeStep + 4.0*den[2];

//    mCoeffU[0] = mTimeStep*mTimeStep*(num[2]*mTimeStep*mTimeStep - 2*num[1]*mTimeStep + 4*num[0]);
//    mCoeffU[1] = -mTimeStep*mTimeStep*(4*mTimeStep*num[1] - 4*mTimeStep*mTimeStep*num[2]);
//    mCoeffU[2] = -mTimeStep*mTimeStep*(8*num[0] - 6*mTimeStep*mTimeStep*num[2]);
//    mCoeffU[3] = mTimeStep*mTimeStep*(4*num[2]*mTimeStep*mTimeStep + 4*num[1]*mTimeStep);
//    mCoeffU[4] = mTimeStep*mTimeStep*(num[2]*mTimeStep*mTimeStep + 2*num[1]*mTimeStep + 4*num[0]); //To newest U
//
//    mCoeffY[0] = NoDelayAndPointersIntegrator*(den[2]*NoDelayAndPointersIntegrator - 2*den[1]*mTimeStep + 4*den[0]);
//    mCoeffY[1] = -NoDelayAndPointersIntegrator*(4*mTimeStep*den[1] - 4*NoDelayAndPointersIntegrator*den[2]);
//    mCoeffY[2] = -NoDelayAndPointersIntegrator*(8*den[0] - 6*NoDelayAndPointersIntegrator*den[2]);
//    mCoeffY[3] = NoDelayAndPointersIntegrator*(4*den[2]*NoDelayAndPointersIntegrator + 4*den[1]*mTimeStep);
//    mCoeffY[4] = NoDelayAndPointersIntegrator*(den[2]*NoDelayAndPointersIntegrator + 2*den[1]*mTimeStep + 4*den[0]);
}


void SecondOrderTransferFunction::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void SecondOrderTransferFunction::initializeValues(double u0, double y0)
{
    mDelayU[0] = u0;
    mDelayU[1] = u0;
    mDelayY[0] = y0;
    mDelayY[1] = y0;
}


double SecondOrderTransferFunction::update(double u)
{
    mValue = 1.0/mCoeffY[0]*(mCoeffU[0]*u + mCoeffU[1]*mDelayU[0] + mCoeffU[2]*mDelayU[1] - (mCoeffY[1]*mDelayY[0] + mCoeffY[2]*mDelayY[1]));

    if (mValue > mMax)
    {
        mDelayU[1] = mMax;
        mDelayU[0] = mMax;
        mDelayY[1] = mMax;
        mDelayY[0] = mMax;
        mValue     = mMax;
    }
    else if (mValue < mMin)
    {
        mDelayU[1] = mMin;
        mDelayU[0] = mMin;
        mDelayY[1] = mMin;
        mDelayY[0] = mMin;
        mValue     = mMin;
    }
    else
    {
        mDelayU[1] = mDelayU[0];
        mDelayU[0] = u;
        mDelayY[1] = mDelayY[0];
        mDelayY[0] = mValue;
    }

    return mValue;
}


//! Return current filter output value
//! @return The filtered actual value.
double SecondOrderTransferFunction::value()
{
    return mValue;
}





void SecondOrderTransferFunctionVariable::initialize(double *pTimeStep, double num[3], double den[3], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mDelayU[0] = u0;
    mDelayU[1] = u0;
    mDelayY[0] = std::max(std::min(y0, mMax), mMin);
    mDelayY[1] = mDelayY[0];
    mpTimeStep = pTimeStep;
    mPrevTimeStep = (*pTimeStep);
    setNumDen(num, den);
}


void SecondOrderTransferFunctionVariable::setNum(double num[3])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    mNum[2] = num[2];
    recalculateCoefficients();
}


void SecondOrderTransferFunctionVariable::setDen(double den[3])
{
    mDen[0] = den[0];
    mDen[1] = den[1];
    mDen[2] = den[2];
    recalculateCoefficients();
}


void SecondOrderTransferFunctionVariable::setNumDen(double num[3], double den[3])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    mNum[2] = num[2];
    mDen[0] = den[0];
    mDen[1] = den[1];
    mDen[2] = den[2];
    recalculateCoefficients();
}


void SecondOrderTransferFunctionVariable::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void SecondOrderTransferFunctionVariable::initializeValues(double u0, double y0)
{
    mDelayU[0] = u0;
    mDelayU[1] = u0;
    mDelayY[0] = y0;
    mDelayY[1] = y0;
}


double SecondOrderTransferFunctionVariable::update(double u)
{
    if(mPrevTimeStep != (*mpTimeStep))
    {
        mPrevTimeStep = (*mpTimeStep);
        recalculateCoefficients();
    }

    mValue = 1.0/mCoeffY[0]*(mCoeffU[0]*u + mCoeffU[1]*mDelayU[0] + mCoeffU[2]*mDelayU[1] - (mCoeffY[1]*mDelayY[0] + mCoeffY[2]*mDelayY[1]));

    if (mValue > mMax)
    {
        mDelayU[1] = mMax;
        mDelayU[0] = mMax;
        mDelayY[1] = mMax;
        mDelayY[0] = mMax;
        mValue     = mMax;
    }
    else if (mValue < mMin)
    {
        mDelayU[1] = mMin;
        mDelayU[0] = mMin;
        mDelayY[1] = mMin;
        mDelayY[0] = mMin;
        mValue     = mMin;
    }
    else
    {
        mDelayU[1] = mDelayU[0];
        mDelayU[0] = u;
        mDelayY[1] = mDelayY[0];
        mDelayY[0] = mValue;
    }

    return mValue;
}


//! Return current filter output value
//! @return The filtered actual value.
double SecondOrderTransferFunctionVariable::value()
{
    return mValue;
}

void SecondOrderTransferFunctionVariable::recalculateCoefficients()
{
    mCoeffU[0] = mNum[0]*(*mpTimeStep)*(*mpTimeStep) + 2.0*mNum[1]*(*mpTimeStep) + 4.0*mNum[2];
    mCoeffU[1] = 2.0*mNum[0]*(*mpTimeStep)*(*mpTimeStep) - 8.0*mNum[2];
    mCoeffU[2] = mNum[0]*(*mpTimeStep)*(*mpTimeStep) - 2.0*mNum[1]*(*mpTimeStep) + 4.0*mNum[2];

    mCoeffY[0] = mDen[0]*(*mpTimeStep)*(*mpTimeStep) + 2.0*mDen[1]*(*mpTimeStep) + 4.0*mDen[2];
    mCoeffY[1] = 2.0*mDen[0]*(*mpTimeStep)*(*mpTimeStep) - 8.0*mDen[2];
    mCoeffY[2] = mDen[0]*(*mpTimeStep)*(*mpTimeStep) - 2.0*mDen[1]*(*mpTimeStep) + 4.0*mDen[2];
}
