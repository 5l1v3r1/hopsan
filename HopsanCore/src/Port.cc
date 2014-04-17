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
//! @file   Port.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains Port base class as well as Sub classes
//!
//$Id$

#include "Port.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "HopsanEssentials.h"
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"

using namespace std;
using namespace hopsan;

//! @brief Port base class constructor
Port::Port(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort)
{
    mPortName = rPortName;
    mNodeType = rNodeType;
    mpComponent = pParentComponent;
    mpParentPort = pParentPort; //Only used by subports in multiports
    mConnectionRequired = true;
    mConnectedPorts.clear();
    mpNode = 0;
    mpStartNode = 0;

    // Create the initial node
    mpNode = getComponent()->getHopsanEssentials()->createNode(mNodeType.c_str());
    this->setNode(mpNode);
    if (getComponent()->getSystemParent())
    {
        getComponent()->getSystemParent()->addSubNode(mpNode);
    }
}


//Destructor
Port::~Port()
{
    if (mpStartNode != 0)
    {
        //! Remove the mapping to eventual system parameters to avoid cowboy-writing in memory after deleted port.
//        for(size_t i = 0; i < mpStartNode->getNumDataVariables(); ++i)
//        {
//FIXA, the parameters will probably be deleted when parent component is deleted -> no problems            getComponent()->getSystemParent()->getSystemParameters().unMapParameter(mpStartNode->getDataPtr(i));
//        }
    }

    if (mpStartNode)
    {
        getComponent()->getHopsanEssentials()->removeNode(mpStartNode);
    }

    // Remove node if it was not handled by disconnect (like in non-connected ports)
    if (mpNode)
    {
        if (mpNode->getOwnerSystem())
        {
            mpNode->getOwnerSystem()->removeSubNode(mpNode);
        }
        getComponent()->getHopsanEssentials()->removeNode(mpNode);
    }
}



//! @brief Returns the type of node that can be connected to this port
const HString &Port::getNodeType() const
{
    return mNodeType;
}


//! @brief Returns the parent component
Component* Port::getComponent() const
{
    if(mpParentPort)
        return mpParentPort->getComponent();
    else
        return mpComponent;
}


//! @brief Returns a pointer to the connected node or 0 if no node exist
//! @param[in] subPortIdx Ignored on non multi ports
Node* Port::getNodePtr(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    return mpNode;
}

//! @brief Adds a subport to a multiport
Port* Port::addSubPort()
{
    mpComponent->addFatalMessage("Port::addSubPort(): This should only be implemented and called from multiports.");
    return 0;
}

//! @brief Removes a subport from multiport
void Port::removeSubPort(Port* pPort)
{
    HOPSAN_UNUSED(pPort)
            mpComponent->addFatalMessage("Port::removeSubPort(): This should only be implemented and called from multiports.");
}

//! @brief This function registers the startvalue parameters from the start node
void Port::registerStartValueParameters()
{
    // Prevent regestering if subports in multiport, or if startnode is missing
    if (mpStartNode && !mpParentPort)
    {
        for(size_t i=0; i<mpStartNode->getNumDataVariables(); ++i)
        {
            const NodeDataDescription* pDesc = mpStartNode->getDataDescription(i);
            const HString desc = "startvalue: Port "+getName();
            const HString name = getName()+"#"+pDesc->name;
            getComponent()->addConstant(name, desc, pDesc->unit, *(mpStartNode->getDataPtr(pDesc->id)));
        }
    }
}


//! @brief Load start values by copying the start values from the port to the node
void Port::loadStartValues()
{
    if(mpStartNode)
    {
        mpStartNode->copyNodeDataValuesTo(mpNode);
        mpStartNode->copySignalDataUnitAndDescriptionTo(mpNode);
    }
}


//! @brief Load start values to the start value container from the node (last values from simulation)
void Port::loadStartValuesFromSimulation()
{
    if(isConnected() && mpStartNode)
    {
        mpNode->copyNodeDataValuesTo(mpStartNode);
    }
}


//! @brief Reads a value from the connected node
//! @param [in] idx The data id of the data to read
//! @param [in] subPortIdx The subPort index of a port in a multiport, (ignored if not a multiport)
//! @details Safe but slow version, will not crash if idx out of bounds
//! @return The data value
//! @ingroup ComponentSimulationFunctions
double Port::readNodeSafe(const size_t idx, const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)

    if (idx < mpNode->getNumDataVariables())
    {
        return mpNode->mDataValues[idx];
    }
    getComponent()->addErrorMessage("data idx out of range in Port::readNodeSafe()");
    return -1;
}


//! @brief Writes a value to the connected node
//! @param [in] idx The data id of the data to write  (Such as NodeHydraulic::Pressure)
//! @param [in] value The value of the data to read
//! @param [in] subPortIdx Ignored for non multi ports
//! @details Safe but slow version, will not crash if idx out of bounds
//! @ingroup ComponentSimulationFunctions
void Port::writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if (idx < mpNode->getNumDataVariables())
    {
        mpNode->mDataValues[idx] = value;
    }
    else
    {
        getComponent()->addErrorMessage("data idx out of range in Port::writeNodeSafe()");
    }
}

//! @brief Returns the node pointer in the port
//! @param [in] subPortIdx Ignored for non multi ports
//! @returns The node pointer in this port
const Node *Port::getNodePtr(const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    return mpNode;
}

//! @brief Get a ptr to the data variable in the node
//! @param [in] idx The id of the data variable to return ptr to
//! @param [in] subPortIdx Ignored for non multi ports
//! @returns Pointer to data vaariable or 0 if idx was not found
double *Port::getNodeDataPtr(const size_t idx, const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    if (idx < mpNode->getNumDataVariables())
    {
        return mpNode->getDataPtr(idx);
    }
    else
    {
        return 0;
    }
}


//! @brief Set the node that the port is connected to
//! @param [in] pNode A pointer to the Node, or 0 for NC dummy node
void Port::setNode(Node* pNode)
{
    if (!pNode)
    {
        getComponent()->addFatalMessage("In Port::setNode(), You cant set a NULL node ptr");
    }
    else
    {
        //! @todo what to do with log data (clear maybe) if dummy node
        mpNode->removeConnectedPort(this);
        mpNode = pNode;
        mpNode->addConnectedPort(this);
    }
}


//! @brief Adds a pointer to an other connected port to a port
//! @param [in] pPort A pointer to the other port
//! @param [in] subPortIdx Ignored for non multi ports
void Port::addConnectedPort(Port* pPort, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    mConnectedPorts.push_back(pPort);
}


//! @brief Removes a pointer to an other connected port from a port
//! @param [in] pPort The pointer to the other port to be removed
//! @param [in] subPortIdx Ignored for non multi ports
void Port::eraseConnectedPort(Port* pPort, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        //printf("*it: %p pPort: %p", (void*)(*it), (void*)pPort);
        //cout << endl;
        //cout << "*it: " << *it << " pPort: " << pPort << endl;
        if (*it == pPort)
        {
            mConnectedPorts.erase(it);
            return;
        }
    }
    cout << "Error: You tried to erase port ptr that did not exist in the connected ports list" << endl;
}


//! @brief Get a vector of pointers to all other ports connected connected to this one
//! @param [in] subPortIdx Ignored for non multi ports
//! @returns A refernce to the internal vector of connected port pointers
//! @todo maybe should return const vector so that contents my not be changed
vector<Port*> &Port::getConnectedPorts(const int subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    return mConnectedPorts;
}

//! @brief Returns the number of ports connected to this port
//! @param[in] subPortIdx The index of the subPort to check. Ignored on non multi ports
//! @returns The number of connected ports
size_t Port::getNumConnectedPorts(const int subPortIdx)
{
    return getConnectedPorts(subPortIdx).size();
}

//! @brief Creates a start node in the port
//! @param[in] rNodeType The type of node to create (Such as NodeHydraulic)
void Port::createStartNode(const HString &rNodeType)
{
    mpStartNode = getComponent()->getHopsanEssentials()->createNode(rNodeType.c_str());
    //!< @todo Maye I dont even need to create startnodes for subports in multiports, in that case, move this line into if below

    // Prevent registering startvalues for subports in multiports, It will be very difficult to ensure that those would actually work as expected
    if (mpParentPort == 0)
    {
        registerStartValueParameters();
    }
}

//! @note This one should be called by system, do not call this manually (that will create a mess)
void Port::setVariableAlias(const HString &rAlias, const int id)
{
    //! @todo check id
    // First remove it if already set
    std::map<HString, int>::iterator it = mVariableAliasMap.begin();
    while (it!=mVariableAliasMap.end())
    {
        if (it->second == id)
        {
            mVariableAliasMap.erase(it);
            // Restart search if something was removed as itterator breaks
            it = mVariableAliasMap.begin();
        }
        else
        {
             ++it;
        }
    }

    // Replace with new name, if not empty
    if (!rAlias.empty())
    {
        mVariableAliasMap.insert(std::pair<HString, int>(rAlias, id));
    }
}

//! @brief Get the alias name for a spcific node variable id
//! @param [in] id The node data id of the requested variable (Ex: NodeHydraulic::Pressure)
//! @return The alias name or empty string if no alias name exist for requested variable
const HString &Port::getVariableAlias(const int id)
{
    std::map<HString, int>::const_iterator it;
    for(it=mVariableAliasMap.begin();it!=mVariableAliasMap.end();++it)
    {
        if (it->second == id)
        {
            return it->first;
        }
    }
    return mEmptyString;
}

//! @brief Get the variable id for a specific alias name
//! @param [in] rAlias The alias name to search for
//! @returns The variable id (integer value) (Ex:: NodeHydarulic::Pressure) or -1 if not found
int Port::getVariableIdByAlias(const HString &rAlias) const
{
    std::map<HString, int>::const_iterator it = mVariableAliasMap.find(rAlias);
    {
        if (it!=mVariableAliasMap.end())
        {
            return it->second;
        }
    }
    return -1;
}

//! @brief Get a pointer to the start node
//! @returns StartNode ptr or 0 if no startnode
Node *Port::getStartNodePtr()
{
    return mpStartNode;
}

//! @brief Check if log data  exist in the ports node
//! @param[in] subPortIdx Ignored on non multi ports
//! @returns True or False
bool Port::haveLogData(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if (mpNode)
    {
        // Here we assume that timevector DOES exist. If simulation code is correct it should exist
        return !mpNode->mDataStorage.empty();
    }
    return false;
}


//! @brief Get all node data descriptions
//! @param [in] subPortIdx Ignored on non multi ports
//! @returns A const pointer to the internal node vector with node data descriptions
const std::vector<NodeDataDescription>* Port::getNodeDataDescriptions(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    // We prefere to use the startnode
    if (mpStartNode)
    {
        return mpStartNode->getDataDescriptions();
    }
    else
    {
        // This node could contain descriptions copied by someone else
        return mpNode->getDataDescriptions();
    }
}


//! @brief Get a specific node data description
//! @param [in] dataid The node data id (Such as NodeHydraulic::Pressure)
//! @param [in] subPortIdx Ignored on non multi ports
//! @returns A const pointer to the node data description, or 0 if no node exist
const NodeDataDescription* Port::getNodeDataDescription(const size_t dataid, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    // We prefere to use the startnode
    if (mpStartNode)
    {
        return mpStartNode->getDataDescription(dataid);
    }
    else
    {
        // This node could contain descriptions copied by someone else
        return mpNode->getDataDescription(dataid);
    }
}


//! @brief Ask the node for the dataId for a particular data name such as (Pressure)
//! @details This is a wrapper function for the actual Node function,
//! @param [in] rName The name of the variable (Such as Pressure)
//! @param [in] subPortIdx Ignored on non multi ports
//! @returns The node data id (positive integer) if the variable name is found else returns -1 to indicate failure
int Port::getNodeDataIdFromName(const HString &rName, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    //! @todo since mpNode should always be set maybe we could remove (almost) all the checks (but not for multiports their mpNOde will be 0)
    if (mpNode != 0)
    {
        return mpNode->getDataIdFromName(rName);
    }
    else
    {
        return -1;
    }
}

//! @brief A help function taht makes it possible to overwrite the unit and description of scalar signal node variables
//! @todo is this even needed anymore now taht we have in/out variables
void Port::setSignalNodeUnitAndDescription(const HString &rUnit, const HString &rDescription)
{
    //! @todo multiport version needed
    mpNode->setSignalDataUnitAndDescription(rUnit, rDescription);
    if (mpStartNode)
    {
        mpStartNode->setSignalDataUnitAndDescription(rUnit, rDescription);
    }
}

//! @param [in] subPortIdx Ignored on non multi ports
vector<double> *Port::getLogTimeVectorPtr(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if (mpNode != 0)
    {
        ComponentSystem *pOwnerSys = mpNode->getOwnerSystem();
        if (pOwnerSys)
        {
            return pOwnerSys->getLogTimeVector();
        }
    }
    return 0; //Nothing found return 0
}

//! @param [in] subPortIdx Ignored on non multi ports
vector<vector<double> > *Port::getLogDataVectorPtr(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if (mpNode != 0)
    {
        return &(mpNode->mDataStorage);
    }
    else
    {
        return 0;
    }
}

bool Port::isInterfacePort() const
{
    return getComponent()->isComponentSystem();
}

//! @param [in] subPortIdx Ignored on non multi ports
vector<double> *Port::getDataVectorPtr(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if(mpNode != 0)
    {
        return &(mpNode->mDataValues);
    }
    else
    {
        return 0;
    }
}

//! @brief Returns the number of data variables in the node
//! @returns The number of data varaibles in the node
size_t Port::getNumDataVariables() const
{
    return mpNode->getNumDataVariables();
}


//! @brief Get the actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param[in] subPortIdx Ignored on non multi ports
//! @returns the start value
double Port::getStartValue(const size_t idx, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if(mpStartNode && getComponent()->getSystemParent()->doesKeepStartValues())
    {
        return mpNode->getDataValue(idx);
    }
    else if(mpStartNode)
    {
        return mpStartNode->getDataValue(idx);
    }
    getComponent()->addErrorMessage("Port::getStartValue(): Port does not have a start value.");
    return -1;
}


//! @brief Set the an actual start value of the port
//! @param [in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param [in] value is the start value that should be written
//! @param[in] subPortIdx Ignored on non multi ports
void Port::setDefaultStartValue(const size_t idx, const double value, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if(mpStartNode)
    {
        mpStartNode->setDataValue(idx, value);
    }
    else
    {
        getComponent()->addWarningMessage("Tried to set StartValue for port: "+getName()+" This was ignored because this port does not have a StartNode.");
    }
}


//! @brief Disables start value for specified data type
//! @param idx Data index of start value to be disabled
void Port::disableStartValue(const size_t idx)
{
    if (mpStartNode)
    {
        // The start value has already been registered as a parameter in the component, so we must unregister it.
        // This is probably not the most beautiful solution.
        HString name = getName()+"#"+mpStartNode->getDataDescription(idx)->name;
        mpComponent->addDebugMessage("Disabling_StartValue: "+name);
        mpComponent->unRegisterParameter(name);

        // Note, the startNode and its value will remain, it will also be copied every time.
        // Components should automatically write the correct initial value to nodes in initialize
        // If a startvalue has been disabled you can not change it, (it actually means that it is hiddden)
    }
}


//! @brief Check if the port is curently connected
//! @brief Returns True or False
bool Port::isConnected()
{
    return (mConnectedPorts.size() > 0);
}

//! @brief Check if this port is connected to other port
//! @todo how do we handle multiports
bool Port::isConnectedTo(Port *pOtherPort)
{
    //! @todo this is a hack for now, since isConnectedTo is overloaded we want the actual multiport to check, Make smarter in the future
    if (pOtherPort->isMultiPort())
    {
        return pOtherPort->isConnectedTo(this);
    }
    else
    {
        std::vector<Port*>::iterator pit;
        for(pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
        {
            //printf("isConnectedTo: *pit: %p pOtherPort: %p", (void*)(*pit), (void*)pOtherPort);
            //cout << endl;
            if ( *pit == pOtherPort )
            {
                return true;
            }
        }
        return false;
    }
}


//! @brief Check if the port MUST be connected
bool Port::isConnectionRequired()
{
    return mConnectionRequired;
}

size_t Port::getNumPorts()
{
    return 1;
}

//! @brief Convenience functin to check if port is multiport
bool Port::isMultiPort() const
{
    return false;
}

Port *Port::getParentPort() const
{
    return mpParentPort;
}

//! @brief Get the port type
PortTypesEnumT Port::getPortType() const
{
    return UndefinedPortType;
}

//! @brief Get the External port type (virtual, should be overloaded in systemports only)
PortTypesEnumT Port::getExternalPortType()
{
    return getPortType();
}

//! @brief Get the Internal port type (virtual, should be overloaded in systemports only)
PortTypesEnumT Port::getInternalPortType()
{
    return getPortType();
}


//! @brief Get the port name
const HString &Port::getName() const
{
    return mPortName;
}


//! @brief Get the name of the commponent that the port is attached to
const HString &Port::getComponentName() const
{
    return getComponent()->getName();
}

//! @brief Get port description
//! @returns Port description
const HString &Port::getDescription() const
{
    return mDescription;
}

//! @brief Set port description
//! @param [in] rDescription The new description
void Port::setDescription(const HString &rDescription)
{
    mDescription = rDescription;
}


SystemPort::SystemPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    // Do nothing special
}

PortTypesEnumT SystemPort::getPortType() const
{
    return SystemPortType;
}

//! @brief Get the External port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getExternalPortType()
{
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        //External ports component parents will belong to the same system as our component parent
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent()->getSystemParent() )
        {
            //! @todo for now we return the first one we find, usually thi is corect except when you are mixing powerports and readports, powerports should be returned in that case but I dont know how to fix this except going through ALL ports every time
            return (*pit)->getPortType();
        }
    }

    //If no external ports found return our actual type (systemport)
    return getPortType();
}

//! @brief Get the Internal port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getInternalPortType()
{
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        //Internal ports component parents will belong to our component parent
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent() )
        {
            //! @todo for now we return the first one we find, usually thi is corect except when you are mixing powerports and readports, powerports should be returned in that case but I dont know how to fix this except going through ALL ports every time
            return (*pit)->getPortType();
        }
    }

    //If no internal ports found return our actual type (systemport)
    return getPortType();
}


//! @brief PowerPort constructor
PowerPort::PowerPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}

PortTypesEnumT PowerPort::getPortType() const
{
    return PowerPortType;
}


ReadPort::ReadPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    createStartNode(rNodeType);
}

PortTypesEnumT ReadPort::getPortType() const
{
    return ReadPortType;
}

void ReadPort::writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx)
{
    HOPSAN_UNUSED(idx)
    HOPSAN_UNUSED(value)
    HOPSAN_UNUSED(subPortIdx)
    mpComponent->addErrorMessage("ReadPort::writeNodeSafe(): Could not write to port, this is a ReadPort.");
}


void ReadPort::writeNode(const size_t idx, const double value, const size_t subPortIdx)
{
    HOPSAN_UNUSED(idx)
    HOPSAN_UNUSED(value)
    HOPSAN_UNUSED(subPortIdx)
    mpComponent->addErrorMessage("ReadPort::writeNode(): Could not write to port, this is a ReadPort.");
}

void ReadPort::loadStartValues()
{
    // Prevent loading startvalues if this port is connected, then the write node will set the start value
    if (!isConnected())
    {
        Port::loadStartValues();
    }
}

//bool ReadPort::hasConnectedExternalSystemWritePort()
//{
//    // Frist figure out who my system parent is
//    Component *pSystemParent;
//    if (this->getComponent()->isComponentSystem())
//    {
//        // If my parent component is a system, then I am a kind of systemport (I am a normal port as interface port on a system)
//        pSystemParent = this->getComponent();
//    }
//    else
//    {
//        // Take my parent components systemparent
//        pSystemParent = this->getComponent()->getSystemParent();
//    }

//    // Now check all connected ports, find a write port
//    vector<Port*>::iterator portIt;
//    for (portIt = mConnectedPorts.begin(); portIt != mConnectedPorts.end(); ++portIt)
//    {
//        Port *pPort = (*portIt);
//        if (pPort->getPortType() == WritePortType)
//        {
//            // Check if this writport belongs to a component whos parent system is the same as my system grand parent
//            if (pPort->getComponent()->getSystemParent() == pSystemParent->getSystemParent())
//            {
//                return true;
//            }
//        }
//    }
//    return false;
//}

bool ReadPort::isConnectedToWriteOrPowerPort()
{
    vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        Port *pPort = (*pit);
        if ((pPort->getPortType() == WritePortType) || (pPort->getPortType() == PowerPortType) || (pPort->getPortType() == PowerMultiportType))
        {
            return true;
        }
    }
    return false;
}

void ReadPort::forceLoadStartValue()
{
    Port::loadStartValues();
}

WritePort::WritePort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    createStartNode(mNodeType);
}

PortTypesEnumT WritePort::getPortType() const
{
    return WritePortType;
}


//double WritePort::readNode(const size_t idx, const size_t subPortIdx) const
//{
//    HOPSAN_UNUSED(idx)
//    HOPSAN_UNUSED(subPortIdx)
//    mpComponent->addWarningMessage("WritePort::readNode(): Could not read from port, this is a WritePort");
//    return -1;
//}

MultiPort::MultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    // Do nothing special
}

MultiPort::~MultiPort()
{
    // Delete all subports thay may remain, if everything is working this shoudl be zero
    //! @todo removed assert, BUT problem needs to be fixed /Peter
    if (mSubPortsVector.size() != 0)
    {
        getComponent()->addFatalMessage("~MultiPort(): mSubPortsVector.size() != 0 in multiport destructor (will fix later)");
    }
}

PortTypesEnumT MultiPort::getPortType() const
{
    return MultiportType;
}

bool MultiPort::isMultiPort() const
{
    return true;
}


//! @brief Reads a value from the connected node
//! @param [in] idx The data id of the data to read
//! @param [in] subPortIdx The subPort index in the multi port
//! @details Safe but slow version, will not crash if idx out of bounds
//! @return The data value or -1 if any of the idxes are out of range
//! @ingroup ComponentSimulationFunctions
double MultiPort::readNodeSafe(const size_t idx, const size_t subPortIdx) const
{
    if (subPortIdx < mSubPortsVector.size())
    {
        return mSubPortsVector[subPortIdx]->readNodeSafe(idx);
    }
    getComponent()->addErrorMessage("portIdx out of range in MultiPort::readNodeSafe()");
    return -1;
}

//! @brief Writes a value to the connected node
//! @param [in] idx The data id of the data to write (Such as NodeHydraulic::Pressure)
//! @param [in] value The value of the data to read
//! @param [in] subPortIdx The subport to write to, (range check is performed)
//! @details Safe but slow version, will not crash if idx out of bounds
//! @ingroup ComponentSimulationFunctions
void MultiPort::writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx)
{
    if (subPortIdx < mSubPortsVector.size())
    {
        mSubPortsVector[subPortIdx]->writeNodeSafe(idx,value);
    }
    else
    {
        getComponent()->addErrorMessage("portIdx out of range in MultiPort::readNodeSafe()");
    }
}

//! @brief Returns the node pointer from one of the subports in the port (const version)
//! @param [in] subPortIdx The sub port to retreive from, (range check is NOT performed!)
//! @returns The node pointer in the sub port
const Node *MultiPort::getNodePtr(const size_t subPortIdx) const
{
    return mSubPortsVector[subPortIdx]->getNodePtr();
}

//! @todo why do we even want a unsafe getNodeDataPtr it should be the safe version
double *MultiPort::getNodeDataPtr(const size_t idx, const size_t subPortIdx) const
{
    //If we try to access node data for subport that does not exist then return multiport shared dummy safe ptr
    if (subPortIdx >= mSubPortsVector.size())
    {
        return Port::getNodeDataPtr(idx, subPortIdx);
    }
    else
    {
        return mSubPortsVector[subPortIdx]->getNodeDataPtr(idx);
    }
}

//! @brief Get all node data descriptions from a connected sub port node
//! @param [in] subPortIdx The subport idx to fetch from (range is checked)
//! @returns A const pointer to the internal node vector with node data descriptions or 0 if subPortIdx is out of range or if no node exist in port
const std::vector<NodeDataDescription>* MultiPort::getNodeDataDescriptions(const size_t subPortIdx)
{
    if (subPortIdx < mSubPortsVector.size())
    {
        return mSubPortsVector[subPortIdx]->getNodeDataDescriptions();
    }
    return 0;
}

//! @brief Get a specific node data description from a connected sub port node
//! @param [in] dataid The node data id (Such as NodeHydraulic::Pressure)
//! @param [in] subPortIdx The subport idx to fetch from (range is NOT checked)
//! @returns A const pointer to the node data description, or 0 if no node exist
const NodeDataDescription* MultiPort::getNodeDataDescription(const size_t dataid, const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getNodeDataDescription(dataid);
    }
    else if (mpStartNode)
    {
        return mpStartNode->getDataDescription(dataid);
    }
    return 0;
}

//! @brief Ask the node for the dataId for a particular data name such as (Pressure)
//! @details This is a wrapper function for the actual Node function,
//! @param [in] rName The name of the variable (Such as Pressure)
//! @param [in] subPortIdx The subPort to ask, (range is NOT checked)
//! @returns The node data id (positive integer) if the variable name is found else returns -1 to indicate failure
int MultiPort::getNodeDataIdFromName(const HString &rName, const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getNodeDataIdFromName(rName);
    }
    return -1;
}

bool MultiPort::haveLogData(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->haveLogData();
    }
    return false;
}

std::vector<double> *MultiPort::getLogTimeVectorPtr(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getLogTimeVectorPtr();
    }
    return 0;
}

std::vector<std::vector<double> > *MultiPort::getLogDataVectorPtr(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getLogDataVectorPtr();
    }
    return 0;
}

std::vector<double> *MultiPort::getDataVectorPtr(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getDataVectorPtr();
    }
    return 0;
}

//! @brief Get the an actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param[in] subPortIdx The sub port to get start value from
//! @todo shouldnt this be called get default startvalue, to avoid confusion with initial value (I am not sure)
//! @returns the start value
double MultiPort::getStartValue(const size_t idx, const size_t subPortIdx)
{
    if(mpStartNode && mpComponent->getSystemParent()->doesKeepStartValues())
    {
        return mSubPortsVector[subPortIdx]->mpNode->getDataValue(idx);
    }
    else if(mpStartNode)
    {
        return mpStartNode->getDataValue(idx);
    }
    mpComponent->addErrorMessage("MultiPort::getStartValue(): Port does not have a start value.");
    return -1.0;
}

void MultiPort::loadStartValues()
{
    if(mpStartNode)
    {
        for(size_t p=0; p<mSubPortsVector.size(); ++p)
        {
            mpStartNode->copyNodeDataValuesTo(mSubPortsVector[p]->mpNode);
            mpStartNode->copySignalDataUnitAndDescriptionTo(mSubPortsVector[p]->mpNode);
        }
    }
}

void MultiPort::loadStartValuesFromSimulation()
{
    //! @todo what about this one then how should we handle this
}

bool MultiPort::isConnectedTo(Port *pOtherPort)
{
    if (this->isMultiPort() && pOtherPort->isMultiPort())
    {
        getComponent()->addFatalMessage("In isConnectedTo() both ports are multiports, this should not happen!");
        return false;
    }

    for (size_t i=0; i<mSubPortsVector.size(); ++i)
    {
        if (mSubPortsVector[i]->isConnectedTo(pOtherPort))
        {
            return true;
        }
    }

    return false;
}

//! @brief Check if the port is curently connected
bool MultiPort::isConnected()
{
    //! @todo actaully we should check all subports if they are connected (but a subport should not exist if not connected)
    return (mSubPortsVector.size() > 0);
}

size_t MultiPort::getNumPorts()
{
    return mSubPortsVector.size();
}

//! @brief Removes a specific subport
void MultiPort::removeSubPort(Port* ptr)
{
    std::vector<Port*>::iterator spit;
    for (spit=mSubPortsVector.begin(); spit!=mSubPortsVector.end(); ++spit)
    {
        if ( *spit == ptr )
        {
            mSubPortsVector.erase(spit);
            delete ptr;
            break;
        }
    }
}

//! @brief Returns the node pointer from one of the subports in the port
//! @param [in] subPortIdx The sub port to retreive from, (range check is performed)
//! @returns The node pointer in the sub port, or 0 if index out of range
Node *MultiPort::getNodePtr(const size_t subPortIdx)
{
    if(mSubPortsVector.size() <= subPortIdx)
    {
        mpComponent->addWarningMessage("MultiPort::getNodePtr(): mSubPortsSVector.size() <= portIdx");
        return 0;
    }
    return mSubPortsVector[subPortIdx]->getNodePtr();
}

//! @brief Get all the connected ports
//! @param[in] subPortIdx The sub port to get connected ports from, Use -1 to indicate that all subports should be considered
//! @returns A vector with port pointers to connected ports
std::vector<Port*> &MultiPort::getConnectedPorts(const int subPortIdx)
{
    if (subPortIdx<0)
    {
        //Ok lets return ALL connected ports
        //! @todo since this function returns a reference to the internal vector we need a new memberVector
        mAllConnectedPorts.clear();
        for (size_t i=0; i<mSubPortsVector.size(); ++i)
        {
            for (size_t j=0; j<mSubPortsVector[i]->getConnectedPorts().size(); ++j)
            {
                mAllConnectedPorts.push_back(mSubPortsVector[i]->getConnectedPorts()[j]);
            }
        }

        return mAllConnectedPorts;
    }
    else
    {
        return mSubPortsVector[subPortIdx]->getConnectedPorts();
    }
}

void MultiPort::setNode(Node* pNode)
{
    HOPSAN_UNUSED(pNode)
    // Do nothing for multiports, only subports are interfaced with
}

PowerMultiPort::PowerMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    MultiPort(rNodeType, rPortName, pParentComponent, pParentPort)
{
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}

PortTypesEnumT PowerMultiPort::getPortType() const
{
    return PowerMultiportType;
}

//! @brief Adds a subport to a powermultiport
Port* PowerMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(PowerPortType, mNodeType, "noname_subport", 0, this) );
    return mSubPortsVector.back();
}

ReadMultiPort::ReadMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    MultiPort(rNodeType, rPortName, pParentComponent, pParentPort)
{
    // Do nothing special
}

PortTypesEnumT ReadMultiPort::getPortType() const
{
    return ReadMultiportType;
}

//! @brief Adds a subport to a readmultiport
Port* ReadMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(ReadPortType, mNodeType, "noname_subport", 0, this) );
    return mSubPortsVector.back();
}

//! @brief A very simple port factory, no need to complicate things with the more advanced one as we will only have a few fixed port types.
//! @param [in] portType The type of port to create
//! @param [in] nodeType The type of node that the port should contain
//! @param [in] name The name of the port
//! @param [in] pPortOwner A pointer to the owner component
//! @param [in] pParentPort A pointer to the parent port in case of creation of a subport to a multiport
//! @return A pointer to the created port
Port* hopsan::createPort(const PortTypesEnumT portType, const HString &rNodeType, const HString &rName, Component *pParentComponent, Port *pParentPort)
{
    switch (portType)
    {
    case PowerPortType :
        return new PowerPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case WritePortType :
        return new WritePort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case ReadPortType :
        return new ReadPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case SystemPortType :
        return new SystemPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case PowerMultiportType :
        return new PowerMultiPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case ReadMultiportType :
        return new ReadMultiPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    default :
       return 0;
    }
}

//! @brief Converts a PortTypeEnum to string
//! @param [in] type The port type enum
//! @return The port type in string format
HString hopsan::portTypeToString(const PortTypesEnumT type)
{
    switch (type)
    {
    case PowerPortType :
        return "PowerPortType";
        break;
    case ReadPortType :
        return "ReadPortType";
        break;
    case WritePortType :
        return "WritePortType";
        break;
    case SystemPortType :
        return "SystemPortType";
        break;
    case MultiportType:
        return "MultiportType";
        break;
    case PowerMultiportType:
        return "PowerMultiportType";
        break;
    case ReadMultiportType:
        return "ReadMultiportType";
        break;
    default :
        return "UndefinedPortType";
    }
}
