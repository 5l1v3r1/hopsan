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
//! @file   GUIPort.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPort class
//!
//$Id$

#include <QInputDialog>
#include <QSvgRenderer>

#include "common.h"
#include "global.h"
#include "GUIPort.h"
#include "GUIConnector.h"

#include "PlotWindow.h"
#include "CoreAccess.h"
#include "GraphicsView.h"
#include "Configuration.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/ModelWidget.h"
#include "MessageHandler.h"


QPointF getOffsetPointfromPort(Port *pStartPort, Port *pEndPort)
{
    QPointF point;

    if((pEndPort->getPortDirection() == LeftRightDirectionType) && (pEndPort->getParentModelObject()->mapToScene(pEndPort->getParentModelObject()->boundingRect().center()).x() > pEndPort->scenePos().x()))
    {
        point.setX(-1 * std::min(20.0, abs(pStartPort->scenePos().x()-pEndPort->scenePos().x())/2.0));
    }
    else if((pEndPort->getPortDirection() == LeftRightDirectionType) && (pEndPort->getParentModelObject()->mapToScene(pEndPort->getParentModelObject()->boundingRect().center()).x() < pEndPort->scenePos().x()))
    {
        point.setX(std::min(20.0, abs(pStartPort->scenePos().x()-pEndPort->scenePos().x())/2.0));
    }
    else if((pEndPort->getPortDirection() == TopBottomDirectionType) && (pEndPort->getParentModelObject()->mapToScene(pEndPort->getParentModelObject()->boundingRect().center()).y() > pEndPort->scenePos().y()))
    {
        point.setY(-1 * std::min(20.0, abs(pStartPort->scenePos().y()-pEndPort->scenePos().y())/2.0));
    }
    else if((pEndPort->getPortDirection() == TopBottomDirectionType) && (pEndPort->getParentModelObject()->mapToScene(pEndPort->getParentModelObject()->boundingRect().center()).y() < pEndPort->scenePos().y()))
    {
        point.setY(std::min(20.0, abs(pStartPort->scenePos().y()-pEndPort->scenePos().y())/2.0));
    }
    return point;
}

//! Constructor.
//! @param portName The name of the port
//! @param x the x-coord. of where the port should be placed.
//! @param y the y-coord. of where the port should be placed.
//! @param rot how the port should be rotated.
//! @param QString(ICONPATH) a string with the path to the svg-figure representing the port.
//! @param parent the port's parent, the component it is a part of.
Port::Port(QString portName, double xpos, double ypos, PortAppearance* pPortAppearance, ModelObject *pParentGUIModelObject)
    : QGraphicsWidget(pParentGUIModelObject)
{
//    qDebug() << "parentType: " << pParentGUIModelObject->type() << " GUISYSTEM=" << GUISYSTEM << " GUICONTAINER=" << GUICONTAINEROBJECT;
//    qDebug() << "======================= parentName: " << pParentGUIModelObject->getName();
    mpParentModelObject = pParentGUIModelObject;
    mpPortAppearance = pPortAppearance;
    mPortDisplayName = portName;

    mpMultiPortIconOverlay = 0;
    mpCQSIconOverlay = 0;
    mpMainIcon = 0;

    //Set default magnification
    mMag = GOLDENRATIO;
    mOverlaySetScale = 1.0;
    mIsMagnified = false;

    refreshPortMainGraphics();
    //qDebug() << "---getPos: " << this->pos();

    //Now adjust position
    setCenterPos(xpos, ypos);
    //qDebug() << "---getPos: " << this->pos();

    //Setup port label (ports allways have lables)
    mpPortLabel = new QGraphicsTextItem(this);
    mpPortLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    mpPortLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    mpPortLabel->setZValue(PortLabelZValue); //High value should be on top of everything
    mpPortLabel->hide();
    //Port label must exist and be set up before we run setDisplayName
    setDisplayName(mPortDisplayName);

    //Setup overlay (if it exists)
    refreshPortOverlayGraphics();
    refreshPortOverlayScale(mpParentModelObject->getParentContainerObject()->mpModelWidget->getGraphicsView()->getZoomFactor());

    mPortAppearanceAfterLastRefresh = *mpPortAppearance; //Remember current appearance

    setAcceptHoverEvents(true);

    // Determine if the port should be shown or not
    if( getParentContainerObject() != 0 )
    {
        showIfNotConnected( getParentContainerObject()->areSubComponentPortsShown() );
    }

    // Create signal connection to the zoom change signal for port overlay scaling and port hide/show function
    GraphicsView *pView = getParentContainerObject()->mpModelWidget->getGraphicsView(); //!< @todo need to be able to access this in some nicer way then ptr madness, also in aother places
    connect(getParentContainerObject(),  SIGNAL(showOrHideAllSubComponentPorts(bool)),   this,   SLOT(showIfNotConnected(bool)),         Qt::UniqueConnection);
    connect(mpParentModelObject,         SIGNAL(visibleChanged()),                       this,   SLOT(showIfNotConnected()),             Qt::UniqueConnection);
    connect(pView,                       SIGNAL(zoomChange(double)),                     this,   SLOT(refreshPortOverlayScale(double)),   Qt::UniqueConnection);
}

Port::~Port()
{
    //! @todo Hack, we need to mark this as 0 to avoid graphics refresh triggerd from connector deleteMe in case of subsystemport (bellow), The interaction between ports and connectors should be rewritten in a smarter way
    mpPortAppearance = 0; //

    //If any connectors are present they need to be deleated also
    // We need to use while and access first element every time as Vector will be modified when connector removes itself
    //! @todo What about Undo, right now these deleations are not registered
    disconnectAndRemoveAllConnectedConnectors();
    //! @todo Maybe we should use signal and slots instead to handle connector removal on port delete, we are doing that with GuiModelObjects right now

    //We dont need to disconnect the permanent connection to the mainwindow buttons and the view zoom change signal for port overlay scaleing
    //They should be disconnected automatically when the objects die
}

//! Magnify the port with a class mebmer factor 'mMag'. Is used i.e. at hovering over disconnected port.
//! @param blowup says if the port should be magnified or not.
void Port::magnify(bool doMagnify)
{
    if( doMagnify && !mIsMagnified )
    {
        this->setScale(this->scale()*mMag);

        mIsMagnified = true; //This must be set before setPortOverlayScale
        this->refreshPortOverlayScale(mOverlaySetScale); //Set the scale to the same as current but this time the extra scale factor will be added
    }
    else if ( !doMagnify && mIsMagnified )
    {

        this->setScale(this->scale()*1.0/mMag);

        mIsMagnified = false; //This must be set before setPortOverlayScale
        this->refreshPortOverlayScale(mOverlaySetScale); //Set the scale to the same as current, no extra magnification will be added
    }
}


//! Reimplemented to call custom show hide instead
void Port::setVisible(bool value)
{
    if (value && mpPortAppearance->mEnabled)
    {
        this->show();
    }
    else
    {
        this->hide();
    }
}


//! Defines what happens when mouse cursor begins to hover a port.
//! @param *event defines the mouse event.
void Port::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (mpPortAppearance->mEnabled)
    {
        QGraphicsWidget::hoverEnterEvent(event);

        this->setCursor(Qt::CrossCursor);

        magnify(true);
        this->setZValue(HoveredPortZValue);

        refreshPortLabelText();
        mpPortLabel->show();
    }
}

//! Defines what happens when mouse cursor stops hovering a port.
//! @param *event defines the mouse event.
void Port::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (mpPortAppearance->mEnabled)
    {
        this->setCursor(Qt::ArrowCursor);
        magnify(false);

        mpPortLabel->hide();
        this->setZValue(PortZValue);

        QGraphicsWidget::hoverLeaveEvent(event);
    }
}

//! @brief Updates the gui port position on its parent object, taking coordinates in parent coordinate system
//! @todo is it necessary to be able to update orientation also?
void Port::setCenterPos(const double x, const double y)
{
    //Place the guiport with center in x and y, in parent local coordinates
//    QPointF posdiff = QPointF(x,y) - this->mapToParent(this->boundingRect().center());
//    this->moveBy(posdiff.x(), posdiff.y());// setPos(this->pos()+posdiff);
    setPos(x,y);
}

//! @brief Conveniance function when fraction positions are known
void Port::setCenterPosByFraction(double x, double y)
{
    //! @todo for now root systems may not have an icon, if icon is empty ports will end up in zero, which is OK, maybe we should always force a default icon
    this->setCenterPos(x*mpParentModelObject->boundingRect().width(), y*mpParentModelObject->boundingRect().height());
}

//! @brief Returns the center position in parent coordinates
QPointF Port::getCenterPos()
{
    //return this->mapToParent(this->boundingRect().center());
    return pos();
}

//! @brief Overloaded setRotation to store rotation in appearance data
void Port::setRotation(double angle)
{
    mpPortAppearance->rot = angle;
    QGraphicsWidget::setRotation(angle);
}

//! @brief Defines what happens when clicking on a port.
//! @param *event defines the mouse event.
void Port::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mpPortAppearance->mEnabled)
    {
        if(!mpParentModelObject->getParentContainerObject()->mpModelWidget->isEditingEnabled())
            return;

        //QGraphicsSvgItem::mousePressEvent(event); //Don't work if this is called
        if (event->button() == Qt::LeftButton)
        {
            getParentContainerObject()->createConnector(this);
        }
        else if (event->button() == Qt::RightButton)
        {
            //Nothing
        }
    }
}


//! @brief Defines what happens when double clicking on a port. Nothing should happen.
//! @param *event defines the mouse event.
void Port::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
{
    //Nothing to do, reimplemented just to do nothing
}


void Port::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (mpPortAppearance->mEnabled)
    {
        // Prevent the user from plotting MULTIPORTS.
        if(getPortType() == QString("PowerMultiportType"))
        {
            QMenu menu;
            QAction *action;
            action = menu.addAction(QString("Cannot plot MultiPorts"));
            action->setDisabled(true);
            menu.exec(event->screenPos());
        }
        else
        {
            openRightClickMenu(event->screenPos());
        }
    }
}


void Port::openRightClickMenu(QPoint screenPos)
{
    QMenu menu;

    // First build alias menue
    QMenu *pAliasMenu = new QMenu("Define Alias");
    QMap<QAction*, int> aliasActions;
    QVector<CoreVariameterDescription> variameterDescriptions;
    mpParentModelObject->getVariameterDescriptions(variameterDescriptions); //!< @todo would be nice to be able to get this info only for a particular port
    for (int i=0; i<variameterDescriptions.size(); ++i)
    {
        if (variameterDescriptions[i].mPortName == this->getName())
        {
            if (gpConfig->getShowHiddenNodeDataVariables() || (variameterDescriptions[i].mVariabelType != "Hidden"))
            {
                QAction *pAliasAction;
                if (variameterDescriptions[i].mAlias.isEmpty())
                {
                    pAliasAction = pAliasMenu->addAction(variameterDescriptions[i].mName);
                }
                else
                {
                    pAliasAction = pAliasMenu->addAction(variameterDescriptions[i].mName+" = "+variameterDescriptions[i].mAlias);
                }
                aliasActions.insert(pAliasAction,i);
            }
        }
    }
    menu.addMenu(pAliasMenu);
    menu.addSeparator();

    // Now build plot menue
    QMap<QAction*, int> plotActions;

    QVector<SharedVectorVariableT> logVars = mpParentModelObject->getParentContainerObject()->getLogDataHandler()->getMatchingVariablesAtGeneration(QRegExp(makeConcatName(mpParentModelObject->getName(),this->getName(),".*")));
    for(int i=0; i<logVars.size(); ++i)
    {
        QAction *pTempAction;
        //! @todo This is a ugly special hack for Signal Value but I cant thing of anything better, (maye make it impossible to have custom plotscales for Values)
        //! @todo should have a help function for this as similar checks are done elsewere
        const QString &dataName = logVars[i]->getDataName();
        const QString &dataUnit = logVars[i]->getDataUnit();

        QString displayUnit;
        UnitScale custUS;
        mpParentModelObject->getCustomPlotUnitOrScale(this->getName()+"#"+dataName, custUS);
        if (custUS.isEmpty())
        {
            if ( dataName == "Value" && dataUnit != "-")
            {
                QStringList pqs = gpConfig->getPhysicalQuantitiesForUnit(dataUnit);
                //! @todo if same unit exist in multiple places we have a problem
                if (pqs.size() > 1)
                {
                    gpMessageHandler->addWarningMessage(QString("Unit %1 is associated to multiple physical quantities, default unit selection may be incorrect").arg(dataUnit));
                }
                QString defaultUnit;
                if (pqs.size() == 1)
                {
                    defaultUnit = gpConfig->getDefaultUnit(pqs.first());
                }

                if (!defaultUnit.isEmpty())
                {
                    displayUnit = defaultUnit;
                }
                else
                {
                    displayUnit = dataUnit;
                }
            }
            else
            {
                displayUnit = gpConfig->getDefaultUnit(dataName);
            }
        }
        else
        {
            displayUnit = custUS.mUnit;
        }

        QString actionText;
        if (logVars[i]->hasAliasName())
        {
            actionText = QString("Plot %1 (%2) [%3]").arg(dataName).arg(logVars[i]->getAliasName()).arg(displayUnit);
        }
        else
        {
            actionText = QString("Plot %1 [%2]").arg(dataName).arg(displayUnit);;
        }
        pTempAction = menu.addAction(actionText);
        plotActions.insert(pTempAction, i);
    }

    // Execute menue and then check selected action
    QAction *selectedAction = menu.exec(screenPos);
    // Check for alias action
    QMap<QAction*, int>::iterator it = aliasActions.find(selectedAction);
    if (it != aliasActions.end())
    {
        openDefineAliasDialog(variameterDescriptions[it.value()].mName, variameterDescriptions[it.value()].mAlias);
    }
    else
    {
        // Check for plot action
        it = plotActions.find(selectedAction);
        if (it != plotActions.end())
        {
            plot(logVars[it.value()]->getDataName(), "");
        }
    }
}


void Port::openDefineAliasDialog(const QString &rVarName, const QString &rCurrentAlias)
{
    bool ok;
    QString fullName = makeConcatName(mpParentModelObject->getName(),this->getName(),rVarName);
    QString alias = QInputDialog::getText(gpMainWindowWidget, "Define Alias",
                                          tr("Alias for: ")+fullName, QLineEdit::Normal,
                                          rCurrentAlias, &ok);
    if(ok)
    {
        //! @todo should not go through logdatahnadler for this, should access directly, and signal the alias change to logdata handler
        getParentContainerObject()->getLogDataHandler()->defineAlias(alias, fullName);
    }
}


void Port::moveEvent(QGraphicsSceneMoveEvent *event)
{
    Q_UNUSED(event);
    double px = mpParentModelObject->boundingRect().width();
    double py = mpParentModelObject->boundingRect().height();

    mpPortAppearance->x = this->pos().x()/px;
    mpPortAppearance->y = this->pos().y()/py;
}


void Port::refreshPortMainGraphics()
{
    double rotAng = mpPortAppearance->rot; // OK, uggly, but has to be done in case the mpMainIcon is reset below
    if (mPortAppearanceAfterLastRefresh.mMainIconPath != mpPortAppearance->mMainIconPath)
    {
        if (mpMainIcon != 0)
        {
            // We must restore original angle and translation before deleting and applying new main graphis, i know it sucks, but we have to
            setRotation(0);
            setTransform(QTransform::fromTranslate(mpMainIcon->boundingRect().center().x(), mpMainIcon->boundingRect().center().y()), true);
            mpMainIcon->deleteLater();
        }

        prepareGeometryChange();
        mpMainIcon = new QGraphicsSvgItem(mpPortAppearance->mMainIconPath, this);
        resize(mpMainIcon->boundingRect().width(), mpMainIcon->boundingRect().height());
        //qDebug() << "_______diff: " << -mpMainIcon->boundingRect().center();
        setTransform(QTransform::fromTranslate(-mpMainIcon->boundingRect().center().x(), -mpMainIcon->boundingRect().center().y()), true);
    }
    // Always refresh rotation
    setTransformOriginPoint(boundingRect().center()); //All rotaion and other transformation should be aplied around the port center
    setRotation(rotAng); // This will also reset mpPortAppearance->rot
}


void Port::refreshPortOverlayGraphics()
{
    //! @todo maybe put main icon in here also

    //Ok if new appearance is different from previous, then remove old graphics and replace with new one
    //Check CQS overlay
    if (mpPortAppearance->mCQSOverlayPath != mPortAppearanceAfterLastRefresh.mCQSOverlayPath)
    {
        if (mpCQSIconOverlay != 0)
        {
            mpCQSIconOverlay->deleteLater();
        }

        if (mpPortAppearance->mCQSOverlayPath.isEmpty())
        {
            mpCQSIconOverlay = 0;
        }
        else
        {
            //! @todo check if file exist
            mpCQSIconOverlay = new QGraphicsSvgItem(mpPortAppearance->mCQSOverlayPath, this);
            mpCQSIconOverlay->setZValue(CQSOverlayZValue);
            mpCQSIconOverlay->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        }
    }

    //Check multiport overlay
    if (mpPortAppearance->mMultiPortOverlayPath != mPortAppearanceAfterLastRefresh.mMultiPortOverlayPath)
    {
        if (mpMultiPortIconOverlay != 0)
        {
            mpMultiPortIconOverlay->deleteLater();
        }

        if (mpPortAppearance->mMultiPortOverlayPath.isEmpty())
        {
            mpMultiPortIconOverlay = 0;
        }
        else
        {
            //! @todo check if file exist
            mpMultiPortIconOverlay = new QGraphicsSvgItem(mpPortAppearance->mMultiPortOverlayPath, this);
            mpMultiPortIconOverlay->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
            mpMultiPortIconOverlay->setZValue(MultiportOverlayZValue);
        }
    }

    this->refreshPortOverlayPosition();
    this->refreshPortOverlayScale(mOverlaySetScale);

    //Port label must exist and be set up before we run setDisplayName
    this->setDisplayName(this->getName());
}


//! @brief Refreshes the port overlay graphics and lable position
void Port::refreshPortOverlayPosition()
{
    this->magnify(false); //We must turn magnification off or the pos adjustment calculation will be massivly brainfucked

    //Refresh the port overlay graphics positions
    if (mpCQSIconOverlay != 0)
    {

        //    QPen rpen(QColor("red"));
        //    QPen gpen(QColor("green"));
        //    QPen bpen(QColor("black"));
        //    QPen ypen(QColor("yellow"));
        //    rpen.setWidthF(2.0);
        //    gpen.setWidthF(1.5);
        //    bpen.setWidthF(1.0);
        //    ypen.setWidthF(0.5);
        //if(this->scene() != 0)
        //{
        //    this->scene()->addRect(this->sceneBoundingRect(), rpen);
        //    this->scene()->addRect(QRectF(mpCQSIconOverlay->parentItem()->mapToScene(mpCQSIconOverlay->pos()), mpCQSIconOverlay->boundingRect().size() ), gpen);

        //    //QPointF pt1 = mpParentGuiModelObject->mapToScene(this->getCenterPos());
        //    //QPointF pt2 = mpCQSIconOverlay->mapToScene(mpCQSIconOverlay->boundingRect().center());

        //    //QPointF pt1 = mpParentGuiModelObject->mapToScene(this->pos());
        //    //QPointF pt2 = this->mapToScene(mpCQSIconOverlay->pos());

        //    QPointF pt1 = this->parentItem()->mapToScene(this->pos());
        //    QPointF pt2 = mpCQSIconOverlay->parentItem()->mapToScene(mpCQSIconOverlay->pos());

        //    this->scene()->addEllipse(pt1.x(), pt1.y(), 1, 1, rpen);
        //    this->scene()->addEllipse(pt2.x(), pt2.y(), 1, 1, gpen);
        //}

        // We take the port center and transform to scene, here we subtract half the overlay size to align overlay center with port center then we transform back to port coordinate system
        //mpCQSIconOverlay->setPos( this->mapFromScene( mpParentGuiModelObject->mapToScene(this->getCenterPos()) - mpCQSIconOverlay->boundingRect().center() ) );
        QPointF diff(mpCQSIconOverlay->boundingRect().width()/2, mpCQSIconOverlay->boundingRect().height()/2);
        mpCQSIconOverlay->setPos(this->mapFromScene(this->parentItem()->mapToScene(this->pos())-diff));
    }
    if (mpMultiPortIconOverlay != 0)
    {
        // We take the port center and transform to scene, here we subtract half the overlay size to align overlay center with port center then we transform back to port coordinate system
        //mpMultiPortIconOverlay->setPos( this->mapFromScene( mpParentGuiModelObject->mapToScene(this->getCenterPos()) - mpMultiPortIconOverlay->boundingRect().center() ) );
        QPointF diff(mpMultiPortIconOverlay->boundingRect().width()/2, mpMultiPortIconOverlay->boundingRect().height()/2);
        mpMultiPortIconOverlay->setPos(this->mapFromScene(this->parentItem()->mapToScene(this->pos())-diff));
    }

    //! @todo Do we even need to recalculate lable pos every time, the offset will never change and lable should move with port anyway
    //We take the ports bounding rect center in scene coordinates and add a predetermined offset, then transform back to port coordinate system
    this->mpPortLabel->setPos( this->mapFromScene( this->sceneBoundingRect().center() + QPointF(10, 10) ) );
}


//! @brief recreate the port graphics overlay
//! @todo This needs to be synced and clean up with addPortOverlayGraphics, right now duplicate work, also should not change if icon same as before
//! @todo Maybe we should preload all cqs and multiport overlays (just three of them) and create som kind of shared svg renderer that all ports can share, then we dont need to reload graphics from file every freaking time it changes and in every systemport
void Port::refreshPortGraphics()
{
    qDebug() << "!!! REFRESHING PORT GRAPHICS !!!";

    //! @todo maybe we should make the ports own their own appearance instead of their parent object
    if (mpPortAppearance != 0)
    {
        //Systemports may change appearance depending on what is connected
        CoreSystemAccess::PortTypeIndicatorT int_ext_act = CoreSystemAccess::ActualPortType;
        QString cqsType;
        if (getPortType() == "SystemPortType")
        {
            if (getParentModelObject()->getTypeName() == HOPSANGUICONTAINERPORTTYPENAME)
            {
                int_ext_act = CoreSystemAccess::ExternalPortType;

                //If we are port in containerport model object then ask our parent system model object about cqs-type
                cqsType = getParentContainerObject()->getTypeCQS();

                //Dont show cqs typ internally, it will become confusing, only show question marks if undefined
                if (cqsType != "UndefinedCQSType")
                {
                    cqsType = "NULL";
                }
            }
            else
            {
                int_ext_act = CoreSystemAccess::InternalPortType;

                //If we are external systemport then ask our model object about the cqs-type
                cqsType = getParentModelObject()->getTypeCQS();
                qDebug() << "cqsType: " << cqsType;
            }
        }
        else
        {
            cqsType = getParentModelObject()->getTypeCQS();
        }
        mpPortAppearance->selectPortIcon(cqsType, getPortType(int_ext_act), getNodeType());

        refreshPortMainGraphics();
        refreshPortOverlayGraphics();

        //Remember current appearance so that we can check what has changed the next time, (to avoid reloading graphics)
        mPortAppearanceAfterLastRefresh = *mpPortAppearance;

        //Make sure connected connectors are refreshed as well
        for(int i=0; i<mConnectedConnectors.size(); ++i)
        {
            mConnectedConnectors[i]->drawConnector();
        }

        //! @todo maybe we could check this first and skip creating all of the graphics if it is not going to be shown
        this->setEnable(mpPortAppearance->mEnabled);
    }
}

//! @brief Scales the port overlay graphics and tooltip
void Port::refreshPortOverlayScale(double scale)
{
    mOverlaySetScale = scale;   //Remember what scale we are supposed to have

    //Should we add extra overlay scale when magnified
    double overlayExtraScaleFactor;
    if (mIsMagnified)
    {
        overlayExtraScaleFactor = mMag;
    }
    else
    {
        overlayExtraScaleFactor = 1.0;
    }

    if (mpCQSIconOverlay != 0)
    {
        mpCQSIconOverlay->setScale(mOverlaySetScale * overlayExtraScaleFactor);
    }
    if (mpMultiPortIconOverlay != 0)
    {
        mpMultiPortIconOverlay->setScale(mOverlaySetScale * overlayExtraScaleFactor);
    }
}

//! @brief Updates the port lable text
void Port::refreshPortLabelText()
{
    //Set the lable, &#160 means extra white space, to make lable more readable
    QString label("<p><span style=\"background-color:lightyellow;font-size:14px;text-align:center\">&#160;&#160;");

    label.append(mPortDisplayName).append("&#160;&#160;");

    //! @todo should get portdescription once and store it instead of getting it every time (search in core)
    //! @todo we should show unit here aswell (for signal ports)
    QString desc = getPortDescription();
    if (desc.isEmpty())
    {
        // backwards compatible
        if (!mpPortAppearance->mDescription.isEmpty())
        {
            gpMessageHandler->addWarningMessage("You seem to have entered a port description in the xml file, this description should be in the componnet code instead. In the addPort function call.");
            //Append description
           desc = mpPortAppearance->mDescription;
           label.append("<br>\"" + desc + "\"");
        }
    }
    else
    {
        label.append("<br>\"" + desc + "\"");;
    }
    label.append("</span></p>");

    // Build the variable / alias list
    QMap<QString, QString> var_alias = getParentModelObject()->getVariableAliases(this->getName());
    if (!var_alias.isEmpty())
    {
        label.append("<table style=\"background-color:lightyellow;font-size:12px\">");
        label.append("<tr><th>Name</th><th width=\"12\"></th><th>Alias</th></tr>");
        QMap<QString, QString>::iterator it;
        for (it=var_alias.begin(); it!=var_alias.end(); ++it)
        {
            label.append("<tr><td align=center>");
            label.append(it.key());
            label.append("</td><td></td><td align=center>");
            label.append(it.value());
            label.append("</td></tr>");
        }
        label.append("</table>");
    }


    mpPortLabel->setHtml(label);
    mpPortLabel->adjustSize();
}

//! @brief Returns a pointer to the GraphicsView that the port belongs to.
ContainerObject *Port::getParentContainerObject()
{
    return mpParentModelObject->getParentContainerObject();
}


//! @brief Returns a pointer to the GUIComponent the port belongs to.
ModelObject *Port::getParentModelObject()
{
    return mpParentModelObject;
}


//! @brief Plots the variable with name 'dataName' in the node the port is connected to.
//! @param dataName tells which variable to plot.
//! @param dataUnit sets the unit to show in the plot (has no connection to data, just text).
PlotWindow *Port::plot(QString dataName, QString /*dataUnit*/, QColor desiredCurveColor)
{
    QString fullName = makeConcatName(mpParentModelObject->getName(),this->getName(),dataName);
    //! @todo  why do we have unit here
    return getParentContainerObject()->getLogDataHandler()->plotVariable(0, fullName, -1, 0, desiredCurveColor);
}

//! @brief Wrapper for the Core getPortTypeString() function
QString Port::getPortType(const CoreSystemAccess::PortTypeIndicatorT ind)
{
    return getParentContainerObject()->getCoreSystemAccessPtr()->getPortType(getParentModelObjectName(), this->getName(), ind);
}


//! @brief Wrapper for the Core getNodeType() function
QString Port::getNodeType()
{
    return getParentContainerObject()->getCoreSystemAccessPtr()->getNodeType(getParentModelObjectName(), this->getName());
}

QString Port::getPortDescription()
{
    return getParentContainerObject()->getCoreSystemAccessPtr()->getPortDescription(getParentModelObjectName(), this->getName());
}

QStringList Port::getVariableNames()
{
    QVector<QString> names, units;
    getParentContainerObject()->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(getParentModelObjectName(), getName(), names, units);
    return QStringList::fromVector(names);
}

QStringList Port::getFullVariableNames()
{
    QStringList names = getVariableNames();
    QStringList names2;
    Q_FOREACH(QString name, names)
    {
        names2.append(makeConcatName(getParentModelObjectName(), getName(), name));
    }
    return names2;
}


PortDirectionT Port::getPortDirection()
{
    //! @todo will this work if parentguimodelobject is flipped
    double scene_angle = normDeg360( this->mpParentModelObject->rotation() + this->rotation() );

    if ( fuzzyEqual(scene_angle, 0, 1.0) || fuzzyEqual(scene_angle, 180, 1.0) )
    {
        //qDebug() << "Returning LEFTRIGHT";
        return LeftRightDirectionType;
    }
    else
    {
        //qDebug() << "Returning TOPBOTTOM";
        return TopBottomDirectionType;
    }
}


void Port::rememberConnection(Connector *pConnector)
{
    mConnectedConnectors.append(pConnector);
    getParentModelObject()->rememberConnector(pConnector);
    //qDebug() << "Adding connection, connections = " << mnConnections;

    // Refresh port graphics if it is a system port
    if (getPortType() == "SystemPortType")
    {
        refreshPortGraphics();
    }
}


void Port::forgetConnection(Connector *pConnector)
{
    int idx = mConnectedConnectors.indexOf(pConnector);
    mConnectedConnectors.remove(idx);
    getParentModelObject()->forgetConnector(pConnector);

    // Refresh port graphics if it is a system port
    if (getPortType() == "SystemPortType")
    {
        refreshPortGraphics();
    }

    if(!isConnected())
    {
        setVisible(getParentContainerObject()->areSubComponentPortsShown());
    }

    //qDebug() << "Removing connection, connections = " << mnConnections;
}

//! @brief Convenienc function to dissconnect and remove all conected connectors, usefull to call before deleteing ports
//! @note The main reason for this function is that connector port relasionship during delete is MADNESS
//! @note No undo will be registred
void Port::disconnectAndRemoveAllConnectedConnectors()
{
    while (mConnectedConnectors.size() > 0)
    {
        mConnectedConnectors[0]->deleteMeWithNoUndo();
    }
}

//! @brief Return a copy of the currently connected connectors
//! @return QVector with connector pointers
QVector<Connector*> Port::getAttachedConnectorPtrs() const
{
    return mConnectedConnectors;
}


//! @brief Ask if the port is connected or not
//! @return if the port is connected or not
bool Port::isConnected()
{
    return (mConnectedConnectors.size() > 0);
}


//! @brief Returns a vector with all ports connected to this port.
QVector<Port *> Port::getConnectedPorts()
{
    QVector<Port *> vector;
    for(int i=0; i<mConnectedConnectors.size(); ++i)
    {
        if(mConnectedConnectors.at(i)->getStartPort() == this)
            vector.append(mConnectedConnectors.at(i)->getEndPort());
        else
            vector.append(mConnectedConnectors.at(i)->getStartPort());
    }
    return vector;
}


bool Port::isAutoPlaced()
{
    return mpPortAppearance->mAutoPlaced;
}

const PortAppearance *Port::getPortAppearance() const
{
    return mpPortAppearance;
}


//! @brief virtual function, only usefull for group port, guiport will return it self (this)
Port* Port::getRealPort()
{
    return this;
}


//! @brief Returns the rotation set by port appearance
double Port::getPortRotation()
{
    return mpPortAppearance->rot;
}


void Port::setEnable(bool enable)
{
    mpPortAppearance->mEnabled = enable;

    if(!enable)
    {
        // Remove all connections to this port if it is set to disabled
        disconnectAndRemoveAllConnectedConnectors();
        // Also hide it
        hide();
    }
    else
    {
        // Only show if not connected and not suposed to be hidden if unconnected
        if (!isConnected() && getParentContainerObject()->areSubComponentPortsShown())
        {
            show();
        }
    }
}

//! @brief This function tag or untags the Port appearance data as modified
void Port::setModified(bool modified)
{
    mpPortAppearance->mPoseModified = modified;
}


void Port::hide()
{
    this->magnify(false);
    mpPortLabel->hide();
    QGraphicsWidget::hide();
}

void Port::show()
{
    if (mpPortAppearance->mEnabled)
    {
        QGraphicsWidget::show();
    }
    mpPortLabel->hide();
}


QString Port::getName() const
{
    return mPortDisplayName;
}


QString Port::getParentModelObjectName() const
{
    return mpParentModelObject->getName();
}


void Port::setDisplayName(const QString name)
{
    mPortDisplayName = name;
    refreshPortLabelText();
}


bool Port::getLastNodeData(QString dataName, double& rData)
{
    return mpParentModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getLastNodeData(getParentModelObjectName(), this->getName(), dataName, rData);
}


//! @brief Slot that hides the port if "hide ports" setting is enabled, but only if the project tab is opened.
//! @param doShow shall we show unconnected ports
void Port::showIfNotConnected(bool doShow)
{
    if(!isConnected() && mpPortAppearance->mEnabled && doShow && mpParentModelObject->isVisible())
    {
        this->show();
    }
    else
    {
        this->hide();
    }
}


GroupPort::GroupPort(QString name, double xpos, double ypos, PortAppearance* pPortAppearance, ModelObject *pParentObject)
    : Port(name, xpos, ypos, pPortAppearance, pParentObject)
{
    //Nothing for now
}

//! @brief Overloaded as groups laks core connection
QString GroupPort::getPortType(const CoreSystemAccess::PortTypeIndicatorT /*ind*/)
{
    //! @todo Return something smart
    return "GroupPortType";
}


//! @brief Overloaded as groups laks core connection
QString GroupPort::getNodeType()
{
    //! @todo Return something smart
    return "GropPortNodeType";
}

void GroupPort::rememberConnection(Connector *pConnector)
{
    Port::rememberConnection(pConnector);
    mSharedGroupPortInfo->mConnectedConnectors.append(pConnector);
}

void GroupPort::forgetConnection(Connector *pConnector)
{
    Port::forgetConnection(pConnector);
    mSharedGroupPortInfo->mConnectedConnectors.remove(mSharedGroupPortInfo->mConnectedConnectors.indexOf(pConnector));
}


Port* GroupPort::getRealPort()
{
    if (mSharedGroupPortInfo->mConnectedConnectors.size() == 0)
    {
        return 0;
    }
    else
    {
        Connector *pCon = mSharedGroupPortInfo->mConnectedConnectors[0];

        // If the startport is one of the shared ports, this or our sibling (internal <-> external), we should choose the other port instead
        if (mSharedGroupPortInfo->mSharedPorts.contains(pCon->getStartPort()))
        {
            return pCon->getEndPort();
        }
        else
        {
            return pCon->getStartPort();
        }
    }
}

//! @brief Returns a vector with all ports connected to this port.
QVector<Port *> GroupPort::getConnectedPorts()
{
    QVector<Port *> vector;
    for(int i=0; i<mSharedGroupPortInfo->mConnectedConnectors.size(); ++i)
    {
        Connector *pCon = mSharedGroupPortInfo->mConnectedConnectors[i];

        // If the startport is one of the shared ports, this or our sibling (internal <-> external), we should choose the other port instead
        if (mSharedGroupPortInfo->mSharedPorts.contains(pCon->getStartPort()))
        {
            vector.append(pCon->getEndPort());
        }
        else
        {
            vector.append(pCon->getStartPort());
        }
    }
    return vector;
}

SharedGroupInfoPtrT GroupPort::getSharedGroupPortInfo()
{
    return mSharedGroupPortInfo;
}

void GroupPort::setSharedGroupPortInfo(SharedGroupInfoPtrT sharedGroupPortInfo)
{
    if (!mSharedGroupPortInfo.isNull())
    {
        mSharedGroupPortInfo->mSharedPorts.removeAll(this); // Remove knowledge about this port
    }
    mSharedGroupPortInfo = sharedGroupPortInfo;         // Set new shared info
    mSharedGroupPortInfo->mSharedPorts.append(this);    // Remember this port
}
