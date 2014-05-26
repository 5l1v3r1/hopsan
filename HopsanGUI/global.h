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
//! @file   common.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains HopsanGUI global pointers and objects
//!
//$Id$

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QSplashScreen>

// Forward declaration of global objects
// Note! If you want to use one of these pointers the actual header for that class need to be included in your cpp file as well
class QWidget;
class MainWindow;
class Configuration;
class DesktopHandler;
class CopyStack;
class PlotHandler;
class LibraryWidget;
class TerminalWidget;
class ModelHandler;
class PlotWidget;
class CentralTabWidget;
class SystemParametersWidget;
class UndoWidget;
class LibraryHandler;
class GUIMessageHandler;
class HelpPopUpWidget;
class SensitivityAnalysisDialog;
class HelpDialog;
class PythonTerminalWidget;
class OptimizationDialog;
class QAction;
class OptionsDialog;
class QGridLayout;
class FindWidget;
class ModelicaLibrary;

// Global pointer to the main window and QWidget cast version
extern MainWindow* gpMainWindow;
extern QWidget *gpMainWindowWidget;

// Global object pointers in main
extern Configuration *gpConfig;
extern DesktopHandler *gpDesktopHandler;
extern CopyStack *gpCopyStack;
extern QSplashScreen *gpSplash;
extern GUIMessageHandler *gpMessageHandler;

// Global object pointers that are children to main window
extern PlotHandler *gpPlotHandler;
extern LibraryWidget *gpLibraryWidget;
extern TerminalWidget *gpTerminalWidget;
extern ModelHandler *gpModelHandler;
extern PlotWidget *gpPlotWidget;
extern CentralTabWidget *gpCentralTabWidget;
extern SystemParametersWidget *gpSystemParametersWidget;
extern UndoWidget *gpUndoWidget;
extern LibraryHandler *gpLibraryHandler;
extern HelpPopUpWidget *gpHelpPopupWidget;
extern SensitivityAnalysisDialog *gpSensitivityAnalysisDialog;
extern HelpDialog *gpHelpDialog;
extern PythonTerminalWidget *gpPythonTerminalWidget;
extern OptimizationDialog *gpOptimizationDialog;
extern OptionsDialog *gpOptionsDialog;
extern QGridLayout *gpCentralGridLayout;
extern FindWidget *gpFindWidget;
extern ModelicaLibrary *gpModelicaLibrary;

//Global actions
extern QAction *gpTogglePortsAction;
extern QAction *gpToggleNamesAction;

// Global variables
extern QString gHopsanCoreVersion;

#endif // GLOBAL_H
