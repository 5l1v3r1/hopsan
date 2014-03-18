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
//! @file   WhiteGaussianNoise.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-06-09
//!
//! @brief Contains a white gaussian noise generator
//!
//$Id$

#include "ComponentUtilities/WhiteGaussianNoise.h"
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <cmath>


using namespace hopsan;

double WhiteGaussianNoise::getValue()
{
    // Calc Gaussian random value
     double random1 = 0;
     while(random1 == 0)
     {
        random1 = (double)rand() / (double)RAND_MAX;
     }
     double random2 = (double)rand() / (double)RAND_MAX;
     return sqrt((-1.0)*log(random1))*cos(M_PI*random2);
}
