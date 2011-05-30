/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, BjÃ¶rn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at LinkÃ¶ping University. Modifying, using or
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

//! @class Node
//! @brief The Node base class
//! @ingroup Nodes

#include <fstream>
#include <cassert>
#include <iostream>
#include <sstream>
#include "Node.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "Port.h"

using namespace std;
using namespace hopsan;

//! Node base class constructor
//! @param [in] datalength The length of the data vector
Node::Node(size_t datalength)
{
    //Make sure clear (should not really be needed)
    mDataVector.clear();
    mDataStorage.clear();
    mTimeStorage.clear();
    mPortPtrs.clear();

    //Init pointer
    mpOwnerSystem = 0;

    //Sett initial node type
    mNodeType = "UndefinedNode";

    //Resize
    mDataVector.resize(datalength,0.0);
    mDataNames.resize(datalength,"");
    mDataUnits.resize(datalength,"");
    mPlotBehaviour.resize(datalength, Node::PLOT);

    //Set log specific variables
    mLogSpaceAllocated = false;
    mLogCtr = 0;
    //Default log node allways
    mDoLog = true;
    mLogTimeDt = 0.0;
    mLastLogTime = 0.0; //Initial valus should not matter, will be overwritten when selecting log amount
    mLogSlots = 0;
}


//!
//! @brief returns the node type
//!
NodeTypeT &Node::getNodeType()
{
    return mNodeType;
}


//!
//! @brief set data in node
//! @param [in] data_type Identifier for the typ of node data to set
//! @param [in] data The data value
//!
void Node::setData(const size_t &data_type, const double &data)
{
    mDataVector[data_type] = data;
}


//!
//! @brief get data from node
//! @param [in] data_type Identifier for the type of node data to get
//! @return The data value
//!
double Node::getData(const size_t data_type)
{
    return mDataVector[data_type];
}


//!
//! @brief get data reference from node, (Dont us this function, It may be removed)
//! @param [in] data_type Identifier for the type of node data to get
//! @return A reference to the data value
//!
double &Node::getDataRef(const size_t data_type)
{
    return mDataVector[data_type];
}

double *Node::getDataPtr(const size_t data_type)
{
    return &mDataVector[data_type];
}


//! Set data name and unit for a specified data variable
//! @param [in] id This is the ENUM data id
//! @param [in,out] name The variable name
//! @param [in,out] unit The variable unit
void Node::setDataCharacteristics(size_t id, string name, string unit, Node::PLOTORNOT plotBehaviour)
{
    mDataNames[id] = name;
    mDataUnits[id] = unit;
    mPlotBehaviour[id] = plotBehaviour;
}


//! Get a specific data name
//! @param [in] id This is the ENUM data id
string Node::getDataName(size_t id)
{
    return mDataNames[id];
}


//! Get a specific data unit
//! @param [in] id This is the ENUM data id
string Node::getDataUnit(size_t id)
{
    return mDataUnits[id];
}


//! @brief This function gives you the data Id for a names data variable
//! @param [in] name The data name
//! @return The Id, -1 if requested data name is not found
int Node::getDataIdFromName(const string name)
{
    for (size_t i=0; i<mDataNames.size(); ++i)
    {
        if (name == mDataNames[i])
        {
            return i;
        }
    }
    return -1; //Did not find this name retunr -1 to signal failure
}


//! Get all data names, values and units
//! @param [in,out] rNames This vector will contain the names
//! @param [in,out] rValues This vector will contain the values
//! @param [in,out] rUnits This vector will contain the units
void Node::getDataNamesValuesAndUnits(vector<string> &rNames, std::vector<double> &rValues, vector<string> &rUnits)
{
    for(size_t i=0; i<mDataNames.size(); ++i)
    {
        if(mPlotBehaviour[i] == Node::PLOT)
        {
            rNames.push_back(mDataNames[i]);
            rValues.push_back(mDataVector[i]);
            rUnits.push_back(mDataUnits[i]);
        }
    }
}


bool Node::setDataValuesByNames(vector<string> names, std::vector<double> values)
{
    bool success = true;
    for(size_t i=0; i<names.size(); ++i)
    {
        this->setData(this->getDataIdFromName(names[i]),values[i]);
        //! @todo introduce setDataSafe and similar at many places in code
    }
    return success;
}


//! Get all data names and units
//! @param [in,out] rNames This vector will contain the names
//! @param [in,out] rUnits This vector will contain the units
void Node::getDataNamesAndUnits(vector<string> &rNames, vector<string> &rUnits)
{
    std::cout << "mDataNames.size(): " << mPlotBehaviour.size() << std::endl;
    for(size_t i=0; i<mDataNames.size(); ++i)
    {
        std::cout << "mPlotBehaviour.size(): " << mPlotBehaviour.size() << std::endl;
        if(mPlotBehaviour[i] == Node::PLOT)
        {
            rNames.push_back(mDataNames[i]);
            rUnits.push_back(mDataUnits[i]);
        }
    }
    //rNames = mDataNames;
    //rUnits = mDataUnits;
}


void Node::copyNodeVariables(Node *pNode)
{
    //this ska kopiera sina varabler till rNode
    if(pNode->getNodeType()==this->getNodeType())
    {
        for(size_t i=0; i<mDataNames.size(); ++i)
        {
            cout << "Name: " << mDataNames[i] << "  Value: " << mDataVector[i] << "  , " << pNode->mDataVector[i] << "  Unit: " << mDataUnits[i] << endl;
            if(mPlotBehaviour[i] == Node::PLOT)
            {
                //pNode->mDataNames[i] = mDataNames[i];
                pNode->mDataVector[i] = mDataVector[i];
                //pNode->mDataUnits[i] = mDataUnits[i];
            }
        }
        setSpecialStartValues(pNode); //Handles Wave, imp variables and similar
    }
    else
    {
        assert(false);
    }
}

void Node::setSpecialStartValues(Node */*pNode*/)
{
    //This method schould be implemented in child Nodes
    cout << "This nodetype seem not to have any hidden variables for the user." << endl;
}


//! This function will set the number of log data slots for preallocation and logDt based on the number of samples that should be loged
//! @param [in] nSamples The desired number of log data samples
void Node::setLogSettingsNSamples(size_t nSamples, double start, double stop, double sampletime)
{
    //make sure we dont try to log more samples than we will simulate
    //! @todo may need som rounding tricks here

    start = max(start, 0.0);  // Do not log data for negative time

    if ( ((stop - start) / sampletime) < nSamples )
    {
        mLogSlots = ((stop - start) / sampletime);
        std::stringstream ss;
        ss << "You requested nSamples: " << nSamples << ". This is more than total simulation samples, limiting to: " << mLogSlots;
        gCoreMessageHandler.addWarningMessage(ss.str(), "toofewsamples");
    }
    else
    {
        mLogSlots = nSamples;
    }

    mLogTimeDt = (stop - start) / (double)mLogSlots;
    mLastLogTime = start-mLogTimeDt;
}


//! This function will set the number of log data slots for preallocation and logDt based on a skip factor to the sample time
//! @param [in] factor The timestep skip factor
void Node::setLogSettingsSkipFactor(double factor, double start, double stop,  double sampletime)
{
    //! @todo maybe only use integer factors
    //make sure factor is not less then 1.0
    factor = max(1.0, factor);
    mLogTimeDt = sampletime * factor;
    mLastLogTime = start-mLogTimeDt;
    mLogSlots = (size_t)((stop-start)/mLogTimeDt+0.5); //Round to nearest
}


//! This function will set the number of log data slots for preallocation and logDt
//! @param [in] log_dt The desired log timestep
void Node::setLogSettingsSampleTime(double log_dt, double start, double stop,  double sampletime)
{
    //! @todo make sure that we dont have log_dt lower than sampletime ( we cant log more then we calc
    mLogTimeDt = log_dt;
    mLastLogTime = start-mLogTimeDt;
    mLogSlots = (size_t)((stop-start)/log_dt+0.5); //Round to nearest
}

//void Node::preAllocateLogSpace(const size_t nSlots)
//{
//    size_t data_size = mDataVector.size();
//    mTimeStorage.resize(nSlots);
//    mDataStorage.resize(nSlots, vector<double>(data_size));
//
//    cout << "requestedSize: " << nSlots << " " << data_size << " Capacities: " << mTimeStorage.capacity() << " " << mDataStorage.capacity() << " " << mDataStorage[1].capacity() << " Size: " << mTimeStorage.size() << " " << mDataStorage.size() << " " << mDataStorage[1].size() << endl;
//    mLogSpaceAllocated = true;
//
//    //Make sure the ctr is 0 if we simulate teh same model several times in a row
//    mLogCtr = 0;
//}

//! Pre allocate memory for the needed amount of log data
void Node::preAllocateLogSpace()
{
    size_t data_size = mDataVector.size();
    mTimeStorage.resize(mLogSlots);
    mDataStorage.resize(mLogSlots, vector<double>(data_size));

    cout << "requestedSize: " << mLogSlots << " " << data_size << " Capacities: " << mTimeStorage.capacity() << " " << mDataStorage.capacity() << " " << mDataStorage[1].capacity() << " Size: " << mTimeStorage.size() << " " << mDataStorage.size() << " " << mDataStorage[1].size() << endl;
    mLogSpaceAllocated = true;

    //Make sure the ctr is 0 if we simulate the same model several times in a row
    mLogCtr = 0;
}


//! Copy current data vector into log storage, also adds current time
void Node::logData(const double time)
{
    if (mDoLog)
    {
        //! @todo Danger comparing doubles
        //! @todo Mayb can use mLogTimeDt = -1 (or similar) instead of bool to avoid extra comparison
        //! @todo is this correct, Subtract a tenth of logDt to avoid numerical problem with double >= double
        if (time >= mLastLogTime+mLogTimeDt-mLogTimeDt/10.0)
        {
            if (mLogSpaceAllocated)
            {
                //cout << "mLogCtr: " << mLogCtr << endl;
                //! @todo this if check should not be needed if everything else is working
                if (mLogCtr < mTimeStorage.size())
                {
                    mTimeStorage[mLogCtr] = time;   //We log the "real"  simulation time for the sample
                    mDataStorage[mLogCtr] = mDataVector;
                }
                ++mLogCtr;
            }
            else
            {
                //! @todo for now always append
                mTimeStorage.push_back(time);   //We log the "real"  simulation time for the sample
                mDataStorage.push_back(mDataVector);
            }
            //mLastLogTime = time;
            mLastLogTime = mLastLogTime+mLogTimeDt; //Can not use "real" time directly as this may mean that not all log slots will be filled
        }
    }
}


//! @brief Returns a pointer to the component with the write port in the node.
//! If connection is ok, any node can only have one write port. If no write port exists, a null pointer is returned.
Component *Node::getWritePortComponentPtr()
{
    for(size_t i=0; i!=mPortPtrs.size(); ++i)
    {
        if(mPortPtrs.at(i)->getPortType() == WRITEPORT)
        {
            return mPortPtrs.at(i)->getComponent();
        }
    }

    return 0;   //Return null pointer if no write port was found
}


//! debug function to dump loged node data to a file
void Node::saveLogData(string filename)
{
    ofstream out_file;
    out_file.open(filename.c_str());

    if (out_file.good())
    {
        assert(mTimeStorage.size() == mDataStorage.size());
        //First write HEADER containing node type
        out_file << mNodeType << endl;
        //Write log data to file
        for (size_t row=0; row<mTimeStorage.size(); ++row)
        {
            out_file << mTimeStorage[row];
            for (size_t datacol=0; datacol<mDataVector.size(); ++datacol)
            {
                out_file << " " << mDataStorage[row][datacol];
            }
            out_file << endl;
        }
        out_file.close();
        cout << "Done! Saving node data to file" << endl;
    }
    else
    {
        cout << "Warning! Could not open out file for writing" << endl;
    }
}


//! Adds a pointer to a port connected to this node
//! @param [in] pPort The port pointer
void Node::setPort(Port *pPort)
{
    //Prevent duplicate port registration that can happen when "merging" nodes
    //The other code (connect) will be easier to write if we handle this in here
    bool found = false;
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
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
        mPortPtrs.push_back(pPort);
    }
}


//! Removes a port poniter from this node
//! @param [in] pPort The port pointer to be removed
void Node::removePort(Port *pPort)
{
    bool found = false;
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if (*it == pPort)
        {
            mPortPtrs.erase(it);
            found = true;
            break;
        }
    }

    if (!found)
    {
        cout << "Warning: you are trying to remove a Port that does not exist in this node  (does nothing)" << endl;
    }
}


//! Check if a specified port is connected to this node
//! @param [in] pPort The port pointer to find
//! @return Is specified port connected (true or false)
bool Node::isConnectedToPort(Port *pPort)
{
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if (*it == pPort)
        {
            return true;
        }
    }
    return false;
}


//! Enable node data logging
void Node::enableLog()
{
    mDoLog = true;
}


//! Disable node data logging
void Node::disableLog()
{
    mDoLog = false;
}


int Node::getNumberOfPortsByType(int type)
{
    assert(mPortPtrs.size() > 1);
    int n_Ports = 0;
    std::vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if ((*it)->getPortType() == type)
        {
            n_Ports++;
        }
    }
    return n_Ports;
}

ComponentSystem *Node::getOwnerSystem()
{
    return mpOwnerSystem;
}

//NodeFactory hopsan::gCoreNodeFactory;
//DLLIMPORTEXPORT NodeFactory* hopsan::getCoreNodeFactoryPtr()
//{
//    return &gCoreNodeFactory;
//}
