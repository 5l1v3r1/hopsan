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
//! @file   GUIContainerPort.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the ContainerPort class
//!
//$Id$

#include "global.h"
#include "GUIContainerPort.h"
#include "GUIContainerObject.h"
#include "GUIPort.h"
#include "Dialogs/ContainerPortPropertiesDialog.h"

ContainerPort::ContainerPort(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
        : ModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    // Sets the ports
    createPorts();
    refreshDisplayName();
}

void ContainerPort::deleteInHopsanCore()
{
    if (isSystemPort())
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->deleteSystemPort(this->getName());
    }
    else
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->unReserveUniqueName(this->getName());
    }
}

//! @brief Help function to create ports in the SystemPort Object when it is created
void ContainerPort::createPorts()
{
    //A system port only contains one port, which should be first in the map, ignore any others (should not be any more)
    PortAppearanceMapT::iterator i = mModelObjectAppearance.getPortAppearanceMap().begin();

    //Check if a systemport name is given in appearance data (for example if the systemport is loaded from file)
    //In that case override the default port name with this name
    QString desiredportname;
    if (mModelObjectAppearance.getDisplayName().isEmpty())
    {
        desiredportname = i.key();
    }
    else
    {
        desiredportname = mModelObjectAppearance.getDisplayName();
    }

    double x = i.value().x;
    double y = i.value().y;

    //! @todo should make this function select a systemport icon not undefined
    i.value().selectPortIcon("", "", "NodeEmpty");

    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Adding systemport with name: " << desiredportname;
    mName = mpParentContainerObject->getCoreSystemAccessPtr()->addSystemPort(desiredportname);
    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,resulting in name from core: " << mModelObjectAppearance.getDisplayName();
    mpPort = new Port(mName, x*boundingRect().width(), y*boundingRect().height(), &(i.value()), this);

    mPortListPtrs.append(mpPort);
    refreshDisplayName(); //Must run this after append port cause portname will also be refreshed
}


//! Returns a string with the GUIObject type.
QString ContainerPort::getTypeName() const
{
    return HOPSANGUICONTAINERPORTTYPENAME;
}

//! @brief Refreshes the displayed name from the actual name
//! @param [in] overrideName If this is non empty the name of the component in the gui will be forced to a specific value. DONT USE THIS UNLESS YOU HAVE TO
void ContainerPort::refreshDisplayName(QString overrideName)
{
    ModelObject::refreshDisplayName(overrideName);
    mPortListPtrs[0]->setDisplayName(mName);
}

//! @brief ContainerPorts shall only save their port name if they are systemports, if they are group ports no core data should be saved
void ContainerPort::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    if (isSystemPort())
    {
        rDomElement.setAttribute(HMF_NAMETAG, getName());
    }
}

//! @brief Opens the properties dialog
void ContainerPort::openPropertiesDialog()
{
    ContainerPortPropertiesDialog *pDialog = new ContainerPortPropertiesDialog(this, gpMainWindowWidget);
    pDialog->exec();
    delete pDialog;
}


//! @brief Event when double clicking on container port icon.
void ContainerPort::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);
    openPropertiesDialog();
}


int ContainerPort::type() const
{
    return Type;
}

QString ContainerPort::getHmfTagName() const
{
    return HMF_SYSTEMPORTTAG;
}

//! @brief Check if this is a system port (a container port belonging to a system)
bool ContainerPort::isSystemPort() const
{
    return (mpParentContainerObject->type() == SystemContainerType);
}
