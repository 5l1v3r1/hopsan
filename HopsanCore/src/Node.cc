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
//! @file   Node.cc
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Node base classes
//!
//$Id$

//! @class hopsan::Node
//! @brief The Node base class
//! @ingroup Nodes

#include <fstream>
#include <cassert>
#include <iostream>
#include <sstream>
#include "Node.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "Port.h"
#include "ComponentSystem.h"

using namespace std;
using namespace hopsan;

//! Node base class constructor
//! @param [in] datalength The length of the data vector
Node::Node(const size_t datalength)
{
    // Make sure clear (should not really be needed)
    mDataValues.clear();
    mDataStorage.clear();
    mConnectedPorts.clear();

    // Init pointer
    mpOwnerSystem = 0;

    // Set initial node type
    mNodeType = "UndefinedNodeType";

    // Resize
    mDataDescriptions.resize(datalength);
    mDataValues.resize(datalength,0.0);

    // Default dissabled logging
    disableLog();
}

Node::~Node()
{
    // Nothing special, but needed to prevent warning about undefined behaviour
}


//!
//! @brief returns the node type
//!
const HString &Node::getNodeType() const
{
    return mNodeType;
}

//! @brief Returns the total number of variables in a node
size_t Node::getNumDataVariables() const
{
    return mDataValues.size();
}


double *Node::getDataPtr(const size_t data_type)
{
    return &mDataValues[data_type];
}


void Node::setNiceName(const HString &rNicename)
{
    mNiceName = rNicename;
}


const HString &Node::getNiceName() const
{
    return mNiceName;
}


//! @brief Set data name and unit for a specified data variable
//! @param [in] id This is the ENUM data id
//! @param [in] rName The variable name
//! @param [in] rShortname The short variable name
//! @param [in] rUnit The variable unit
//! @param [in] vartype The type of the variable
void Node::setDataCharacteristics(const size_t id, const HString &rName, const HString &rShortname, const HString &rUnit, const NodeDataVariableTypeEnumT vartype)
{
    mDataDescriptions[id].id = id;
    mDataDescriptions[id].name = rName;
    mDataDescriptions[id].shortname = rShortname;
    mDataDescriptions[id].unit = rUnit;
    mDataDescriptions[id].varType = vartype;
}


//! Get a specific data name and unit
//! @param [in] id This is the ENUM data id
//! @returns A pointer to the desired description or 0 if out of bounds
const NodeDataDescription* Node::getDataDescription(const size_t id) const
{
    if (id < mDataDescriptions.size())
    {
        return &(mDataDescriptions[id]);
    }
    return 0;
}

//! @brief This function can be used to set unit string and displayName for signal nodes ONLY
void Node::setSignalDataUnitAndDescription(const HString &/*rUnit*/, const HString &/*rDescription*/)
{
    // Do nothing by default
}


//! @brief This function gives you the data Id for a names data variable
//! @param [in] rName The data name
//! @return The Id, -1 if requested data name is not found
int Node::getDataIdFromName(const HString &rName) const
{
    for (size_t i=0; i<mDataDescriptions.size(); ++i)
    {
        if (rName == mDataDescriptions.at(i).name)
        {
            return i;
        }
    }
    return -1; //Did not find this name return -1 to signal failure
}


//! @brief Get the vector of data descriptions for the node
//! @returns A pointer to the descriptions vector
const std::vector<NodeDataDescription>* Node::getDataDescriptions() const
{
    return &mDataDescriptions;
}


//! @brief Copy variable valus from this to pNode
//! @param pOtherNode The destination node to copy into
void Node::copyNodeDataValuesTo(Node *pOtherNode) const
{
    // Copy variable valus from this to pNode
    if(pOtherNode->getNodeType()==this->getNodeType())
    {
        for(size_t i=0; i<pOtherNode->getNumDataVariables(); ++i)
        {
            //! @todo look over if all vector positions should be set or not.
            pOtherNode->mDataValues[i] = mDataValues[i];
        }
        setTLMNodeDataValuesTo(pOtherNode); //Handles Wave, imp variables and similar
    }
    else
    {
        if (mpOwnerSystem)
        {
            mpOwnerSystem->addFatalMessage("Node::copyNodeDataValuesTo(): Nodes do not have the same type.");
        }
    }
}

void Node::copySignalDataUnitAndDescriptionTo(Node* /*pOtherNode*/) const
{
    // This is only possible in signal nodes
}

void Node::setTLMNodeDataValuesTo(Node* /*pOtherNode*/) const
{
    // This method schould be implemented in child Nodes
}


//! @brief Pre allocate memory for the needed amount of log data
void Node::preAllocateLogSpace(const size_t nLogSlots)
{
    // Dont try to allocate if we are not going to log
    if (mDoLog)
    {
        mDataStorage.resize(nLogSlots, vector<double>(mDataValues.size()));
    }
}


//! @brief Copy current data vector into log storage at given logslot
//! @warning No bounds check is done
void Node::logData(const size_t logSlot)
{
    if (mDoLog)
    {
        mDataStorage[logSlot] = mDataValues;
    }
}


//! @brief Returns a pointer to the component with the write port in the node.
//! If connection is ok, any node can only have one write port. If no write port exists, a null pointer is returned.
Component *Node::getWritePortComponentPtr() const
{
    for(size_t i=0; i<mConnectedPorts.size(); ++i)
    {
        if(mConnectedPorts.at(i)->getPortType() == WritePortType)
        {
            return mConnectedPorts.at(i)->getComponent();
        }
    }

    return 0;   //Return null pointer if no write port was found
}


//! @brief Adds a pointer to a port connected to this node
//! @param [in] pPort The port pointer
void Node::addConnectedPort(Port *pPort)
{
    //Prevent duplicate port registration that can happen when "merging" nodes
    //The other code (connect) will be easier to write if we handle this in here
    bool found = false;
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            found = true;
            break;
            //cout << "Warning: you are trying to add a Port that does already exist in this node  (does nothing)" << endl;
        }
    }

    if (!found)
    {
        mConnectedPorts.push_back(pPort);
    }
}


//! @brief Removes a port pointer from this node, NOT delete, only remove
//! @param [in] pPort The port pointer to be removed
void Node::removeConnectedPort(const Port *pPort)
{
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            mConnectedPorts.erase(it);
            break;
        }
    }
}


//! Check if a specified port is connected to this node
//! @param [in] pPort The port pointer to find
//! @return Is specified port connected (true or false)
bool Node::isConnectedToPort(const Port *pPort) const
{
    vector<Port*>::const_iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            return true;
        }
    }
    return false;
}


//! @brief Enable node data logging
void Node::enableLog()
{
    if(!mForceDisableLog)
    {
        mDoLog = true;
    }
}


//! @brief Disable node data logging
void Node::disableLog()
{
    mDoLog = false;
    mDataStorage.clear();
}


void Node::setForceDisableLog(bool value)
{
    mForceDisableLog = value;
}

bool Node::getForceDisableLog() const
{
    return mForceDisableLog;
}


//! @brief Returns the number of attached ports of a specific type
int Node::getNumberOfPortsByType(const int type) const
{
    int nPorts = 0;
    std::vector<Port*>::const_iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if ((*it)->getPortType() == type)
        {
            nPorts++;
        }
    }
    return nPorts;
}

size_t Node::getNumConnectedPorts() const
{
    return mConnectedPorts.size();
}

//! @brief Returns a pointer to the ComponentSystem that own this Node
ComponentSystem *Node::getOwnerSystem() const
{
    return mpOwnerSystem;
}

HString hopsan::nodeDataVariableTypeAsString(const NodeDataVariableTypeEnumT type)
{
    switch(type)
    {
    case DefaultType:
        return "Default";
    case IntensityType:
        return "Intensity";
    case FlowType:
        return "Flow";
    case TLMType:
        return "TLM";
    case HiddenType:
        return "Hidden";
    default:
        return "Incorrect Node Variable Type";
    }
}
