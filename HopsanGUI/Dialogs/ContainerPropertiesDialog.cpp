/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   ContainerPropertiesDialog.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-24
//!
//! @brief Contains a class for manipulation of Container properties
//!
//$Id$

#include "ContainerPropertiesDialog.h"

#include <QDebug>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QListWidget>

//#include "common.h"
#include "global.h"
#include "Configuration.h"
#include "ModelObjectPropertiesDialog.h"
#include "DesktopHandler.h"
#include "ModelHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Widgets/LibraryWidget.h"
#include "Dialogs/MovePortsDialog.h"
#include "Dialogs/ParameterSettingsLayout.h"


//! @brief Constructor for the container properties dialog
//! @param[in] pContainerObject Pointer to the container
//! @param[in] pParentWidget Pointer to the parent widget
ContainerPropertiesDialog::ContainerPropertiesDialog(ContainerObject *pContainerObject, QWidget *pParentWidget)
    : ModelObjectPropertiesDialog(pContainerObject, pParentWidget)
{
    mpContainerObject = pContainerObject;

        //Set the name and size of the main window
    this->setObjectName("SystemPropertiesDialog");
    this->resize(380,200);
    this->setWindowTitle("System Properties");
    this->setPalette(gpConfig->getPalette());

    //Info
    QString author, email, affiliation, description;
    mpContainerObject->getModelInfo(author, email, affiliation, description);
    //QGroupBox *pInfoGroupBox = new QGroupBox("Info", this);
    QWidget *pInfoWidget = new QWidget(this);
    QGridLayout *pInfoLayout = new QGridLayout();

    mpAuthorLabel = new QLabel("Author: ", this);
    mpEmailLabel = new QLabel("Email: ", this);
    mpAffiliationLabel = new QLabel("Affiliation: ", this);
    mpDescriptionLabel = new QLabel("Description: ", this);
    mpCQSLabel = new QLabel("CQS-type: ", this);
    mpCQSTypeLabel = new QLabel(mpContainerObject->getTypeCQS(), this);
    QLabel *pBasePathLabel = new QLabel("BasePath: ", this);

    mpAuthorEdit = new QLineEdit(author, this);
    mpEmailEdit = new QLineEdit(email, this);
    mpAffiliationEdit = new QLineEdit(affiliation, this);
    mpDescriptionEdit = new QTextEdit(description, this);
    mpDescriptionEdit->setFixedHeight(60);
    QLineEdit *pBasePathEdit = new QLineEdit(mpContainerObject->getAppearanceData()->getBasePath(), this);
    pBasePathEdit->setReadOnly(true);

    pInfoLayout->addWidget(mpAuthorLabel,       0, 0);
    pInfoLayout->addWidget(mpAuthorEdit,        0, 1);
    pInfoLayout->addWidget(mpEmailLabel,        1, 0);
    pInfoLayout->addWidget(mpEmailEdit,         1, 1);
    pInfoLayout->addWidget(mpAffiliationLabel,  2, 0);
    pInfoLayout->addWidget(mpAffiliationEdit,   2, 1);
    pInfoLayout->addWidget(mpDescriptionLabel,  3, 0);
    pInfoLayout->addWidget(mpDescriptionEdit,   3, 1);
    pInfoLayout->addWidget(mpCQSLabel,          4, 0);
    pInfoLayout->addWidget(mpCQSTypeLabel,      4, 1);
    pInfoLayout->addWidget(pBasePathLabel,      5, 0);
    pInfoLayout->addWidget(pBasePathEdit,       5, 1);
    pInfoLayout->addWidget(new QWidget(this),   6, 0, 1, 2);
    //pInfoGroupBox->setLayout(pInfoLayout);
    pInfoLayout->setRowStretch(6, 1);
    pInfoWidget->setLayout(pInfoLayout);

    //Define items in the dialog box
        //Name edit
    mpNameLayout = new QHBoxLayout();
    mpNameLabel = new QLabel("Name: ", this);
    mpNameEdit = new QLineEdit(mpContainerObject->getName(), this);

       //Icon paths
    mpUserIconLabel = new QLabel("User Icon Path:", this);
    mpIsoIconLabel = new QLabel( "ISO Icon Path:", this);
    mpUserIconPath = new QLineEdit(mpContainerObject->getIconPath(UserGraphics, Absolute), this);
    mpIsoIconPath = new QLineEdit(mpContainerObject->getIconPath(ISOGraphics, Absolute), this);
    mpUserIconLabel->setMinimumWidth(80);
    mpUserIconPath->setMinimumWidth(200);
    mpIsoIconBrowseButton = new QPushButton(tr("..."), this);
    mpUserIconBrowseButton = new QPushButton(tr("..."), this);
//    mpIsoIconBrowseButton->setFixedSize(25, 22);
//    mpUserIconBrowseButton->setFixedSize(25, 22);
    mpIsoIconBrowseButton->setAutoDefault(false);
    mpUserIconBrowseButton->setAutoDefault(false);

        //Graphics checkboxes
    mpIsoCheckBox = new QCheckBox(tr("Use ISO 1219 Graphics"), this);
    mpIsoCheckBox->setCheckable(true);
    mpIsoCheckBox->setChecked(mpContainerObject->getGfxType());

        //Graphics scales
    QString text;
    QLabel *pUserIconScaleLabel = new QLabel("User Icon Scale:", this);
    QLabel *pIsoIconScaleLabel = new QLabel("ISO Icon Scale:", this);
    text.setNum(mpContainerObject->getAppearanceData()->getIconScale(UserGraphics));
    mpUserIconScaleEdit = new QLineEdit(this);
    mpUserIconScaleEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2, this));
    mpUserIconScaleEdit->setText(text);
    text.setNum(mpContainerObject->getAppearanceData()->getIconScale(ISOGraphics));
    mpIsoIconScaleEdit = new QLineEdit(this);
    mpIsoIconScaleEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2, this));
    mpIsoIconScaleEdit->setText(text);


        //Appearance Group Box
    //mpAppearanceGroupBox = new QGroupBox("Appearance", this);
    QWidget *pAppearanceWidget = new QWidget(this);
    mpAppearanceLayout = new QGridLayout(this);
    mpAppearanceLayout->addWidget(mpNameLabel, 0, 0);
    mpAppearanceLayout->addWidget(mpNameEdit, 0, 1, 1, 2);
    mpAppearanceLayout->addWidget(mpUserIconLabel, 1, 0);
    mpAppearanceLayout->addWidget(mpUserIconPath, 1, 1);
    mpAppearanceLayout->addWidget(mpUserIconBrowseButton, 1, 2);
    mpAppearanceLayout->addWidget(mpIsoIconLabel, 2, 0);
    mpAppearanceLayout->addWidget(mpIsoIconPath, 2, 1);
    mpAppearanceLayout->addWidget(mpIsoIconBrowseButton, 2, 2);
    mpAppearanceLayout->addWidget(pUserIconScaleLabel, 3, 0);
    mpAppearanceLayout->addWidget(mpUserIconScaleEdit, 3, 1);
    mpAppearanceLayout->addWidget(pIsoIconScaleLabel, 4, 0);
    mpAppearanceLayout->addWidget(mpIsoIconScaleEdit, 4, 1);
    mpAppearanceLayout->addWidget(mpIsoCheckBox, 5, 0, 1, -1);
    mpAppearanceLayout->addWidget(new QWidget(this), 6, 0, 1, 2);
    mpAppearanceLayout->setRowStretch(6, 1);
    //mpAppearanceGroupBox->setLayout(mpAppearanceLayout);
    pAppearanceWidget->setLayout(mpAppearanceLayout);

        //Load start values or not
    mpUseStartValues = new QCheckBox("Keep start values from previous simulation", this);
    mpUseStartValues->setCheckable(true);
    mpUseStartValues->setChecked(mpContainerObject->getCoreSystemAccessPtr()->doesKeepStartValues());

        //Disable undo checkbox
    mpDisableUndoCheckBox = new QCheckBox(tr("Disable Undo Function"), this);
    mpDisableUndoCheckBox->setCheckable(true);
    mpDisableUndoCheckBox->setChecked(!mpContainerObject->isUndoEnabled());

        //Save undo checkbox
    mpSaveUndoCheckBox = new QCheckBox(tr("Save Undo Function In Model File"), this);
    mpSaveUndoCheckBox->setCheckable(true);
    mpSaveUndoCheckBox->setChecked(mpContainerObject->getSaveUndo());

        //Startup python script file
    mpPyScriptLabel = new QLabel("Script File:", this);
    mpPyScriptPath = new QLineEdit(this);
    mpPyScriptLabel->setMinimumWidth(80);
    mpPyScriptPath->setMinimumWidth(200);
    mpPyScriptBrowseButton = new QPushButton(tr("..."), this);

        //Settings Group Box
    //mpSettingsGroupBox = new QGroupBox("Settings", this);
    QWidget *pSettingsWidget = new QWidget(this);
    mpSettingsLayout = new QGridLayout(this);
    mpSettingsLayout->addWidget(mpPyScriptLabel,        0, 0);
    mpSettingsLayout->addWidget(mpPyScriptPath,         0, 1);
    mpSettingsLayout->addWidget(mpPyScriptBrowseButton, 0, 2);
    mpSettingsLayout->addWidget(mpUseStartValues,       1, 0, 1, 2);
    mpSettingsLayout->addWidget(mpDisableUndoCheckBox,  2, 0, 1, 2);
    mpSettingsLayout->addWidget(mpSaveUndoCheckBox,     3, 0, 1, 2);
    mpSettingsLayout->addWidget(new QWidget(this),      4, 0, 1, 2);
    mpSettingsLayout->setRowStretch(4, 1);
    //mpSettingsGroupBox->setLayout(mpSettingsLayout);
    pSettingsWidget->setLayout(mpSettingsLayout);

        //Set GuiSystem specific stuff
    if (mpContainerObject->type() == SystemContainerType)
    {
            //Time step
        mpTimeStepCheckBox = new QCheckBox("Inherit time step from parent system", this);
        mpTimeStepCheckBox->setChecked(mpContainerObject->getCoreSystemAccessPtr()->doesInheritTimeStep());
        mpTimeStepLabel = new QLabel(" Time Step:", this);
        mpTimeStepLabel->setDisabled(mpTimeStepCheckBox->isChecked());
        QString TimeStepText;
        if(mpTimeStepCheckBox->isChecked())
        {
            TimeStepText.setNum(mpContainerObject->getParentContainerObject()->getCoreSystemAccessPtr()->getDesiredTimeStep());
        }
        else
        {
            TimeStepText.setNum(mpContainerObject->getCoreSystemAccessPtr()->getDesiredTimeStep());
        }
        mpTimeStepEdit = new QLineEdit(this);
        mpTimeStepEdit->setValidator(new QDoubleValidator(0.0, 2000000000.0, 10, this));
        mpTimeStepEdit->setText(TimeStepText);
        mpTimeStepEdit->setDisabled(mpTimeStepCheckBox->isChecked());

        connect(mpTimeStepCheckBox, SIGNAL(toggled(bool)), mpTimeStepLabel, SLOT(setDisabled(bool)));
        connect(mpTimeStepCheckBox, SIGNAL(toggled(bool)), mpTimeStepEdit, SLOT(setDisabled(bool)));
        connect(mpTimeStepCheckBox, SIGNAL(toggled(bool)), this, SLOT(fixTimeStepInheritance(bool)));
        mpSettingsLayout->addWidget(mpTimeStepCheckBox, 4, 0, 1, 2);
        mpSettingsLayout->addWidget(mpTimeStepLabel, 5, 0, 1, 1);
        mpSettingsLayout->addWidget(mpTimeStepEdit, 5, 1, 1, 1);

            //Log samples
        mpNumLogSamplesEdit = new QLineEdit(this);
        mpNumLogSamplesEdit->setValidator(new QIntValidator(0, 2000000000, this));
        mpNumLogSamplesEdit->setText(QString("%1").arg(mpContainerObject->getNumberOfLogSamples())); //!< @todo what if group
        mpSettingsLayout->addWidget(new QLabel(tr("Log Samples:"), this), 6, 0);
        mpSettingsLayout->addWidget(mpNumLogSamplesEdit, 6, 1);

        // Log start time
        mpLogStartTimeEdit = new QLineEdit(this);
        mpLogStartTimeEdit->setValidator(new QDoubleValidator(this));
        mpLogStartTimeEdit->setText(QString("%1").arg(mpContainerObject->getLogStartTime())); //!< @todo what if group
        mpSettingsLayout->addWidget(new QLabel(tr("Log Start:"), this), 7, 0);
        mpSettingsLayout->addWidget(mpLogStartTimeEdit, 7, 1);

        QPushButton *pClearLogDataButton = new QPushButton("Clear All Log Data", this);
        pClearLogDataButton->setEnabled(!mpContainerObject->getLogDataHandler()->isEmpty());
        connect(pClearLogDataButton, SIGNAL(clicked()), this, SLOT(clearLogData()));
        mpSettingsLayout->addWidget(pClearLogDataButton, 8,0);

        mpPyScriptPath->setText(mpContainerObject->getScriptFile());

        // System Parameters Group Box
        QVector<CoreParameterData> paramDataVector;
        mpContainerObject->getParameters(paramDataVector);
        for(int i=0; i<paramDataVector.size(); ++i)
        {
            mvParameterLayoutPtrs.push_back(new ParameterSettingsLayout(paramDataVector[i], mpContainerObject));
        }
        // Adjust sizes of labels, to make sure that all text is visible and that the spacing is not too big between them
        int descriptionSize=30;
        int nameSize = 10;
        for(int i=0; i<mvParameterLayoutPtrs.size(); ++i)
        {
            descriptionSize = std::max(descriptionSize, mvParameterLayoutPtrs.at(i)->mDescriptionLabel.width());
            nameSize = std::max(nameSize, mvParameterLayoutPtrs.at(i)->mNameLabel.width());
        }
        for(int i=0; i<mvParameterLayoutPtrs.size(); ++i)
        {
           // mvParameterLayoutPtrs.at(i)->mDescriptionLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
            mvParameterLayoutPtrs.at(i)->mDescriptionLabel.setFixedWidth(0);
            mvParameterLayoutPtrs.at(i)->mNameLabel.setFixedWidth(nameSize+10);
        }
    }

        //This is the main Vertical layout of the dialog
    //mpSettingsScrollLayout = new QVBoxLayout(this);
    //mpSettingsScrollLayout->addWidget(pInfoGroupBox);
    //mpSettingsScrollLayout->addWidget(mpAppearanceGroupBox);
    //mpSettingsScrollLayout->addWidget(mpSettingsGroupBox);
    // Check if we should add systemparameters

        //Done and Cancel Buttons
    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpEditPortPos = new QPushButton(tr("&Move ports"), this);
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpDoneButton = new QPushButton(tr("&Done"), this);
    mpDoneButton->setDefault(true);
    mpButtonBox->addButton(mpEditPortPos, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpDoneButton, QDialogButtonBox::ActionRole);
    //mpScrollLayout->addWidget(mpButtonBox, 0, Qt::AlignHCenter);

//    mpSettingsWidget = new QWidget();
//    mpSettingsWidget->setLayout(mpSettingsScrollLayout);
//    mpSettingsWidget->setPalette(gConfig.getPalette());

//    QScrollArea *pSettingsScrollArea = new QScrollArea(this);
//    pSettingsScrollArea->setWidget(mpSettingsWidget);
//    pSettingsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    //LOGGING

    QListWidget *pDataLoggingList = new QListWidget(this);
    pDataLoggingList->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QListWidgetItem *testItem = new QListWidgetItem("Test item!", pDataLoggingList);
    testItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    testItem->setCheckState(Qt::Checked);
    pDataLoggingList->addItem(testItem);

    QVBoxLayout *pDataLoggingLayout = new QVBoxLayout(this);
    pDataLoggingLayout->addWidget(pDataLoggingList);

    QWidget *pDataLoggingWidget = new QWidget(this);
    pDataLoggingWidget->setLayout(pDataLoggingLayout);

    QScrollArea *pDataLoggingScrollArea = new QScrollArea(this);
    pDataLoggingScrollArea->setWidget(pDataLoggingWidget);

    //END LOGGING

    //Arrange tabs and set primary layout
    QGridLayout *pPrimaryLayout = new QGridLayout(this);
    QTabWidget *pTabWidget = new QTabWidget(this);
    //pTabWidget->addTab(pSettingsScrollArea, "General");
    pTabWidget->addTab(pSettingsWidget, "Settings");
    pTabWidget->addTab(pAppearanceWidget, "Appearance");
    pTabWidget->addTab(pInfoWidget, "Model Info");
    //pTabWidget->addTab(pDataLoggingScrollArea, "Data Logging");
    pDataLoggingScrollArea->hide();
    pPrimaryLayout->addWidget(pTabWidget);
    pPrimaryLayout->addWidget(mpButtonBox);
    setLayout(pPrimaryLayout);

    if( (mpContainerObject != gpModelHandler->getCurrentViewContainerObject()) && (mvParameterLayoutPtrs.size()>0))
    {
        QGridLayout *pParameterLayout = new QGridLayout();
        //mpSystemParametersGroupBox = new QGroupBox("System Parameters", this);
        QWidget *pSystemParametersWidget = new QWidget(this);

        for(int i=0; i<mvParameterLayoutPtrs.size(); ++i)
        {
            pParameterLayout->addLayout(mvParameterLayoutPtrs[i], i, 0);
            pParameterLayout->addWidget(new QWidget(this), i, 1);
        }

        pParameterLayout->addWidget(new QWidget(this), pParameterLayout->rowCount(), 0);
        pParameterLayout->setRowStretch(pParameterLayout->rowCount(), 1);
        pParameterLayout->setColumnStretch(1, 1);

        //mpSystemParametersGroupBox->setLayout(pParameterLayout);
        pSystemParametersWidget->setLayout(pParameterLayout);

        //mpSettingsScrollLayout->addWidget(mpSystemParametersGroupBox);
        pTabWidget->addTab(pSystemParametersWidget, "System Parameters");
    }

    //mpSettingsWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    //mpSettingsScrollLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//    pPrimaryLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
//    //pSettingsScrollArea->setFixedHeight(std::min(mpSettingsWidget->height()+3, maxHeight));
//    if(pSettingsScrollArea->minimumHeight() == maxHeight)
//    {
//        pSettingsScrollArea->setMinimumWidth(mpSettingsWidget->width()+19);
//    }
//    else
//    {
//        pSettingsScrollArea->setMinimumWidth(mpSettingsWidget->width()+3);
//    }
//    pSettingsScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
//    pSettingsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //Create connections
    connect(mpCancelButton,         SIGNAL(clicked()), this, SLOT(close()));
    connect(mpDoneButton,           SIGNAL(clicked()), this, SLOT(setValues()));
    connect(mpIsoIconBrowseButton,  SIGNAL(clicked()), this, SLOT(browseIso()));
    connect(mpUserIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseUser()));
    connect(mpPyScriptBrowseButton, SIGNAL(clicked()), this, SLOT(browseScript()));
    connect(mpEditPortPos,          SIGNAL(clicked()), this, SLOT(editPortPos()));
}


void ContainerPropertiesDialog::editPortPos()
{
    //! @warning memory leak, should not create new dialogs every time, they are never removed
    MovePortsDialog *dialog = new MovePortsDialog(mpContainerObject->getAppearanceData(), 0, mpContainerObject->getGfxType());
    connect(dialog, SIGNAL(finished()), mpContainerObject, SLOT(refreshAppearance()), Qt::UniqueConnection);
}


void ContainerPropertiesDialog::fixTimeStepInheritance(bool value)
{
    QString TimeStepText;
    if(value)
    {
        TimeStepText.setNum(mpContainerObject->getParentContainerObject()->getCoreSystemAccessPtr()->getDesiredTimeStep());
    }
    else
    {
        TimeStepText.setNum(mpContainerObject->getCoreSystemAccessPtr()->getDesiredTimeStep());
    }
    mpTimeStepEdit->setText(TimeStepText);
}


//! @brief Updates model settings according to the selected values
void ContainerPropertiesDialog::setValues()
{
    this->mpContainerObject->setName(this->mpNameEdit->text());

    if(mpIsoCheckBox->isChecked() && mpContainerObject->getGfxType() != ISOGraphics)
    {
        this->mpContainerObject->setGfxType(ISOGraphics);
        gpLibraryWidget->setGfxType(ISOGraphics);
    }
    else if(!mpIsoCheckBox->isChecked() && mpContainerObject->getGfxType() != UserGraphics)
    {
        this->mpContainerObject->setGfxType(UserGraphics);
        gpLibraryWidget->setGfxType(UserGraphics);
    }

    mpContainerObject->getCoreSystemAccessPtr()->setLoadStartValues(mpUseStartValues->isChecked());

    if(mpContainerObject->isUndoEnabled() == mpDisableUndoCheckBox->isChecked())
    {
        mpContainerObject->setUndoEnabled(!mpDisableUndoCheckBox->isChecked());
    }

    mpContainerObject->setSaveUndo(mpSaveUndoCheckBox->isChecked());

    //Set the icon paths, only update and refresh appearance if a change has occurred
    if ( mpContainerObject->getIconPath(ISOGraphics, Absolute) != mpIsoIconPath->text() )
    {
        mpContainerObject->setIconPath(mpIsoIconPath->text(), ISOGraphics, Absolute);
        mpContainerObject->refreshAppearance();
    }
    if ( mpContainerObject->getIconPath(UserGraphics, Absolute) != mpUserIconPath->text() )
    {
        mpContainerObject->setIconPath(mpUserIconPath->text(), UserGraphics, Absolute);
        mpContainerObject->refreshAppearance();
    }

    //Set scale if they have changed
    //! @todo maybe use fuzzy compare utility function instead (but then we need to include utilities here)
    if ( fabs(mpContainerObject->getAppearanceData()->getIconScale(ISOGraphics) - mpIsoIconScaleEdit->text().toDouble()) > 0.001 )
    {
        mpContainerObject->getAppearanceData()->setIconScale(mpIsoIconScaleEdit->text().toDouble(), ISOGraphics);
        mpContainerObject->refreshAppearance();
    }
    if ( fabs(mpContainerObject->getAppearanceData()->getIconScale(UserGraphics) - mpUserIconScaleEdit->text().toDouble()) > 0.001 )
    {
        mpContainerObject->getAppearanceData()->setIconScale(mpUserIconScaleEdit->text().toDouble(), UserGraphics);
        mpContainerObject->refreshAppearance();
    }

    //Set GuiSystem specific stuff
    if (mpContainerObject->type() == SystemContainerType)
    {
        mpContainerObject->getCoreSystemAccessPtr()->setInheritTimeStep(mpTimeStepCheckBox->isChecked());
        mpContainerObject->getCoreSystemAccessPtr()->setDesiredTimeStep(mpTimeStepEdit->text().toDouble());
        mpContainerObject->setNumberOfLogSamples(mpNumLogSamplesEdit->text().toInt());
        mpContainerObject->setLogStartTime(mpLogStartTimeEdit->text().toDouble());
        mpContainerObject->setScriptFile(mpPyScriptPath->text());
    }

    //Set model info
    mpContainerObject->setModelInfo(mpAuthorEdit->text(), mpEmailEdit->text(), mpAffiliationEdit->text(), mpDescriptionEdit->toPlainText());

    bool success = this->setParameterValues(mvParameterLayoutPtrs);

    if (success)
    {
        this->close();
    }
}


//! @brief Slot that opens a file dialog where user can select a user icon for the system
void ContainerPropertiesDialog::browseUser()
{
    QString iconFileName = QFileDialog::getOpenFileName(this, tr("Choose user icon"),
                                                        gpDesktopHandler->getModelsPath());
    if (!iconFileName.isEmpty())
    {
        mpUserIconPath->setText(iconFileName);
    }
}


//! @brief Slot that opens a file dialog where user can select an iso icon for the system
void ContainerPropertiesDialog::browseIso()
{
    QDir fileDialogOpenDir;
    QString iconFileName = QFileDialog::getOpenFileName(this, tr("Choose ISO icon"),
                                                        gpDesktopHandler->getModelsPath());
    if (!iconFileName.isEmpty())
    {
        mpIsoIconPath->setText(iconFileName);
    }
}


//! @brief Slot that opens a file dialog where user can select a script file for the system
void ContainerPropertiesDialog::browseScript()
{
    //QDir fileDialogOpenDir;
    QString scriptFileName = QFileDialog::getOpenFileName(this, tr("Choose script"),
                                                         gpDesktopHandler->getModelsPath());
    if (!scriptFileName.isEmpty())
    {
        mpPyScriptPath->setText(scriptFileName);
    }
}

void ContainerPropertiesDialog::clearLogData()
{
    QMessageBox existWarningBox(QMessageBox::Warning, "Warning","ALL log data in current model will be cleared. Continue?", 0, 0);
    existWarningBox.addButton("Yes", QMessageBox::AcceptRole);
    existWarningBox.addButton("No", QMessageBox::RejectRole);
    existWarningBox.setWindowIcon(QIcon(QString(QString(ICONPATH) + "hopsan.png")));
    bool doIt = (existWarningBox.exec() == QMessageBox::AcceptRole);

    if(doIt)
    {
        mpContainerObject->getLogDataHandler()->clear();
    }
}
