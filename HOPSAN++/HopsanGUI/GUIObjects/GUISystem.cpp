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
//! @file   GUISystem.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI System class, representing system components
//!
//$Id$

#include "global.h"
#include "GUISystem.h"
#include "GraphicsView.h"
#include "CoreAccess.h"
#include "loadFunctions.h"
#include "MainWindow.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "version_gui.h"
#include "LibraryHandler.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "Dialogs/ContainerPropertiesDialog.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIWidgets.h"
#include "Widgets/PyDockWidget.h"
#include "Configuration.h"
#include "GUIContainerObject.h"

SystemContainer::SystemContainer(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : ContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    this->mpModelWidget = pParentContainer->mpModelWidget;
    this->commonConstructorCode();
}

//Root system specific constructor
SystemContainer::SystemContainer(ModelWidget *parentModelWidget, QGraphicsItem *pParent)
    : ContainerObject(QPointF(0,0), 0, 0, Deselected, UserGraphics, 0, pParent)
{
    this->mModelObjectAppearance = *(gpLibraryHandler->getModelObjectAppearancePtr(HOPSANGUISYSTEMTYPENAME)); //This will crash if Subsystem not already loaded
    this->mpModelWidget = parentModelWidget;
    this->commonConstructorCode();
    this->mpUndoStack->newPost();
    this->mSaveUndoStack = false;       //Do not save undo stack by default
}

void SystemContainer::deleteInHopsanCore()
{
    this->setUndoEnabled(false, true); //The last true means DONT ASK
    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,GUISystem destructor";
    //First remove all contents
    this->clearContents();

    if (mpParentContainerObject != 0)
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->removeSubComponent(this->getName(), true);
    }
    else
    {
        mpCoreSystemAccess->deleteRootSystemPtr();
    }

    delete mpCoreSystemAccess;
}

//! @brief This code is common among the two constructors, we use one function to avoid code duplication
void SystemContainer::commonConstructorCode()
{
        //Set the hmf save tag name
    mHmfTagName = HMF_SYSTEMTAG;

        //Set default values
    mLoadType = "EMBEDED";
    mNumberOfLogSamples = 2048;
    mLogStartTime = 0;

        //Create the object in core, and update name
    if (this->mpParentContainerObject == 0)
    {
        //Create root system
        qDebug() << "creating ROOT access system";
        mpCoreSystemAccess = new CoreSystemAccess();
        this->setName("RootSystem");
        //qDebug() << "the core root system name: " << mpCoreSystemAccess->getRootSystemName();
    }
    else
    {
        //Create subsystem
        qDebug() << "creating subsystem and setting name in " << mpParentContainerObject->getCoreSystemAccessPtr()->getSystemName();
        mName = mpParentContainerObject->getCoreSystemAccessPtr()->createSubSystem(this->getName());
        refreshDisplayName();
        qDebug() << "creating CoreSystemAccess for this subsystem, name: " << this->getName() << " parentname: " << mpParentContainerObject->getName();
        mpCoreSystemAccess = new CoreSystemAccess(this->getName(), mpParentContainerObject->getCoreSystemAccessPtr());
    }

    refreshDisplayName(); //Make sure name window is correct size for center positioning

    if(mpParentContainerObject)
    {
        connect(mpParentContainerObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisibleIfSignal(bool)));
    }
}


//!
//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
//!
void SystemContainer::setName(QString newName)
{
    if (mpParentContainerObject == 0)
    {
        mName = mpCoreSystemAccess->setSystemName(newName);
    }
    else
    {
        mpParentContainerObject->renameModelObject(this->getName(), newName);
    }
    refreshDisplayName();
}


//! Returns a string with the sub system type.
QString SystemContainer::getTypeName() const
{
    //! @todo is this OK should really ask the subsystem but result should be subsystem i think
    return HOPSANGUISYSTEMTYPENAME;
}

//! @brief Get the system cqs type
//! @returns A string containing the CQS type
QString SystemContainer::getTypeCQS()
{
    return mpCoreSystemAccess->getSystemTypeCQS();
}

//! @brief get The parameter names of this system
//! @returns A QStringList containing the parameter names
QStringList SystemContainer::getParameterNames()
{
    return mpCoreSystemAccess->getSystemParameterNames();
}

//! @brief Get a vector contain data from all parameters
//! @param [out] rParameterDataVec A vector that will contain parameter data
void SystemContainer::getParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    mpCoreSystemAccess->getSystemParameters(rParameterDataVec);
}

//! @brief Function that returns the specified parameter value
//! @param name Name of the parameter to return value from
QString SystemContainer::getParameterValue(const QString paramName)
{
    return mpCoreSystemAccess->getSystemParameterValue(paramName);
}

//! @brief Get parameter data for a specific parameter
//! @param [out] rData The parameter data
void SystemContainer::getParameter(const QString paramName, CoreParameterData &rData)
{
    return mpCoreSystemAccess->getSystemParameter(paramName, rData);
}

//! @brief Get a pointer the the CoreSystemAccess object that this system is representing
CoreSystemAccess* SystemContainer::getCoreSystemAccessPtr()
{
    return mpCoreSystemAccess;
}

//! @brief Overloaded version that returns self if root system
ContainerObject *SystemContainer::getParentContainerObject()
{
    if (mpParentContainerObject==0)
    {
        return this;
    }
    else
    {
        return mpParentContainerObject;
    }
}


int SystemContainer::type() const
{
    return Type;
}


//! @brief Opens the GUISystem properties dialog
void SystemContainer::openPropertiesDialog()
{
    //! @todo shouldnt this be in the containerproperties class, right now groups are not working thats is why it is here, the containerproperties dialog only works with systems for now
    ContainerPropertiesDialog dialog(this, gpMainWindowWidget);
    dialog.setAttribute(Qt::WA_DeleteOnClose, false);
    dialog.exec();
}


//! @brief Saves the System specific coredata to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
void SystemContainer::saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    ModelObject::saveCoreDataToDomElement(rDomElement);

    if (mLoadType == "EXTERNAL" && contents == FullModel)
    {
        //This information should ONLY be used to indicate that a system is external, it SHOULD NOT be included in the actual external system
        //If it would be, the load function will fail
        rDomElement.setAttribute( HMF_EXTERNALPATHTAG, relativePath(mModelFileInfo.absoluteFilePath(), mpParentContainerObject->getModelFileInfo().absolutePath()) );
    }

    if (mLoadType != "EXTERNAL" && contents == FullModel)
    {
        appendSimulationTimeTag(rDomElement, mpModelWidget->getStartTime().toDouble(), this->getTimeStep(), mpModelWidget->getStopTime().toDouble(), this->doesInheritTimeStep());
        appendLogSettingsTag(rDomElement, getLogStartTime(), getNumberOfLogSamples());
    }

    //Save the parameter values for the system
    // In case of external system save those that have been changed
    //! @todo right now we save all of them, but I think this is good even if they have not changed
    QVector<CoreParameterData> paramDataVector;
    this->getParameters(paramDataVector);
    QDomElement xmlParameters = appendDomElement(rDomElement, HMF_PARAMETERS);
    for(int i=0; i<paramDataVector.size(); ++i)
    {
        QDomElement xmlParameter = appendDomElement(xmlParameters, HMF_PARAMETERTAG);
        xmlParameter.setAttribute(HMF_NAMETAG, paramDataVector[i].mName);
        xmlParameter.setAttribute(HMF_VALUETAG, paramDataVector[i].mValue);
        xmlParameter.setAttribute(HMF_TYPE, paramDataVector[i].mType);
        xmlParameter.setAttribute(HMF_UNIT, paramDataVector[i].mUnit);
    }

    // Save the alias names in this system
    QDomElement xmlAliases = appendDomElement(rDomElement, HMF_ALIASES);
    QStringList aliases = getAliasNames();
    //! @todo need one function that gets both alias anf full maybe
    for (int i=0; i<aliases.size(); ++i)
    {
        QDomElement alias = appendDomElement(xmlAliases, HMF_ALIAS);
        alias.setAttribute(HMF_TYPE, "variable"); //!< @todo not maunal type
        alias.setAttribute(HMF_NAMETAG, aliases[i]);
        QString fullName = getFullNameFromAlias(aliases[i]);
        appendDomTextNode(alias, "fullname",fullName );
    }
}

void SystemContainer::saveSensitivityAnalysisSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement XMLsens = appendDomElement(rDomElement, HMF_SENSITIVITYANALYSIS);
    QDomElement XMLsetting = appendDomElement(XMLsens, HMF_SETTINGS);
    appendDomIntegerNode(XMLsetting, HMF_ITERATIONS, mSensSettings.nIter);
    if(mSensSettings.distribution == SensitivityAnalysisSettings::UniformDistribution)
    {
        appendDomTextNode(XMLsetting, HMF_DISTRIBUTIONTYPE, HMF_UNIFORMDIST);
    }
    else if(mSensSettings.distribution == SensitivityAnalysisSettings::NormalDistribution)
    {
        appendDomTextNode(XMLsetting, HMF_DISTRIBUTIONTYPE, HMF_NORMALDIST);
    }

    //Parameters
    QDomElement XMLparameters = appendDomElement(XMLsens, HMF_PARAMETERS);
    for(int i = 0; i < mSensSettings.parameters.size(); ++i)
    {
        QDomElement XMLparameter = appendDomElement(XMLparameters, HMF_PARAMETERTAG);
        appendDomTextNode(XMLparameter, HMF_COMPONENTTAG, mSensSettings.parameters.at(i).compName);
        appendDomTextNode(XMLparameter, HMF_PARAMETERTAG, mSensSettings.parameters.at(i).parName);
        appendDomValueNode2(XMLparameter, HMF_MINMAX, mSensSettings.parameters.at(i).min, mSensSettings.parameters.at(i).max);
        appendDomValueNode(XMLparameter, HMF_AVERAGE, mSensSettings.parameters.at(i).aver);
        appendDomValueNode(XMLparameter, HMF_SIGMA, mSensSettings.parameters.at(i).sigma);
    }

    //Variables
    QDomElement XMLobjectives = appendDomElement(XMLsens, HMF_PLOTVARIABLES);
    for(int i = 0; i < mSensSettings.variables.size(); ++i)
    {
        QDomElement XMLobjective = appendDomElement(XMLobjectives, HMF_PLOTVARIABLE);
        appendDomTextNode(XMLobjective, HMF_COMPONENTTAG, mSensSettings.variables.at(i).compName);
        appendDomTextNode(XMLobjective, HMF_PORTTAG, mSensSettings.variables.at(i).portName);
        appendDomTextNode(XMLobjective, HMF_PLOTVARIABLE, mSensSettings.variables.at(i).varName);
    }
}


void SystemContainer::loadSensitivityAnalysisSettingsFromDomElement(QDomElement &rDomElement)
{
    qDebug() << rDomElement.toDocument().toString();

    QDomElement settingsElement = rDomElement.firstChildElement(HMF_SETTINGS);
    if(!settingsElement.isNull())
    {
        mSensSettings.nIter = parseDomIntegerNode(settingsElement.firstChildElement(HMF_ITERATIONS), mSensSettings.nIter);
        QDomElement distElement = settingsElement.firstChildElement(HMF_DISTRIBUTIONTYPE);
        if(!distElement.isNull() && distElement.text() == HMF_UNIFORMDIST)
        {
            mSensSettings.distribution = SensitivityAnalysisSettings::UniformDistribution;
        }
        else if(!distElement.isNull() && distElement.text() == HMF_NORMALDIST)
        {
            mSensSettings.distribution = SensitivityAnalysisSettings::NormalDistribution;
        }
    }

    QDomElement parametersElement = rDomElement.firstChildElement(HMF_PARAMETERS);
    if(!parametersElement.isNull())
    {
        QDomElement parameterElement =parametersElement.firstChildElement(HMF_PARAMETERTAG);
        while (!parameterElement.isNull())
        {
            SensitivityAnalysisParameter par;
            par.compName = parameterElement.firstChildElement(HMF_COMPONENTTAG).text();
            par.parName = parameterElement.firstChildElement(HMF_PARAMETERTAG).text();
            parseDomValueNode2(parameterElement.firstChildElement(HMF_MINMAX), par.min, par.max);
            par.aver = parseDomValueNode(parameterElement.firstChildElement(HMF_AVERAGE), 0);
            par.sigma = parseDomValueNode(parameterElement.firstChildElement(HMF_SIGMA), 0);
            mSensSettings.parameters.append(par);

            parameterElement = parameterElement.nextSiblingElement(HMF_PARAMETERTAG);
        }
    }

    QDomElement variablesElement = rDomElement.firstChildElement(HMF_PLOTVARIABLES);
    if(!variablesElement.isNull())
    {
        QDomElement variableElement = variablesElement.firstChildElement(HMF_PLOTVARIABLE);
        while (!variableElement.isNull())
        {
            SensitivityAnalysisVariable var;

            var.compName = variableElement.firstChildElement(HMF_COMPONENTTAG).text();
            var.portName = variableElement.firstChildElement(HMF_PORTTAG).text();
            var.varName = variableElement.firstChildElement(HMF_PLOTVARIABLE).text();
            mSensSettings.variables.append(var);

            variableElement = variableElement.nextSiblingElement((HMF_PLOTVARIABLE));
        }
    }
}


void SystemContainer::saveOptimizationSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement XMLopt = appendDomElement(rDomElement, HMF_OPTIMIZATION);
    QDomElement XMLsetting = appendDomElement(XMLopt, HMF_SETTINGS);
    appendDomIntegerNode(XMLsetting, HMF_ITERATIONS, mOptSettings.mNiter);
    appendDomIntegerNode(XMLsetting, HMF_SEARCHPOINTS, mOptSettings.mNsearchp);
    appendDomValueNode(XMLsetting, HMF_REFLCOEFF, mOptSettings.mRefcoeff);
    appendDomValueNode(XMLsetting, HMF_RANDOMFACTOR, mOptSettings.mRandfac);
    appendDomValueNode(XMLsetting, HMF_FORGETTINGFACTOR, mOptSettings.mForgfac);
    appendDomValueNode(XMLsetting, HMF_FUNCTOL, mOptSettings.mFunctol);
    appendDomValueNode(XMLsetting, HMF_PARTOL, mOptSettings.mPartol);
    appendDomBooleanNode(XMLsetting, HMF_PLOT, mOptSettings.mPlot);
    appendDomBooleanNode(XMLsetting, HMF_SAVECSV, mOptSettings.mSavecsv);

    //Parameters
    appendDomBooleanNode(XMLsetting, HMF_LOGPAR, mOptSettings.mlogPar);
    QDomElement XMLparameters = appendDomElement(XMLopt, HMF_PARAMETERS);
    for(int i = 0; i < mOptSettings.mParamters.size(); ++i)
    {
        QDomElement XMLparameter = appendDomElement(XMLparameters, HMF_PARAMETERTAG);
        appendDomTextNode(XMLparameter, HMF_COMPONENTTAG, mOptSettings.mParamters.at(i).mComponentName);
        appendDomTextNode(XMLparameter, HMF_PARAMETERTAG, mOptSettings.mParamters.at(i).mParameterName);
        appendDomValueNode2(XMLparameter, HMF_MINMAX, mOptSettings.mParamters.at(i).mMin, mOptSettings.mParamters.at(i).mMax);
    }

    //Objective Functions
    QDomElement XMLobjectives = appendDomElement(XMLopt, HMF_OBJECTIVES);
    for(int i = 0; i < mOptSettings.mObjectives.size(); ++i)
    {
        QDomElement XMLobjective = appendDomElement(XMLobjectives, HMF_OBJECTIVE);
        appendDomTextNode(XMLobjective, HMF_FUNCNAME, mOptSettings.mObjectives.at(i).mFunctionName);
        appendDomValueNode(XMLobjective, HMF_WEIGHT, mOptSettings.mObjectives.at(i).mWeight);
        appendDomValueNode(XMLobjective, HMF_NORM, mOptSettings.mObjectives.at(i).mNorm);
        appendDomValueNode(XMLobjective, HMF_EXP, mOptSettings.mObjectives.at(i).mExp);

        QDomElement XMLObjectiveVariables = appendDomElement(XMLobjective, HMF_PLOTVARIABLES);
        if(!(mOptSettings.mObjectives.at(i).mVariableInfo.isEmpty()))
        {
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mVariableInfo.size(); ++j)
            {
                QDomElement XMLObjectiveVariable = appendDomElement(XMLObjectiveVariables, HMF_PLOTVARIABLE);
                appendDomTextNode(XMLObjectiveVariable, HMF_COMPONENTTAG, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(0));
                appendDomTextNode(XMLObjectiveVariable, HMF_PORTTAG, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(1));
                appendDomTextNode(XMLObjectiveVariable, HMF_PLOTVARIABLE, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(2));
            }
        }


        if(!(mOptSettings.mObjectives.at(i).mData.isEmpty()))
        {
            QDomElement XMLdata = appendDomElement(XMLobjective, HMF_DATA);
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mData.size(); ++j)
            {
                appendDomTextNode(XMLdata, HMF_PARAMETERTAG, mOptSettings.mObjectives.at(i).mData.at(j));
            }
        }
    }
}


void SystemContainer::loadOptimizationSettingsFromDomElement(QDomElement &rDomElement)
{
    qDebug() << rDomElement.toDocument().toString();

    QDomElement settingsElement = rDomElement.firstChildElement(HMF_SETTINGS);
    if(!settingsElement.isNull())
    {
        mOptSettings.mNiter = parseDomIntegerNode(settingsElement.firstChildElement(HMF_ITERATIONS), mOptSettings.mNiter);
        mOptSettings.mNsearchp = parseDomIntegerNode(settingsElement.firstChildElement(HMF_SEARCHPOINTS), mOptSettings.mNsearchp);
        mOptSettings.mRefcoeff = parseDomValueNode(settingsElement.firstChildElement(HMF_REFLCOEFF), mOptSettings.mRefcoeff);
        mOptSettings.mRandfac = parseDomValueNode(settingsElement.firstChildElement(HMF_RANDOMFACTOR), mOptSettings.mRandfac);
        mOptSettings.mForgfac = parseDomValueNode(settingsElement.firstChildElement(HMF_FORGETTINGFACTOR), mOptSettings.mForgfac);
        mOptSettings.mFunctol = parseDomValueNode(settingsElement.firstChildElement(HMF_FUNCTOL), mOptSettings.mFunctol);
        mOptSettings.mPartol = parseDomValueNode(settingsElement.firstChildElement(HMF_PARTOL), mOptSettings.mPartol);
        mOptSettings.mPlot = parseDomBooleanNode(settingsElement.firstChildElement(HMF_PLOT), mOptSettings.mPlot);
        mOptSettings.mSavecsv = parseDomBooleanNode(settingsElement.firstChildElement(HMF_SAVECSV), mOptSettings.mSavecsv);
        mOptSettings.mlogPar = parseDomBooleanNode(settingsElement.firstChildElement(HMF_LOGPAR), mOptSettings.mlogPar);
    }

    QDomElement parametersElement = rDomElement.firstChildElement(HMF_PARAMETERS);
    if(!parametersElement.isNull())
    {
        QDomElement parameterElement = parametersElement.firstChildElement(HMF_PARAMETERTAG);
        while (!parameterElement.isNull())
        {
            OptParameter parameter;
            parameter.mComponentName = parameterElement.firstChildElement(HMF_COMPONENTTAG).text();
            parameter.mParameterName = parameterElement.firstChildElement(HMF_PARAMETERTAG).text();
            parseDomValueNode2(parameterElement.firstChildElement(HMF_MINMAX), parameter.mMin, parameter.mMax);
            mOptSettings.mParamters.append(parameter);

            parameterElement = parameterElement.nextSiblingElement(HMF_PARAMETERTAG);
        }

        mOptSettings.mSavecsv = parseDomBooleanNode(settingsElement.firstChildElement(HMF_SAVECSV), mOptSettings.mSavecsv);
    }

    QDomElement objectivesElement = rDomElement.firstChildElement(HMF_OBJECTIVES);
    if(!objectivesElement.isNull())
    {
        QDomElement objElement = objectivesElement.firstChildElement(HMF_OBJECTIVE);
        while (!objElement.isNull())
        {
            Objectives objectives;

            objectives.mFunctionName = objElement.firstChildElement(HMF_FUNCNAME).text();
            objectives.mWeight = objElement.firstChildElement(HMF_WEIGHT).text().toDouble();
            objectives.mNorm = objElement.firstChildElement(HMF_NORM).text().toDouble();
            objectives.mExp = objElement.firstChildElement(HMF_EXP).text().toDouble();

            QDomElement variablesElement = objElement.firstChildElement(HMF_PLOTVARIABLES);
            if(!variablesElement.isNull())
            {
                QDomElement varElement = variablesElement.firstChildElement(HMF_PLOTVARIABLE);
                while (!varElement.isNull())
                {
                    QStringList variableInfo;

                    variableInfo.append(varElement.firstChildElement(HMF_COMPONENTTAG).text());
                    variableInfo.append(varElement.firstChildElement(HMF_PORTTAG).text());
                    variableInfo.append(varElement.firstChildElement(HMF_PLOTVARIABLE).text());

                    objectives.mVariableInfo.append(variableInfo);

                    varElement = varElement.nextSiblingElement(HMF_PLOTVARIABLE);
                }
            }

            QDomElement dataElement = objElement.firstChildElement(HMF_DATA);
            if(!dataElement.isNull())
            {
                QDomElement parElement = dataElement.firstChildElement(HMF_PARAMETERTAG);
                while (!parElement.isNull())
                {
                    objectives.mData.append(parElement.text());

                    parElement = parElement.nextSiblingElement(HMF_PARAMETERTAG);
                }
            }

            objElement = objElement.nextSiblingElement(HMF_OBJECTIVE);

            mOptSettings.mObjectives.append(objectives);
        }
    }
}


void SystemContainer::getSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings)
{
    sensSettings = mSensSettings;
}


void SystemContainer::setSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings)
{
    mSensSettings = sensSettings;
}


void SystemContainer::getOptimizationSettings(OptimizationSettings &optSettings)
{
    optSettings = mOptSettings;
}


void SystemContainer::setOptimizationSettings(OptimizationSettings &optSettings)
{
    mOptSettings = optSettings;
}


//! @brief Saves the System specific GUI data to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
QDomElement SystemContainer::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    QDomElement guiStuff = ModelObject::saveGuiDataToDomElement(rDomElement);

    //Should we try to append appearancedata stuff, we dont want this in external systems as they contain their own appearance
    if (mLoadType!="EXTERNAL")
    {
        //! @todo what happens if a subsystem (embeded) is asved, then we dont want to set the current graphics view
        if (this->mpModelWidget->getGraphicsView() != 0)
        {
            qreal x,y,zoom;
            this->mpModelWidget->getGraphicsView()->getViewPort(x,y,zoom);
            appendViewPortTag(guiStuff, x, y, zoom);
        }
        QDomElement portsHiddenElement = appendDomElement(guiStuff, HMF_PORTSTAG);
        portsHiddenElement.setAttribute("hidden", !mShowSubComponentPorts);
        QDomElement namesHiddenElement = appendDomElement(guiStuff, HMF_NAMESTAG);
        namesHiddenElement.setAttribute("hidden", !mShowSubComponentNames);

        QString gfxType = "iso";
        if(mGfxType == UserGraphics)
            gfxType = "user";
        QDomElement gfxTypeElement = appendDomElement(guiStuff, HMF_GFXTAG);
        gfxTypeElement.setAttribute("type", gfxType);

        QDomElement scriptFileElement = appendDomElement(guiStuff, HMF_SCRIPTFILETAG);
        scriptFileElement.setAttribute("path", mScriptFilePath);

        this->refreshExternalPortsAppearanceAndPosition();
        QDomElement xmlApp = appendOrGetCAFRootTag(guiStuff);

        //Before we save the modelobjectappearance data we need to set the correct basepath, (we ask our parent it will know)
        if (this->getParentContainerObject() != 0)
        {
            this->mModelObjectAppearance.setBasePath(this->getParentContainerObject()->getAppearanceData()->getBasePath());
        }
        this->mModelObjectAppearance.saveToDomElement(xmlApp);
    }

    saveOptimizationSettingsToDomElement(guiStuff);
    saveSensitivityAnalysisSettingsToDomElement(guiStuff);

    //Save undo stack if setting is activated
    if(mSaveUndoStack)
    {
        guiStuff.appendChild(mpUndoStack->toXml());
    }

    return guiStuff;
}

//! @brief Overloaded special XML DOM save function for System Objects
//! @param[in] rDomElement The DOM Element to save to
void SystemContainer::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    if(this == mpModelWidget->getTopLevelSystemContainer() && contents==FullModel)
    {
        //Append model info
        QString author, email, affiliation, description;
        getModelInfo(author, email, affiliation, description);
        QDomElement infoElement = appendDomElement(rDomElement, HMF_INFOTAG);
        appendDomTextNode(infoElement, HMF_AUTHORTAG, author);
        appendDomTextNode(infoElement, HMF_EMAILTAG, email);
        appendDomTextNode(infoElement, HMF_AFFILIATIONTAG, affiliation);
        appendDomTextNode(infoElement, HMF_DESCRIPTIONTAG, description);
    }

    //qDebug() << "Saving to dom node in: " << this->mModelObjectAppearance.getName();
    QDomElement xmlSubsystem = appendDomElement(rDomElement, mHmfTagName);

    //! @todo maybe use enums instead of strings
    //! @todo should not need to set this here
    if (mpParentContainerObject==0)
    {
        mLoadType = "ROOT"; //!< @todo this is a temporary hack for the xml save function (see bellow)
    }
    else if (!mModelFileInfo.filePath().isEmpty())
    {
        mLoadType = "EXTERNAL";
    }
    else
    {
        mLoadType = "EMBEDED";
    }

    // Save Core related stuff
    this->saveCoreDataToDomElement(xmlSubsystem, contents);
//    if(contents==FullModel)
//    {
//        xmlSubsystem.setAttribute(HMF_LOGSAMPLES, mNumberOfLogSamples);
//    }

    if(contents==FullModel)
    {
        // Save gui object stuff
        this->saveGuiDataToDomElement(xmlSubsystem);
    }

        //Save all of the sub objects
    if (mLoadType=="EMBEDED" || mLoadType=="ROOT")
    {
            //Save subcomponents and subsystems
        QDomElement xmlObjects = appendDomElement(xmlSubsystem, HMF_OBJECTS);
        ModelObjectMapT::iterator it;
        for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
        {
            it.value()->saveToDomElement(xmlObjects, contents);
        }

        if(contents==FullModel)
        {
                //Save all widgets
            QMap<size_t, Widget *>::iterator itw;
            for(itw = mWidgetMap.begin(); itw!=mWidgetMap.end(); ++itw)
            {
                itw.value()->saveToDomElement(xmlObjects);
            }

                //Save the connectors
            QDomElement xmlConnections = appendDomElement(xmlSubsystem, HMF_CONNECTIONS);
            for(int i=0; i<mSubConnectorList.size(); ++i)
            {
                mSubConnectorList[i]->saveToDomElement(xmlConnections);
            }
        }
    }
}

//! @brief Loads a System from an XML DOM Element
//! @param[in] rDomElement The element to load from
void SystemContainer::loadFromDomElement(QDomElement &rDomElement)
{
    // Loop back up to root level to get version numbers
    QString hmfFormatVersion = rDomElement.ownerDocument().firstChildElement(HMF_ROOTTAG).attribute(HMF_VERSIONTAG, "0");
    QString coreHmfVersion = rDomElement.ownerDocument().firstChildElement(HMF_ROOTTAG).attribute(HMF_HOPSANCOREVERSIONTAG, "0");

    // Load model info
    QDomElement infoElement = rDomElement.parentNode().firstChildElement(HMF_INFOTAG);
    if(!infoElement.isNull())
    {
        QString author, email, affiliation, description;
        QDomElement authorElement = infoElement.firstChildElement(HMF_AUTHORTAG);
        if(!authorElement.isNull())
        {
            author = authorElement.text();
        }
        QDomElement emailElement = infoElement.firstChildElement(HMF_EMAILTAG);
        if(!emailElement.isNull())
        {
            email = emailElement.text();
        }
        QDomElement affiliationElement = infoElement.firstChildElement(HMF_AFFILIATIONTAG);
        if(!affiliationElement.isNull())
        {
            affiliation = affiliationElement.text();
        }
        QDomElement descriptionElement = infoElement.firstChildElement(HMF_DESCRIPTIONTAG);
        if(!descriptionElement.isNull())
        {
            description = descriptionElement.text();
        }

        this->setModelInfo(author, email, affiliation, description);
    }

    //Check if the subsystem is external or internal, and load appropriately
    QString external_path = rDomElement.attribute(HMF_EXTERNALPATHTAG);
    if (external_path.isEmpty())
    {
        //Load embedded subsystem
        //0. Load core and gui stuff
        //! @todo might need some error checking here incase some fields are missing
        //Now load the core specific data, might need inherited function for this
        this->setName(rDomElement.attribute(HMF_NAMETAG));

        //Load the GUI stuff like appearance data and viewport
        QDomElement guiStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
        this->mModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(CAF_ROOT).firstChildElement(CAF_MODELOBJECT));
        this->refreshDisplayName(); // This must be done becouse in some occations the loadAppearanceDataline above will overwrite the correct name
        this->mShowSubComponentNames = !parseAttributeBool(guiStuff.firstChildElement(HMF_NAMESTAG),"hidden",true);
        this->mShowSubComponentPorts = !parseAttributeBool(guiStuff.firstChildElement(HMF_PORTSTAG),"hidden",true);
        QString gfxType = guiStuff.firstChildElement(HMF_GFXTAG).attribute("type");
        if(gfxType == "user") { mGfxType = UserGraphics; }
        else if(gfxType == "iso") { mGfxType = ISOGraphics; }
        //! @todo these two should not be set here
        gpMainWindow->mpToggleNamesAction->setChecked(mShowSubComponentNames);
        gpMainWindow->mpTogglePortsAction->setChecked(mShowSubComponentPorts);
        double x = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("x").toDouble();
        double y = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("y").toDouble();
        double zoom = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("zoom").toDouble();
        setScriptFile(guiStuff.firstChildElement(HMF_SCRIPTFILETAG).attribute("path"));

        bool dontClearUndo = false;
        if(!guiStuff.firstChildElement(HMF_UNDO).isNull())
        {
            QDomElement undoElement = guiStuff.firstChildElement(HMF_UNDO);
            mpUndoStack->fromXml(undoElement);
            dontClearUndo = true;
            mSaveUndoStack = true;      //Set save undo stack setting to true if loading a hmf file with undo stack saved
        }

        mpModelWidget->getGraphicsView()->setZoomFactor(zoom);

        qDebug() << "Center on " << x << ", " << y;
        mpModelWidget->getGraphicsView()->centerOn(x, y);
        //! @todo load viewport and pose and stuff

        //Load simulation time
        QString startT,stepT,stopT;
        bool inheritTs;
        parseSimulationTimeTag(rDomElement.firstChildElement(HMF_SIMULATIONTIMETAG), startT, stepT, stopT, inheritTs);
        this->setTimeStep(stepT.toDouble());
        mpCoreSystemAccess->setInheritTimeStep(inheritTs);

        // Load number of log samples
        parseLogSettingsTag(rDomElement.firstChildElement(HMF_SIMULATIONLOGSETTINGS), mLogStartTime, mNumberOfLogSamples);
        //! @deprecated 20131002 we keep this below for backwards compatibility for a while
        if(rDomElement.hasAttribute(HMF_LOGSAMPLES))
        {
            mNumberOfLogSamples = rDomElement.attribute(HMF_LOGSAMPLES).toInt();
        }

        // Only set start stop time for the top level system
        if (mpParentContainerObject == 0)
        {
            mpModelWidget->setTopLevelSimulationTime(startT,stepT,stopT);
        }

        //1. Load global parameters
        QDomElement xmlParameters = rDomElement.firstChildElement(HMF_PARAMETERS);
        QDomElement xmlSubObject = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
        while (!xmlSubObject.isNull())
        {
            loadSystemParameter(xmlSubObject, hmfFormatVersion, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_PARAMETERTAG);
        }

        //2. Load all sub-components
        QDomElement xmlSubObjects = rDomElement.firstChildElement(HMF_OBJECTS);
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_COMPONENTTAG);
        while (!xmlSubObject.isNull())
        {
            verifyHmfComponentCompatibility(xmlSubObject, hmfFormatVersion, coreHmfVersion);
            ModelObject* pObj = loadModelObject(xmlSubObject, this, NoUndo);
            if(pObj == NULL)
            {
                gpTerminalWidget->mpConsole->printErrorMessage(QString("Model contains component from a library that has not been loaded. TypeName: ") +
                                                                    xmlSubObject.attribute(HMF_TYPENAME) + QString(", Name: ") + xmlSubObject.attribute(HMF_NAMETAG));

                // Insert missing component dummy instead
                xmlSubObject.setAttribute(HMF_TYPENAME, "MissingComponent");
                pObj = loadModelObject(xmlSubObject, this, NoUndo);
            }
            else
            {



                //! @deprecated This StartValue load code is only kept for upconverting old files, we should keep it here until we have some other way of upconverting old formats
                //Load start values //Is not needed, start values are saved as ordinary parameters! This code snippet can probably be removed.
                QDomElement xmlStartValues = xmlSubObject.firstChildElement(HMF_STARTVALUES);
                QDomElement xmlStartValue = xmlStartValues.firstChildElement(HMF_STARTVALUE);
                while (!xmlStartValue.isNull())
                {
                    loadStartValue(xmlStartValue, pObj, NoUndo);
                    xmlStartValue = xmlStartValue.nextSiblingElement(HMF_STARTVALUE);
                }
            }

//            if(pObj && pObj->getTypeName().startsWith("CppComponent"))
//            {
//                recompileCppComponents(pObj);
//            }

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_COMPONENTTAG);
        }

        //3. Load all text box widgets
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_TEXTBOXWIDGETTAG);
        while (!xmlSubObject.isNull())
        {
            loadTextBoxWidget(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_TEXTBOXWIDGETTAG);
        }

        //5. Load all sub-systems
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMTAG);
        while (!xmlSubObject.isNull())
        {
            loadModelObject(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMTAG);
        }

        //6. Load all systemports
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMPORTTAG);
        while (!xmlSubObject.isNull())
        {
            loadContainerPortObject(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMPORTTAG);
        }

        //7. Load all connectors
        QDomElement xmlConnections = rDomElement.firstChildElement(HMF_CONNECTIONS);
        xmlSubObject = xmlConnections.firstChildElement(HMF_CONNECTORTAG);
        QList<QDomElement> failedConnections;
        while (!xmlSubObject.isNull())
        {
            if(!loadConnector(xmlSubObject, this, NoUndo))
            {
//                failedConnections.append(xmlSubObject);
            }
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_CONNECTORTAG);
        }
//        //If some connectors failed to load, it could mean that they were loaded in wrong order.
//        //Try again until they work, or abort if number of attempts are greater than maximum possible for success.
//        int stop=failedConnections.size()*(failedConnections.size()+1)/2;
//        int i=0;
//        while(!failedConnections.isEmpty())
//        {
//            if(!loadConnector(failedConnections.first(), this, NoUndo))
//            {
//                failedConnections.append(failedConnections.first());
//            }
//            failedConnections.removeFirst();
//            ++i;
//            if(i>stop) break;
//        }


        //8. Load favorite variables
//        QDomElement xmlFavVariables = guiStuff.firstChildElement(HMF_FAVORITEVARIABLES);
//        QDomElement xmlFavVariable = xmlFavVariables.firstChildElement(HMF_FAVORITEVARIABLETAG);
//        while (!xmlFavVariable.isNull())
//        {
//            loadFavoriteVariable(xmlFavVariable, this);
//            xmlFavVariable = xmlFavVariable.nextSiblingElement(HMF_FAVORITEVARIABLETAG);
//        }

        //9. Load plot variable aliases
        QDomElement xmlAliases = rDomElement.firstChildElement(HMF_ALIASES);
        QDomElement xmlAlias = xmlAliases.firstChildElement(HMF_ALIAS);
        while (!xmlAlias.isNull())
        {
            loadPlotAlias(xmlAlias, this);
            xmlAlias = xmlAlias.nextSiblingElement(HMF_ALIAS);
        }

        //9.1 Load plot variable aliases
        //! @deprecated Remove in teh future when hmf format stabilized and everyone has upgraded
        xmlSubObject = xmlParameters.firstChildElement(HMF_ALIAS);
        while (!xmlSubObject.isNull())
        {
            loadPlotAlias(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_ALIAS);
        }

        //10. Load optimization settings
        xmlSubObject = guiStuff.firstChildElement(HMF_OPTIMIZATION);
        loadOptimizationSettingsFromDomElement(xmlSubObject);

        //11. Load sensitivity analysis settings
        xmlSubObject = guiStuff.firstChildElement(HMF_SENSITIVITYANALYSIS);
        loadSensitivityAnalysisSettingsFromDomElement(xmlSubObject);

        //Refresh the appearance of the subsystemem and create the GUIPorts based on the loaded portappearance information
        //! @todo This is a bit strange, refreshAppearance MUST be run before create ports or create ports will not know some necessary stuff
        this->refreshAppearance();
        this->refreshExternalPortsAppearanceAndPosition();
        //this->createPorts();

        //Deselect all components
        this->deselectAll();
        if(!dontClearUndo)
        {
            this->mpUndoStack->clear();
        }
        //Only do this for the root system
        //! @todo maybe can do this for subsystems to (even if we dont see them right now)
        if (this->mpParentContainerObject == 0)
        {
            //mpParentModelWidget->getGraphicsView()->centerView();
            mpModelWidget->getGraphicsView()->updateViewPort();
        }
        this->mpModelWidget->setSaved(true);

#ifdef USEPYTHONQT
        gpMainWindow->getPythonDock()->runPyScript(mScriptFilePath);
#endif

        emit systemParametersChanged(); // Make sure we refresh the syspar widget
        emit checkMessages();
    }
    else
    {
        gpTerminalWidget->mpConsole->printWarningMessage("A system you tried to load is taged as an external system, but the ContainerSystem load function only loads embeded systems");
    }
}


void SystemContainer::exportToLabView()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(gpMainWindow, tr("Export to LabVIEW/SIT"),
                                  "This will create source code for a LabVIEW/SIT DLL-file from current model. You will need the HopsanCore source code and Visual Studio 2003 to compile it.\nContinue?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    //Open file dialog and initialize the file stream
    QString filePath;
    //QFileInfo fileInfo;
    //QFile file;
    filePath = QFileDialog::getSaveFileName(gpMainWindow, tr("Export Project to HopsanRT Wrapper Code"),
                                            gpConfig->getLabViewExportDir(),
                                            tr("C++ Source File (*.cpp)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QFileInfo file(filePath);
    gpConfig->setLabViewExportDir(file.absolutePath());

    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToLabViewSIT(filePath, this);
    delete(pCoreAccess);
}

void SystemContainer::exportToFMU()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Functional Mockup Unit"),
                                                    gpConfig->getFmuExportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QDir saveDir;
    saveDir.setPath(savePath);
    gpConfig->setFmuExportDir(saveDir.absolutePath());
    saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    if(!saveDir.entryList().isEmpty())
    {
        qDebug() << saveDir.entryList();
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
        msgBox.setText(QString("Folder is not empty!"));
        msgBox.setInformativeText("Are you sure you want to export files here?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        int answer = msgBox.exec();
        if(answer == QMessageBox::No)
        {
            return;
        }
    }

    exportToFMU(savePath, true);
}


void SystemContainer::exportToFMUCoSim()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Functional Mockup Unit"),
                                                    gpConfig->getFmuExportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QDir saveDir;
    saveDir.setPath(savePath);
    gpConfig->setFmuExportDir(saveDir.absolutePath());
    saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    if(!saveDir.entryList().isEmpty())
    {
        qDebug() << saveDir.entryList();
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
        msgBox.setText(QString("Folder is not empty!"));
        msgBox.setInformativeText("Are you sure you want to export files here?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        int answer = msgBox.exec();
        if(answer == QMessageBox::No)
        {
            return;
        }
    }

    exportToFMU(savePath, false);
}


void SystemContainer::exportToFMU(QString savePath, bool me)
{
    QDir saveDir(savePath);
    if(!saveDir.exists())
    {
        QDir().mkpath(savePath);
    }
    saveDir.setFilter(QDir::NoFilter);

    //Save model to hmf in export directory
    mpModelWidget->saveTo(savePath+"/"+mModelFileInfo.fileName().replace(" ", "_"));

    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToFmu(savePath, me, this);
    delete(pCoreAccess);

}

//void SystemContainer::exportToFMU()
//{
//    //Open file dialog and initialize the file stream
//    QDir fileDialogSaveDir;
//    QString savePath;
//    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Functional Mockup Unit"),
//                                                    gConfig.getFmuExportDir(),
//                                                    QFileDialog::ShowDirsOnly
//                                                    | QFileDialog::DontResolveSymlinks);
//    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

//    QDir saveDir;
//    saveDir.setPath(savePath);
//    gConfig.setFmuExportDir(saveDir.absolutePath());
//    saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
//    if(!saveDir.entryList().isEmpty())
//    {
//        qDebug() << saveDir.entryList();
//        QMessageBox msgBox;
//        msgBox.setWindowIcon(gpMainWindow->windowIcon());
//        msgBox.setText(QString("Folder is not empty!"));
//        msgBox.setInformativeText("Are you sure you want to export files here?");
//        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//        msgBox.setDefaultButton(QMessageBox::No);

//        int answer = msgBox.exec();
//        if(answer == QMessageBox::No)
//        {
//            return;
//        }
//    }
//    saveDir.setFilter(QDir::NoFilter);


//    //Tells if user selected the gcc compiler or not (= visual studio)
//    //bool gccCompiler = mpExportFmuGccRadioButton->isChecked();


//    //Write the FMU ID
//    int random = rand() % 1000;
//    QString randomString = QString().setNum(random);
//    QString ID = "{8c4e810f-3df3-4a00-8276-176fa3c9f"+randomString+"}";  //!< @todo How is this ID defined?


//    //Collect information about input ports
//    QStringList inputVariables;
//    QStringList inputComponents;
//    QStringList inputPorts;
//    QList<int> inputDatatypes;

//    ModelObjectMapT::iterator it;
//    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
//    {
//        if(it.value()->getTypeName() == "SignalInputInterface")
//        {
//            inputVariables.append(it.value()->getName().remove(' '));
//            inputComponents.append(it.value()->getName());
//            inputPorts.append("out");
//            inputDatatypes.append(0);
//        }
//    }


//    //Collect information about output ports
//    QStringList outputVariables;
//    QStringList outputComponents;
//    QStringList outputPorts;
//    QList<int> outputDatatypes;

//    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
//    {
//        if(it.value()->getTypeName() == "SignalOutputInterface")
//        {
//            outputVariables.append(it.value()->getName().remove(' '));
//            outputComponents.append(it.value()->getName());
//            outputPorts.append("in");
//            outputDatatypes.append(0);
//        }
//    }


//    //Create file objects for all files that shall be created
//    QFile modelSourceFile;
//    QString modelName = getModelFileInfo().fileName();
//    modelName.chop(4);
//    QString realModelName = modelName;          //Actual model name (used for hmf file)
//    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
//    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
//    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open " + modelName + ".c for writing.");
//        return;
//    }

//    QFile modelDescriptionFile;
//    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
//    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open modelDescription.xml for writing.");
//        return;
//    }

//    QFile fmuHeaderFile;
//    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
//    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open HopsanFMU.h for writing.");
//        return;
//    }

//    QFile fmuSourceFile;
//    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
//    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
//        return;
//    }

//#ifdef WIN32
//    QFile clBatchFile;
//    clBatchFile.setFileName(savePath + "/compile.bat");
//    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open compile.bat for writing.");
//        return;
//    }
//#endif

//    //progressBar.setLabelText("Writing modelDescription.xml");
//    //progressBar.setValue(1);

//    QTextStream modelDescriptionStream(&modelDescriptionFile);
//    modelDescriptionStream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";       //!< @todo Encoding, should it be UTF-8?
//    modelDescriptionStream << "<fmiModelDescription\n";
//    modelDescriptionStream << "  fmiVersion=\"1.0\"\n";
//    modelDescriptionStream << "  modelName=\"" << modelName << "\"\n";               //!< @todo What's the difference between name and identifier?
//    modelDescriptionStream << "  modelIdentifier=\"" << modelName << "\"\n";
//    modelDescriptionStream << "  guid=\"" << ID << "\"\n";
//    modelDescriptionStream << "  numberOfContinuousStates=\"" << inputVariables.size() + outputVariables.size() << "\"\n";
//    modelDescriptionStream << "  numberOfEventIndicators=\"0\">\n";
//    modelDescriptionStream << "<ModelVariables>\n";
//    int i, j;
//    for(i=0; i<inputVariables.size(); ++i)
//    {
//        QString refString = QString().setNum(i);
//        modelDescriptionStream << "  <ScalarVariable name=\"" << inputVariables.at(i) << "\" valueReference=\""+refString+"\" description=\"input variable\" causality=\"input\">\n";
//        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
//        modelDescriptionStream << "  </ScalarVariable>\n";
//    }
//    for(j=0; j<outputVariables.size(); ++j)
//    {
//        QString refString = QString().setNum(i+j);
//        modelDescriptionStream << "  <ScalarVariable name=\"" << outputVariables.at(j) << "\" valueReference=\""+refString+"\" description=\"output variable\" causality=\"output\">\n";
//        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
//        modelDescriptionStream << "  </ScalarVariable>\n";
//    }
//    modelDescriptionStream << "</ModelVariables>\n";
//    modelDescriptionStream << "</fmiModelDescription>\n";
//    modelDescriptionFile.close();


//    //progressBar.setLabelText("Writing " + modelName + ".c");
//    //progressBar.setValue(2);


//    QTextStream modelSourceStream(&modelSourceFile);
//    modelSourceStream << "// Define class name and unique id\n";
//    modelSourceStream << "    #define MODEL_IDENTIFIER " << modelName << "\n";
//    modelSourceStream << "    #define MODEL_GUID \"" << ID << "\"\n\n";
//    modelSourceStream << "    // Define model size\n";
//    modelSourceStream << "    #define NUMBER_OF_REALS " << inputVariables.size() + outputVariables.size() << "\n";
//    modelSourceStream << "    #define NUMBER_OF_INTEGERS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_BOOLEANS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_STRINGS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_STATES "<< inputVariables.size() + outputVariables.size() << "\n";        //!< @todo Does number of variables equal number of states?
//    modelSourceStream << "    #define NUMBER_OF_EVENT_INDICATORS 0\n\n";
//    modelSourceStream << "    // Include fmu header files, typedefs and macros\n";
//    modelSourceStream << "    #include \"fmuTemplate.h\"\n";
//    modelSourceStream << "    #include \"HopsanFMU.h\"\n\n";
//    modelSourceStream << "    // Define all model variables and their value references\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "    #define " << inputVariables.at(i) << "_ " << i << "\n\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "    #define " << outputVariables.at(j) << "_ " << j+i << "\n\n";
//    modelSourceStream << "    // Define state vector as vector of value references\n";
//    modelSourceStream << "    #define STATES { ";
//    i=0;
//    j=0;
//    if(!inputVariables.isEmpty())
//    {
//        modelSourceStream << inputVariables.at(0) << "_";
//        ++i;
//    }
//    else if(!outputVariables.isEmpty())
//    {
//        modelSourceStream << outputVariables.at(0) << "_";
//        ++j;
//    }
//    for(; i<inputVariables.size(); ++i)
//        modelSourceStream << ", " << inputVariables.at(i) << "_";
//    for(; j<outputVariables.size(); ++j)
//        modelSourceStream << ", " << outputVariables.at(j) << "_";
//    modelSourceStream << " }\n\n";
//    modelSourceStream << "    //Set start values\n";
//    modelSourceStream << "    void setStartValues(ModelInstance *comp) \n";
//    modelSourceStream << "    {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "        r(" << inputVariables.at(i) << "_) = 0;\n";        //!< Fix start value handling
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "        r(" << outputVariables.at(j) << "_) = 0;\n";        //!< Fix start value handling
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Initialize\n";
//    modelSourceStream << "    void initialize(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        initializeHopsanWrapper(\""+realModelName+".hmf\");\n";
//    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
//    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Return variable of real type\n";
//    modelSourceStream << "    fmiReal getReal(ModelInstance* comp, fmiValueReference vr)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        switch (vr) \n";
//    modelSourceStream << "       {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "           case " << inputVariables.at(i) << "_: return getVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ");\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "           case " << outputVariables.at(j) << "_: return getVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ");\n";
//    modelSourceStream << "            default: return 1;\n";
//    modelSourceStream << "       }\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    void setReal(ModelInstance* comp, fmiValueReference vr, fmiReal value)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        switch (vr) \n";
//    modelSourceStream << "       {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "           case " << inputVariables.at(i) << "_: setVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ", value);\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "           case " << outputVariables.at(j) << "_: setVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ", value);\n";
//    modelSourceStream << "            default: return;\n";
//    modelSourceStream << "       }\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Update at time event\n";
//    modelSourceStream << "    void eventUpdate(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        simulateOneStep();\n";
//    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
//    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";      //!< @todo Hardcoded timestep
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    // Include code that implements the FMI based on the above definitions\n";
//    modelSourceStream << "    #include \"fmuTemplate.c\"\n";
//    modelSourceFile.close();


//    //progressBar.setLabelText("Writing HopsanFMU.h");
//    //progressBar.setValue(4);


//    QTextStream fmuHeaderStream(&fmuHeaderFile);
//    QTextLineStream fmuHeaderLines(fmuHeaderStream);
//    fmuHeaderLines << "#ifndef HOPSANFMU_H";
//    fmuHeaderLines << "#define HOPSANFMU_H";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
//    //fmuHeaderLines << "    #define DLLEXPORT __declspec(dllexport)";
//    fmuHeaderLines << "    extern \"C\" {";
//    fmuHeaderLines << "#else";
//    fmuHeaderLines << "    #define DLLEXPORT";
//    fmuHeaderLines << "#endif";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "DLLEXPORT void initializeHopsanWrapper(char* filename);";
//    fmuHeaderLines << "DLLEXPORT void simulateOneStep();";
//    fmuHeaderLines << "DLLEXPORT double getVariable(char* component, char* port, size_t idx);";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "DLLEXPORT void setVariable(char* component, char* port, size_t idx, double value);";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
//    fmuHeaderLines << "}";
//    fmuHeaderLines << "#endif";
//    fmuHeaderLines << "#endif // HOPSANFMU_H";
//    fmuHeaderFile.close();


//    //progressBar.setLabelText("Writing HopsanFMU.cpp");
//    //progressBar.setValue(5);


//    QTextStream fmuSourceStream(&fmuSourceFile);
//    QTextLineStream fmuSrcLines(fmuSourceStream);

//    fmuSrcLines << "#include <iostream>";
//    fmuSrcLines << "#include <assert.h>";
//    fmuSrcLines << "#include \"HopsanCore.h\"";
//    fmuSrcLines << "#include \"HopsanFMU.h\"";
//    //fmuSrcLines << "#include \"include/ComponentEssentials.h\"";
//    //fmuSrcLines << "#include \"include/ComponentUtilities.h\"";
//    fmuSrcLines << "";
//    fmuSrcLines << "static double fmu_time=0;";
//    fmuSrcLines << "static hopsan::ComponentSystem *spCoreComponentSystem;";
//    fmuSrcLines << "static std::vector<std::string> sComponentNames;";
//    fmuSrcLines << "hopsan::HopsanEssentials gHopsanCore;";
//    fmuSrcLines << "";
//    fmuSrcLines << "void initializeHopsanWrapper(char* filename)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    double startT;      //Dummy variable";
//    fmuSrcLines << "    double stopT;       //Dummy variable";
//    fmuSrcLines << "    gHopsanCore.loadExternalComponentLib(\"../componentLibraries/defaultLibrary/components/libdefaultComponentLibrary.so\");";
//    fmuSrcLines << "    spCoreComponentSystem = gHopsanCore.loadHMFModel(filename, startT, stopT);\n";
//    fmuSrcLines << "    assert(spCoreComponentSystem);";
//    fmuSrcLines << "    spCoreComponentSystem->setDesiredTimestep(0.001);";           //!< @todo Time step should not be hard coded
//    fmuSrcLines << "    spCoreComponentSystem->initialize(0,10);";
//    fmuSrcLines << "";
//    fmuSrcLines << "    fmu_time = 0;";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "void simulateOneStep()";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    if(spCoreComponentSystem->checkModelBeforeSimulation())";
//    fmuSrcLines << "    {";
//    fmuSrcLines << "        double timestep = spCoreComponentSystem->getDesiredTimeStep();";
//    fmuSrcLines << "        spCoreComponentSystem->simulate(fmu_time, fmu_time+timestep);";
//    fmuSrcLines << "        fmu_time = fmu_time+timestep;\n";
//    fmuSrcLines << "    }";
//    fmuSrcLines << "    else";
//    fmuSrcLines << "    {";
//    fmuSrcLines << "        std::cout << \"Simulation failed!\";";
//    fmuSrcLines << "    }";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "double getVariable(char* component, char* port, size_t idx)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->readNode(idx);";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "void setVariable(char* component, char* port, size_t idx, double value)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    assert(spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port) != 0);";
//    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->writeNode(idx, value);";
//    fmuSrcLines << "}";
//    fmuSourceFile.close();

//#ifdef WIN32
//    //progressBar.setLabelText("Writing to compile.bat");
//    //progressBar.setValue(6);



//    //Write the compilation script file
//    QTextStream clBatchStream(&clBatchFile);
////    if(gccCompiler)
////    {
//        //! @todo Ship Mingw with Hopsan, or check if it exists in system and inform user if it does not.
//    clBatchStream << "g++ -DWRAPPERCOMPILATION -c -Wl,--rpath,'$ORIGIN/.' HopsanFMU.cpp -I./include\n";
//    clBatchStream << "g++ -shared -Wl,--rpath,'$ORIGIN/.' -o HopsanFMU.dll HopsanFMU.o -L./ -lHopsanCore";
////    }
////    else
////    {
////        //! @todo Check that Visual Studio is installed, and warn user if not
////        clBatchStream << "echo Compiling Visual Studio libraries...\n";
////        clBatchStream << "if defined VS90COMNTOOLS (call \"%VS90COMNTOOLS%\\vsvars32.bat\") else ^\n";
////        clBatchStream << "if defined VS80COMNTOOLS (call \"%VS80COMNTOOLS%\\vsvars32.bat\")\n";
////        clBatchStream << "cl -LD -nologo -DWIN32 -DWRAPPERCOMPILATION HopsanFMU.cpp /I \\. /I \\include\\HopsanCore.h HopsanCore.lib\n";
////    }
//    clBatchFile.close();
//#endif

//    //progressBar.setLabelText("Copying binary files");
//    //progressBar.setValue(7);


//    //Copy binaries to export directory
//#ifdef WIN32
//    QFile dllFile;
//    QFile libFile;
//    QFile expFile;
////    if(gccCompiler)
////    {
//        dllFile.setFileName(gDesktopHandler.getExecPath() + "HopsanCore.dll");
//        dllFile.copy(savePath + "/HopsanCore.dll");
////    }
////    else
////    {
////        //! @todo this seem a bit hardcoded
////        dllFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.dll");
////        dllFile.copy(savePath + "/HopsanCore.dll");
////        libFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.lib");
////        libFile.copy(savePath + "/HopsanCore.lib");
////        expFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.exp");
////        expFile.copy(savePath + "/HopsanCore.exp");
////    }
//#elif linux
//    QFile soFile;
//    soFile.setFileName(gDesktopHandler.getExecPath() + "libHopsanCore.so");
//    soFile.copy(savePath + "/libHopsanCore.so");
//#endif


//    //progressBar.setLabelText("Copying include files");
//    //progressBar.setValue(8);


//    //Copy include files to export directory
//    copyIncludeFilesToDir(savePath);


//    progressBar.setLabelText("Writing "+realModelName+".hmf");
//    progressBar.setValue(9);


//    //Save model to hmf in export directory
//    //! @todo This code is duplicated from ModelWidget::saveModel(), make it a common function somehow
//    QDomDocument domDocument;
//    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());
//    saveToDomElement(hmfRoot);
//    const int IndentSize = 4;
//    QFile xmlhmf(savePath + "/" + mModelFileInfo.fileName());
//    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Unable to open "+savePath+"/"+mModelFileInfo.fileName()+" for writing.");
//        return;
//    }
//    QTextStream out(&xmlhmf);
//    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
//    domDocument.save(out, IndentSize);

//    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
//    pCoreAccess->generateToFmu(savePath, this);
//    delete(pCoreAccess);

//}


void SystemContainer::exportToSimulink()
{
    QDialog *pExportDialog = new QDialog(gpMainWindow);
    pExportDialog->setWindowTitle("Create Simulink Source Files");

    QLabel *pExportDialogLabel1 = new QLabel(tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console."), pExportDialog);
    pExportDialogLabel1->setWordWrap(true);

    QGroupBox *pCompilerGroupBox = new QGroupBox(tr("Choose compiler:"), pExportDialog);
    QRadioButton *pMSVC2008RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2008"));
    QRadioButton *pMSVC2010RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2010"));
    pMSVC2008RadioButton->setChecked(true);
    QVBoxLayout *pCompilerLayout = new QVBoxLayout;
    pCompilerLayout->addWidget(pMSVC2008RadioButton);
    pCompilerLayout->addWidget(pMSVC2010RadioButton);
    pCompilerLayout->addStretch(1);
    pCompilerGroupBox->setLayout(pCompilerLayout);

    QGroupBox *pArchitectureGroupBox = new QGroupBox(tr("Choose architecture:"), pExportDialog);
    QRadioButton *p32bitRadioButton = new QRadioButton(tr("32-bit (x86)"));
    QRadioButton *p64bitRadioButton = new QRadioButton(tr("64-bit (x64)"));
    p32bitRadioButton->setChecked(true);
    QVBoxLayout *pArchitectureLayout = new QVBoxLayout;
    pArchitectureLayout->addWidget(p32bitRadioButton);
    pArchitectureLayout->addWidget(p64bitRadioButton);
    pArchitectureLayout->addStretch(1);
    pArchitectureGroupBox->setLayout(pArchitectureLayout);

    QLabel *pExportDialogLabel2 = new QLabel("Matlab must use the same compiler during compilation.    ", pExportDialog);

    QCheckBox *pDisablePortLabels = new QCheckBox("Disable port labels (for older versions of Matlab)");

    QDialogButtonBox *pExportButtonBox = new QDialogButtonBox(pExportDialog);
    QPushButton *pExportButtonOk = new QPushButton("Ok", pExportDialog);
    QPushButton *pExportButtonCancel = new QPushButton("Cancel", pExportDialog);
    pExportButtonBox->addButton(pExportButtonOk, QDialogButtonBox::AcceptRole);
    pExportButtonBox->addButton(pExportButtonCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *pExportDialogLayout = new QVBoxLayout(pExportDialog);
    pExportDialogLayout->addWidget(pExportDialogLabel1);
    pExportDialogLayout->addWidget(pCompilerGroupBox);
    pExportDialogLayout->addWidget(pArchitectureGroupBox);
    pExportDialogLayout->addWidget(pExportDialogLabel2);
    pExportDialogLayout->addWidget(pDisablePortLabels);
    pExportDialogLayout->addWidget(pExportButtonBox);
    pExportDialog->setLayout(pExportDialogLayout);

    connect(pExportButtonBox, SIGNAL(accepted()), pExportDialog, SLOT(accept()));
    connect(pExportButtonBox, SIGNAL(rejected()), pExportDialog, SLOT(reject()));

    //connect(pExportButtonOk,        SIGNAL(clicked()), pExportDialog, SLOT(accept()));
    //connect(pExportButtonCancel,    SIGNAL(clicked()), pExportDialog, SLOT(reject()));

    if(pExportDialog->exec() == QDialog::Rejected)
    {
        return;
    }


    //QMessageBox::information(gpMainWindow, gpMainWindow->tr("Create Simulink Source Files"),
    //                         gpMainWindow->tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console.\n\nVisual Studio 2008 compiler is supported, although other versions might work as well.."));

    QString fileName;
    if(!mModelFileInfo.fileName().isEmpty())
    {
        fileName = mModelFileInfo.fileName();
    }
    else
    {
        fileName = "untitled.hmf";
    }


        //Open file dialog and initialize the file stream
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Simulink Source Files"),
                                                    gpConfig->getSimulinkExportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel
    QFileInfo file(savePath);
    gpConfig->setSimulinkExportDir(file.absolutePath());

       //Save xml document
    mpModelWidget->saveTo(savePath+"/"+fileName);

    int compiler;
    if(pMSVC2008RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=0;
    }
    else if(pMSVC2008RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        compiler=1;
    }
    else if(pMSVC2010RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=2;
    }
    else/* if(pMSVC2010RadioButton->isChecked() && p64bitRadioButton->isChecked())*/
    {
        compiler=3;
    }


    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToSimulink(savePath, this, pDisablePortLabels->isChecked(), compiler);
    delete(pCoreAccess);


    //Clean up widgets that do not have a parent
    delete(pDisablePortLabels);
    delete(pMSVC2008RadioButton);
    delete(pMSVC2010RadioButton);
    delete(p32bitRadioButton);
    delete(p64bitRadioButton);
}


void SystemContainer::exportToSimulinkCoSim()
{
    QDialog *pExportDialog = new QDialog(gpMainWindow);
    pExportDialog->setWindowTitle("Create Simulink Source Files");

    QLabel *pExportDialogLabel1 = new QLabel(tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console."), pExportDialog);
    pExportDialogLabel1->setWordWrap(true);

    QGroupBox *pCompilerGroupBox = new QGroupBox(tr("Choose compiler:"), pExportDialog);
    QRadioButton *pMSVC2008RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2008"));
    QRadioButton *pMSVC2010RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2010"));
    pMSVC2008RadioButton->setChecked(true);
    QVBoxLayout *pCompilerLayout = new QVBoxLayout;
    pCompilerLayout->addWidget(pMSVC2008RadioButton);
    pCompilerLayout->addWidget(pMSVC2010RadioButton);
    pCompilerLayout->addStretch(1);
    pCompilerGroupBox->setLayout(pCompilerLayout);

    QGroupBox *pArchitectureGroupBox = new QGroupBox(tr("Choose architecture:"), pExportDialog);
    QRadioButton *p32bitRadioButton = new QRadioButton(tr("32-bit (x86)"));
    QRadioButton *p64bitRadioButton = new QRadioButton(tr("64-bit (x64)"));
    p32bitRadioButton->setChecked(true);
    QVBoxLayout *pArchitectureLayout = new QVBoxLayout;
    pArchitectureLayout->addWidget(p32bitRadioButton);
    pArchitectureLayout->addWidget(p64bitRadioButton);
    pArchitectureLayout->addStretch(1);
    pArchitectureGroupBox->setLayout(pArchitectureLayout);

    QLabel *pExportDialogLabel2 = new QLabel("Matlab must use the same compiler during compilation.    ", pExportDialog);

    QCheckBox *pDisablePortLabels = new QCheckBox("Disable port labels (for older versions of Matlab)");

    QDialogButtonBox *pExportButtonBox = new QDialogButtonBox(pExportDialog);
    QPushButton *pExportButtonOk = new QPushButton("Ok", pExportDialog);
    QPushButton *pExportButtonCancel = new QPushButton("Cancel", pExportDialog);
    pExportButtonBox->addButton(pExportButtonOk, QDialogButtonBox::AcceptRole);
    pExportButtonBox->addButton(pExportButtonCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *pExportDialogLayout = new QVBoxLayout(pExportDialog);
    pExportDialogLayout->addWidget(pExportDialogLabel1);
    pExportDialogLayout->addWidget(pCompilerGroupBox);
    pExportDialogLayout->addWidget(pArchitectureGroupBox);
    pExportDialogLayout->addWidget(pExportDialogLabel2);
    pExportDialogLayout->addWidget(pDisablePortLabels);
    pExportDialogLayout->addWidget(pExportButtonBox);
    pExportDialog->setLayout(pExportDialogLayout);

    connect(pExportButtonBox, SIGNAL(accepted()), pExportDialog, SLOT(accept()));
    connect(pExportButtonBox, SIGNAL(rejected()), pExportDialog, SLOT(reject()));

    if(pExportDialog->exec() == QDialog::Rejected)
    {
        return;
    }

    QString fileName;
    if(!mModelFileInfo.fileName().isEmpty())
    {
        fileName = mModelFileInfo.fileName();
    }
    else
    {
        fileName = "untitled.hmf";
    }

        //Open file dialog and initialize the file stream
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Simulink Source Files"),
                                                    gpConfig->getSimulinkExportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel
    QFileInfo file(savePath);
    gpConfig->setSimulinkExportDir(file.absolutePath());




    int compiler;
    if(pMSVC2008RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=0;
    }
    else if(pMSVC2008RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        compiler=1;
    }
    else if(pMSVC2010RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=2;
    }
    else/* if(pMSVC2010RadioButton->isChecked() && p64bitRadioButton->isChecked())*/
    {
        compiler=3;
    }


    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToSimulinkCoSim(savePath, this, pDisablePortLabels->isChecked(), compiler);
    delete(pCoreAccess);


    //Clean up widgets that do not have a parent
    delete(pDisablePortLabels);
    delete(pMSVC2008RadioButton);
    delete(pMSVC2010RadioButton);
    delete(p32bitRadioButton);
    delete(p64bitRadioButton);
}




//! @brief Sets the modelfile info from the file representing this system
//! @param[in] rFile The QFile objects representing the file we want to information about
void SystemContainer::setModelFileInfo(QFile &rFile)
{
    this->mModelFileInfo.setFile(rFile);
}


void SystemContainer::loadParameterFile(const QString &path)
{
    qDebug() << "loadParameterFile()";
    QString parameterFileName = path;
    if(path.isEmpty())
    {
        parameterFileName = QFileDialog::getOpenFileName(gpMainWindow, tr("Load Parameter File"),
                                                             gpConfig->getLoadModelDir(),
                                                             tr("Hopsan Parameter Files (*.hpf *.xml)"));
    }

    if(!parameterFileName.isEmpty())
    {
        mpCoreSystemAccess->loadParameterFile(parameterFileName);
        QFileInfo fileInfo = QFileInfo(parameterFileName);
        gpConfig->setLoadModelDir(fileInfo.absolutePath());
    }
}



//! @brief Function to set the time step of the current system
void SystemContainer::setTimeStep(const double timeStep)
{
    mpCoreSystemAccess->setDesiredTimeStep(timeStep);
    this->hasChanged();
}

void SystemContainer::setVisibleIfSignal(bool visible)
{
    if(this->getTypeCQS() == "S")
    {
        this->setVisible(visible);
    }
}

//! @brief Returns the time step value of the current project.
double SystemContainer::getTimeStep()
{
    return mpCoreSystemAccess->getDesiredTimeStep();
}

//! @brief Check if the system inherits timestep from its parent
bool SystemContainer::doesInheritTimeStep()
{
    return mpCoreSystemAccess->doesInheritTimeStep();
}


//! @brief Returns the number of samples value of the current project.
//! @see setNumberOfLogSamples(double)
size_t SystemContainer::getNumberOfLogSamples()
{
    return mNumberOfLogSamples;
}


//! @brief Sets the number of samples value for the current project
//! @see getNumberOfLogSamples()
void SystemContainer::setNumberOfLogSamples(size_t nSamples)
{
    mNumberOfLogSamples = nSamples;
}

double SystemContainer::getLogStartTime() const
{
    return mLogStartTime;
}

void SystemContainer::setLogStartTime(const double logStartT)
{
    mLogStartTime = logStartT;
}


OptimizationSettings::OptimizationSettings()
{
    //Defaulf values
    mNiter=100;
    mNsearchp=8;
    mRefcoeff=1.3;
    mRandfac=.3;
    mForgfac=0.0;
    mFunctol=.00001;
    mPartol=.0001;
    mPlot=true;
    mSavecsv=false;
    mlogPar = false;
}


SensitivityAnalysisSettings::SensitivityAnalysisSettings()
{
    nIter = 100;
    distribution = UniformDistribution;
}
