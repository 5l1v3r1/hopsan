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
//! @file   UndoWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains the undo widget class (which displays the stack)
//!
//$Id$

#include <QtGui>

#include "Widgets/UndoWidget.h"
#include "UndoStack.h"
#include "ModelWidget.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIContainerObject.h"
#include "common.h"
#include "global.h"
#include "ModelHandler.h"


//! @class UndoWidget
//! @brief The UndoWidget class is used to display a list of undo operations in the GUI.
//!
//! The undo widget is updated by calling the refreshList() function. It asks the undo stack in the current system for undo posts, translate their tags to readable
//! names and prints the list. It does not have a pointer to the undo stack because it depends on which system is open.
//!


//! @brief Construtor for undo list widget
//! @param parent Pointer to the parent main window
UndoWidget::UndoWidget(QWidget *parent)
    : QDialog(parent)
{
    //Set the name and size of the main window
    this->setObjectName("UndoWidget");
    this->resize(400,500);
    this->setWindowTitle("Undo History");

    mpRedoButton = new QPushButton(tr("&Redo"));
    mpRedoButton->setAutoDefault(false);
    mpRedoButton->setFixedHeight(30);
    QFont tempFont = mpRedoButton->font();
    tempFont.setBold(true);
    mpRedoButton->setFont(tempFont);

    mpUndoButton = new QPushButton(tr("&Undo"));
    mpUndoButton->setAutoDefault(false);
    mpUndoButton->setFixedHeight(30);
    mpUndoButton->setFont(tempFont);

    mpClearButton = new QPushButton(tr("&Clear"));
    mpClearButton->setAutoDefault(false);
    mpClearButton->setFixedHeight(30);
    mpClearButton->setFont(tempFont);

    mUndoTable = new QTableWidget(0,1);
    mUndoTable->setBaseSize(400, 500);
    mUndoTable->setColumnWidth(0, 400);
    mUndoTable->horizontalHeader()->setStretchLastSection(true);
    mUndoTable->horizontalHeader()->hide();

    mpLayout = new QGridLayout();
    mpLayout->setContentsMargins(4,4,4,4);
    mpLayout->addWidget(mUndoTable, 0, 0);
    mpLayout->addWidget(mpUndoButton, 1, 0);
    mpLayout->addWidget(mpRedoButton, 2, 0);
    mpLayout->addWidget(mpClearButton, 3, 0);
    setLayout(mpLayout);
}


//! @brief Reimplementation of show function. Updates the list every time before the widget is displayed.
void UndoWidget::show()
{
    refreshList();
    QDialog::show();
}


//! @brief Refresh function for the list. Reads from the current undo stack and displays the results in the table.
void UndoWidget::refreshList()
{
    if(gpModelHandler->count() == 0)
    {
        mpClearButton->setEnabled(false);
        mpUndoButton->setEnabled(false);
        mpRedoButton->setEnabled(false);
        return;
    }
    else
    {
        mpClearButton->setEnabled(true);
        mpUndoButton->setEnabled(true);
        mpRedoButton->setEnabled(true);
    }

    QTableWidgetItem *item;
    mUndoTable->clear();
    mUndoTable->setRowCount(0);


    //XML//

    QColor oddColor = QColor("white");
    QColor evenColor = QColor("whitesmoke");
    QColor activeColor = QColor("chartreuse");

    if(gpModelHandler->count() == 0 || !gpModelHandler->getCurrentViewContainerObject())
    {
        return;
    }

    int pos = 0;
    bool found = true;

    //qDebug() << "refreshList for Undo in: " << gpModelHandler->getCurrentContainer();
    //qDebug() << "refreshList for Undo in: " << gpModelHandler->getCurrentContainer()->getName();
    QDomElement undoRoot = gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mUndoRoot;
    QDomElement postElement = undoRoot.firstChildElement("post");
    while(found)
    {
        found = false;
        while(!postElement.isNull())
        {
            if(postElement.attribute("number").toInt() == pos)
            {
                //Found correct number, write it's contents to the list
                found = true;
                if(postElement.attribute("type") != QString())
                {
                    item = new QTableWidgetItem();
                    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                    item->setText(translateTag(postElement.attribute("type")));
                    if(pos == gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                    {
                        item->setBackgroundColor(activeColor);
                    }
                    else if(pos%2 == 0)
                    {
                        item->setBackgroundColor(evenColor);
                    }
                    else
                    {
                        item->setBackgroundColor(oddColor);
                    }
                    if(pos > gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                    {
                        item->setForeground(QColor("gray"));
                    }
                    mUndoTable->insertRow(0);
                    mUndoTable->setItem(0,0,item);
                }
                else
                {
                    QDomElement stuffElement = postElement.firstChildElement("stuff");
                    while(!stuffElement.isNull())
                    {
                        item = new QTableWidgetItem();
                        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        item->setText(translateTag(stuffElement.attribute("what")));
                        if(pos == gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                        {
                            item->setBackgroundColor(activeColor);
                        }
                        else if(pos%2 == 0)
                        {
                            item->setBackgroundColor(evenColor);
                        }
                        else
                        {
                            item->setBackgroundColor(oddColor);
                        }
                        if(pos > gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                        {
                            item->setForeground(QColor("gray"));
                        }
                        mUndoTable->insertRow(0);
                        mUndoTable->setItem(0,0,item);
                        stuffElement = stuffElement.nextSiblingElement("stuff");
                    }
                }
                break;
            }
            postElement = postElement.nextSiblingElement("post");
        }
        ++pos;
    }
    //qDebug() << gpModelHandler->getCurrentContainer()->mUndoStack->mDomDocument.toString();
}


//! @brief Help function that translates undo tags into more readable strings
//! @param tag Tag name to translate
QString UndoWidget::translateTag(QString tag)
{
    QMap<QString, QString> tagMap;
    tagMap.insert("addedobject",            "Added Object");
    tagMap.insert("addedconnector",         "Added Connector");
    tagMap.insert("deletedobject",          "Deleted Object");
    tagMap.insert("deletedconnector",       "Deleted Connector");
    tagMap.insert("movedobject",            "Moved Object");
    tagMap.insert("rename",                 "Renamed Object");
    tagMap.insert("modifiedconnector",      "Modified Connector");
    tagMap.insert("rotate",                 "Rotated Object");
    tagMap.insert("verticalflip",           "Flipped Vertical");
    tagMap.insert("horizontalflip",         "Flipped Horizontal");
    tagMap.insert("namevisibilitychange",   "Changed Name Visibility");
    tagMap.insert("paste",                  "Paste");
    tagMap.insert("movedmultiple",          "Moved Objects");
    tagMap.insert("cut",                    "Cut");
    tagMap.insert("changedparameters",      "Changed Parameter(s)");
    tagMap.insert("hideallnames",           "Hide All Name Text");
    tagMap.insert("showallnames",           "Show All Name Text");
    tagMap.insert("addedboxwidget",         "Added Box Widget");
    tagMap.insert("deletedboxwidget",       "Deleted Box Widget");
    tagMap.insert("addedtextwidget",        "Added Text Widget");
    tagMap.insert("deletedtextwidget",      "Deleted Text Widget");
    tagMap.insert("movedwidget",            "Moved Widget");
    tagMap.insert("movedmultiplewidgets",   "Moved Widgets");
    tagMap.insert("resizedboxwidget",       "Resized Box Widget");
    tagMap.insert("modifiedboxwidgetstyle", "Modified Box Widget");
    tagMap.insert("modifiedtextwidget",     "Modified Text Widget");
    tagMap.insert("alignx",                 "Align Vertical");
    tagMap.insert("aligny",                 "Align Horizontal");

    if(tagMap.contains(tag))
        return tagMap.find(tag).value();
    else
        return "Undefined Action";
}


//! @brief Returns a pointer to the undo button
QPushButton *UndoWidget::getUndoButton()
{
    return mpUndoButton;
}


//! @brief Returns a pointer to the redo button
QPushButton *UndoWidget::getRedoButton()
{
    return mpRedoButton;
}


//! @brief Returns a pointer to the clear button
QPushButton *UndoWidget::getClearButton()
{
    return mpClearButton;
}