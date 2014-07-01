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
//! @file   EquationSystemSolver.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-02-04
//!
//! @brief Contains an equation system solver utility
//!
//$Id$

#ifndef EQUATIONSYSTEMSOLVER_H
#define EQUATIONSYSTEMSOLVER_H

#include "Component.h"
#include "matrix.h"
#include "ludcmp.h"

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT EquationSystemSolver
{
public:

    EquationSystemSolver(Component *pParentComponent, int n);
    EquationSystemSolver(Component *pParentComponent, int n, Matrix *pJacobian, Vec *pEquations, Vec *pVariables);
    void solve(Matrix &jacobian, Vec &equations, Vec &variables, int iteration);
    void solve(Matrix &jacobian, Vec &equations, Vec &variables);
    void solve();

private:
    Component *mpParentComponent;
    double mSystemEquationWeight[4];
    int *mpOrder;
    Vec *mpDeltaStateVar;
    int mnVars;
    bool mSingular;

    Matrix *mpJacobian;
    Vec *mpEquations;
    Vec *mpVariables;
};


//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT NumericalIntegrationSolver
{
public:
    typedef double (NumericalIntegrationSolver::*callback_function)(int);

    NumericalIntegrationSolver(Component *pParentComponent, std::vector<double> *pStateVars, double tolerance=1e-6, size_t maxIter=1000);
    static const std::vector<HString> getAvailableSolverTypes();

    void solve(int solver);
    void solveForwardEuler();
    void solveMidpointMethod();
    void solveBackwardEuler();
    void solveTrapezoidRule();
    void solveRungeKutta();
    void solveDormandPrince();

    void solvevariableTimeStep();

    double findRoot(int i);

private:
    Component *mpParentComponent;

    //Numerical integration members
    double mTimeStep;
    std::vector<double> *mpStateVars;
    int mnStateVars;

    //Implicit methods members
    double mTolerance;
    size_t mMaxIter;
};


}

#endif // EQUATIONSYSTEMSOLVER_H
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
//! @file   EquationSystemSolver.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-02-04
//!
//! @brief Contains an equation system solver utility
//!
//$Id$

#ifndef EQUATIONSYSTEMSOLVER_H
#define EQUATIONSYSTEMSOLVER_H

#include "Component.h"
#include "matrix.h"
#include "ludcmp.h"

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT EquationSystemSolver
{
public:
    EquationSystemSolver(Component *pParentComponent);
    void solve(Matrix &jacobian, Vec &equations, Vec &variables, int iteration);

private:
    Component *mpParentComponent;
    double mSystemEquationWeight[4];
};

}

#endif // EQUATIONSYSTEMSOLVER_H
