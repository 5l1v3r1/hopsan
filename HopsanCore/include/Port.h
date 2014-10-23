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
//! @file   Port.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains Port base class as well as Sub classes
//!
//$Id$

#ifndef PORT_H_INCLUDED
#define PORT_H_INCLUDED

#include "Node.h"
#include "win32dll.h"
#include "HopsanCoreMacros.h"
#include "HopsanTypes.h"

namespace hopsan {

    // Forward declarations
    class Component;
    class ComponentSystem;
    class ConnectionAssistant;
    class MultiPort;

    //! @brief This enum type specifies all porttypes
    // It is VERY important that the MultiPort enums commes LAST, MULTIPORT is never instantiated but enum MUST be present
    enum PortTypesEnumT {UndefinedPortType, PowerPortType, ReadPortType, WritePortType, SystemPortType, MultiportType, PowerMultiportType, ReadMultiportType};

    class DLLIMPORTEXPORT Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;
        friend class AliasHandler;
        friend class MultiPort;

    public:
        //! @brief This enum specifies the RequiredConnection enums
        //! @todo remove the uppercase enums later when stable 0.6.0 release exist (to force update)
        enum RequireConnectionEnumT {Required, NotRequired, REQUIRED=Required, NOTREQUIRED=NotRequired};

        // Constructors - Destructors
        Port(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        virtual ~Port();

        //! @brief Reads a value from the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to read
        //! @param [in] subPortIdx Ignored on non multi ports
        //! @return The data value
        virtual inline double readNode(const size_t idx, const size_t subPortIdx=0) const
        {
            HOPSAN_UNUSED(subPortIdx)
            return mpNode->mDataValues[idx];
        }

        //! @brief Writes a value to the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to write
        //! @param [in] value The value of the data to read
        //! @param [in] subPortIdx Ignored on non multi ports
        virtual inline void writeNode(const size_t idx, const double value, const size_t subPortIdx=0)
        {
            HOPSAN_UNUSED(subPortIdx)
            mpNode->mDataValues[idx] = value;
        }

        virtual double readNodeSafe(const size_t idx, const size_t subPortIdx=0) const;
        virtual void writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx=0);

        virtual const Node *getNodePtr(const size_t subPortIdx=0)const;
        virtual double *getNodeDataPtr(const size_t idx, const size_t subPortIdx=0) const;
        virtual std::vector<double> *getDataVectorPtr(const size_t subPortIdx=0);

        virtual size_t getNumDataVariables() const;
        virtual const std::vector<NodeDataDescription>* getNodeDataDescriptions(const size_t subPortIdx=0);
        virtual const NodeDataDescription* getNodeDataDescription(const size_t dataid, const size_t subPortIdx=0) const;
        virtual int getNodeDataIdFromName(const HString &rName, const size_t subPortIdx=0);
        virtual void setSignalNodeUnitAndDescription(const HString &rUnit, const HString &rDescription);

        const HString &getVariableAlias(const int id);
        int getVariableIdByAlias(const HString &rAlias) const;

        virtual bool haveLogData(const size_t subPortIdx=0);
        virtual std::vector<double> *getLogTimeVectorPtr(const size_t subPortIdx=0);
        virtual std::vector<std::vector<double> > *getLogDataVectorPtr(const size_t subPortIdx=0);

        virtual bool isConnected() const;
        virtual bool isConnectedTo(Port *pOtherPort);
        bool isConnectionRequired();
        virtual std::vector<Port*> getConnectedPorts(const int subPortIdx=-1) const;
        size_t getNumConnectedPorts(const int subPortIdx=-1);
        virtual size_t getNumPorts();

        bool isInterfacePort() const;
        virtual bool isMultiPort() const;
        Port *getParentPort() const;
        const HString &getNodeType() const;
        virtual PortTypesEnumT getPortType() const;
        virtual PortTypesEnumT getExternalPortType();
        virtual PortTypesEnumT getInternalPortType();

        const HString &getName() const;
        const HString &getComponentName() const;
        const HString &getDescription() const;
        void setDescription(const HString &rDescription);

        virtual double getStartValue(const size_t idx, const size_t subPortIdx=0);
        virtual void loadStartValues();
        virtual void loadStartValuesFromSimulation();

        Component* getComponent() const;

        virtual Node *getNodePtr(const size_t subPortIdx=0);

    protected:
        HString mNodeType;
        Component* mpComponent;
        Port* mpParentPort;

        std::vector<Port*> mConnectedPorts;

        virtual void setDefaultStartValue(const size_t idx, const double value, const size_t subPortIdx=0);
        virtual void disableStartValue(const size_t idx);

        virtual Node *getStartNodePtr();
        virtual void setNode(Node* pNode);

        virtual Port* addSubPort();
        virtual void removeSubPort(Port* pPort);

        void registerStartValueParameters();
        void unRegisterStartValueParameters();

        void addConnectedPort(Port* pPort, const size_t subPortIdx=0);
        void eraseConnectedPort(Port* pPort, const size_t subPortIdx=0);

        void createStartNode(const HString &rNodeType);
        void eraseStartNode();

        void setVariableAlias(const HString &rAlias, const int id);

    private:
        HString mPortName;
        HString mDescription;
        Node *mpNode;
        Node *mpStartNode;
        std::map<HString, int> mVariableAliasMap;
        bool mConnectionRequired;

        const HString mEmptyString;
    };


    class SystemPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        SystemPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);

        PortTypesEnumT getPortType() const;
        PortTypesEnumT getExternalPortType();
        PortTypesEnumT getInternalPortType();
    };


    class MultiPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        MultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        ~MultiPort();

        // Overloaded virtual functions
        virtual PortTypesEnumT getPortType() const;
        bool isMultiPort() const;
        double readNodeSafe(const size_t idx, const size_t subPortIdx) const;
        void writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx);

        //! @brief Reads a value from the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to read
        //! @param [in] subPortIdx The subPort to read from (range is NOT checked)
        //! @return The data value
        inline double readNode(const size_t idx, const size_t subPortIdx) const
        {
            return mSubPortsVector[subPortIdx]->readNode(idx);
        }

        //! @brief Writes a value to the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to write
        //! @param [in] value The value of the data to read
        //! @param [in] subPortIdx The subPort to write to (range is NOT checked)
        inline void writeNode(const size_t idx, const double value, const size_t subPortIdx)
        {
            return mSubPortsVector[subPortIdx]->writeNode(idx,value);
        }

        const Node *getNodePtr(const size_t subPortIdx=0)const;
        double *getNodeDataPtr(const size_t idx, const size_t subPortIdx) const;
        std::vector<double> *getDataVectorPtr(const size_t subPortIdx=0);

        const std::vector<NodeDataDescription>* getNodeDataDescriptions(const size_t subPortIdx=0);
        const NodeDataDescription* getNodeDataDescription(const size_t dataid, const size_t subPortIdx=0) const;
        int getNodeDataIdFromName(const HString &rName, const size_t subPortIdx=0);

        bool haveLogData(const size_t subPortIdx=0);
        std::vector<double> *getLogTimeVectorPtr(const size_t subPortIdx=0);
        std::vector<std::vector<double> > *getLogDataVectorPtr(const size_t subPortIdx=0);

        double getStartValue(const size_t idx, const size_t subPortIdx=0);

        void loadStartValues();
        void loadStartValuesFromSimulation();

        virtual bool isConnectedTo(Port *pOtherPort);
        bool isConnected() const;
        size_t getNumPorts();

        std::vector<Port*> getConnectedPorts(const int subPortIdx=-1) const;

    protected:
        void setNode(Node* pNode);
        Node *getNodePtr(const size_t subPortIdx=0);
        void removeSubPort(Port* ptr);

        std::vector<Port*> mSubPortsVector;
    };


    class PowerPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        PowerPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const;
    };


    class ReadPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        ReadPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const;

        void writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx=0);
        void writeNode(const size_t idx, const double value, const size_t subPortIdx=0);

        virtual void loadStartValues();
        //virtual bool hasConnectedExternalSystemWritePort();
        virtual bool isConnectedToWriteOrPowerPort();
        virtual void forceLoadStartValue();
    };

    class PowerMultiPort :public MultiPort
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        PowerMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const;

    protected:
        Port* addSubPort();
    };

    class ReadMultiPort :public MultiPort
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        ReadMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const;

    protected:
        Port* addSubPort();
    };


    class WritePort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        WritePort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const;
    };

    Port* createPort(const PortTypesEnumT portType, const HString &rNodeType, const HString &rName, Component *pParentComponent, Port *pParentPort=0);
    HString DLLIMPORTEXPORT portTypeToString(const PortTypesEnumT type);
}

#endif // PORT_H_INCLUDED
