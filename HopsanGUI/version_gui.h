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
//! @file   version_gui.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains version definitions for the HopsanGUI, HMF files and component appearance files
//!
//$Id$
#ifndef VERSION_GUI_H
#define VERSION_GUI_H

// Include compiler info and hopsan core macros
#include "compiler_info.h"

// If we don't have the revision number then define UNKNOWN
// On real release  builds, UNKNOWN will be replaced by actual revnum by external script
#ifndef HOPSANGUISVNREVISION
 #define HOPSANGUISVNREVISION UNKNOWN
#endif

#define HOPSANGUIVERSION "0.7.x_r" TO_STR(HOPSANGUISVNREVISION)
#define HMF_VERSIONNUM "0.4"
#define HMF_REQUIREDVERSIONNUM "0.3"
#define CAF_VERSIONNUM "0.3"



#endif // VERSION_GUI_H
