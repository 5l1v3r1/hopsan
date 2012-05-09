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
//! @file   GUIModelObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIModelObject class (The baseclass for all objects representing model parts)
//!
//$Id$

#ifndef GUIMODELOBJECT_H
#define GUIMODELOBJECT_H

#include "GUIObject.h"
#include "GUIModelObjectAppearance.h"
#include "CoreAccess.h"
#include <QGraphicsSvgItem>

class Connector;
class ModelObjectDisplayName;
class Port;
class SystemContainer;

class ModelObject : public WorkspaceObject
{
    Q_OBJECT

public:
    ModelObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType graphics = USERGRAPHICS, ContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);
    virtual ~ModelObject();

    virtual void setParentContainerObject(ContainerObject *pParentContainer);

    //Name methods
    virtual void setName(QString name);
    virtual QString getName();
    virtual void refreshDisplayName();
    virtual void setDisplayName(QString name);
    virtual QString getTypeName();
    virtual int getNameTextPos();
    virtual void setNameTextPos(int textPos);

    //CQS methods
    virtual QString getTypeCQS(){return "hasNoCqsType";} //Overloaded in GUISystem and GUIComponent

    //Appearance methods
    void setAppearanceDataBasePath(const QString basePath);
    virtual ModelObjectAppearance* getAppearanceData();
    bool isVisible();
    QGraphicsSvgItem *getIcon();
    virtual void refreshExternalPortsAppearanceAndPosition();

    //Help methods
    QString getHelpPicture();
    QString getHelpText();

    //Parameter methods
    virtual QStringList getParameterNames();
    virtual void getParameters(QVector<CoreParameterData> &rParameterDataVec);
    virtual void getParameter(const QString paramName, CoreParameterData &rData);
    virtual QString getParameterValue(const QString paramName);

    virtual QString getDefaultParameterValue(const QString paramName) const;


    virtual bool setParameterValue(QString name, QString valueTxt, bool force=false);
    virtual QString getStartValueTxt(QString portName, QString variable);
    virtual bool setStartValue(QString portName, QString variable, QString sysParName);

    //Load and save methods
    virtual void saveToDomElement(QDomElement &rDomElement);
    virtual void loadFromHMF(QString /*modelFilePath=QString()*/) {assert(false);} //Only available in GUISystem for now
    virtual void loadFromDomElement(QDomElement &/*rDomElement*/) {assert(false);} //Only available in GUISystem for now
    virtual void setModelFileInfo(QFile &rFile);

    //Connector methods
    QList<Connector*> getConnectorPtrs();
    void rememberConnector(Connector *item);
    void forgetConnector(Connector *item);

    //Port methods
    void showPorts(bool visible);
    Port *getPort(QString name);
    QList<Port*> &getPortListPtrs();
    virtual void createRefreshExternalPort(QString portName);
    void createRefreshExternalDynamicParameterPort(QString portName);
    virtual void removeExternalPort(QString portName);

    enum { Type = MODELOBJECT };
    int type() const;

    void getLosses(double &total, double &hydraulic, double &mechanic);
    bool isLossesDisplayVisible();

public slots:
    virtual void refreshAppearance();
    void deleteMe();
    void rotate(qreal angle, undoStatus undoSettings = UNDO);
    void flipVertical(undoStatus undoSettings = UNDO);
    void flipHorizontal(undoStatus undoSettings = UNDO);
    void hideName(undoStatus undoSettings = NOUNDO);
    void showName(undoStatus undoSettings = NOUNDO);
    void setIcon(graphicsType);
    void showLosses();
    void hideLosses();
    void redrawConnectors();

signals:
    void nameChanged();

protected:
    //Protected methods
    virtual void openPropertiesDialog(){}
    virtual QAction *buildBaseContextMenu(QMenu &rMenue, QGraphicsSceneContextMenuEvent* pEvent);

    //Reimplemented Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    //Save and load methods
    virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void saveCoreDataToDomElement(QDomElement &rDomElement);

    //Protected members
    ModelObjectAppearance mModelObjectAppearance;

    double mTextOffset;
    int mNameTextPos;

    graphicsType mIconType;
    bool mIconRotation;
    QGraphicsSvgItem *mpIcon;
    QString mLastIconPath;
    qreal mLastIconScale;

    QMap<QString, QString> mDefaultParameterValues;
    QStringList mActiveDynamicParameterPortNames;

    QList<Port*> mPortListPtrs;
    QList<Connector*> mConnectorPtrs;

    ModelObjectDisplayName *mpNameText;

    QGraphicsTextItem *mpLossesDisplay;

    double mTotalLosses;
    double mHydraulicLosses;
    double mMechanicLosses;

    bool mDragCopying;

protected slots:
    void snapNameTextPosition(QPointF pos);
    void calcNameTextPositions(QVector<QPointF> &rPts);
    void setNameTextScale(qreal scale);
    void setIconZoom(const qreal zoom);

private:
    void refreshIconPosition();
};


class ModelObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT

public:
    ModelObjectDisplayName(ModelObject *pParent);

public slots:
    void deselect();

signals:
    void textMoved(QPointF pos);

protected:
    //Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    //Protected members
    ModelObject* mpParentModelObject;
};

#endif // GUIMODELOBJECT_H
