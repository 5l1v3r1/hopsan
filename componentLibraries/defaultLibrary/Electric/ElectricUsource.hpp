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

#ifndef ELECTRICUSOURCE_HPP_INCLUDED
#define ELECTRICUSOURCE_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file ElectricUsource.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Mon 7 Apr 2014 13:06:54
//! @brief An inertia load with spring and damper
//! @ingroup ElectricComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, ComponentLibraries, defaultLibrary, \
Electric}/ElectricUsource.nb*/

using namespace hopsan;

class ElectricUsource : public ComponentC
{
private:
     Port *mpPel1;
     int mNstep;
     //Port Pel1 variable
     double uel1;
     double iel1;
     double cel1;
     double Zcel1;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double uin;
     //outputVariables
     //Expressions variables
     //Port Pel1 pointer
     double *mpND_uel1;
     double *mpND_iel1;
     double *mpND_cel1;
     double *mpND_Zcel1;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpuin;
     //inputParameters pointers
     //outputVariables pointers
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new ElectricUsource();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        mpPel1=addPowerPort("Pel1","NodeElectric");
        //Add inputVariables to the component
            addInputVariable("uin","Voltage","V",12.,&mpuin);

        //Add inputParammeters to the component
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pel1
        mpND_uel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Voltage);
        mpND_iel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Current);
        mpND_cel1=getSafeNodeDataPtr(mpPel1, NodeElectric::WaveVariable);
        mpND_Zcel1=getSafeNodeDataPtr(mpPel1, NodeElectric::CharImpedance);

        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpND_uel1);
        iel1 = (*mpND_iel1);
        cel1 = (*mpND_cel1);
        Zcel1 = (*mpND_Zcel1);

        //Read inputVariables from nodes
        uin = (*mpuin);

        //Read inputParameters from nodes

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==


        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpND_uel1);
        iel1 = (*mpND_iel1);

        //Read inputVariables from nodes
        uin = (*mpuin);

        //LocalExpressions

          //Expressions
          cel1 = uin;
          Zcel1 = 0.;

        //Calculate the delayed parts


        //Write new values to nodes
        //Port Pel1
        (*mpND_cel1)=cel1;
        (*mpND_Zcel1)=Zcel1;
        //outputVariables

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // ELECTRICUSOURCE_HPP_INCLUDED
