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
//! @file   OptimizationDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-10-24
//!
//! @brief Contains a class for the optimization dialog
//!
//$Id$

#include <QDebug>
#include <limits>
#ifndef WIN32
#include <unistd.h>
#endif
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GUIPort.h"
#include "Dialogs/OptimizationDialog.h"
#include "HcomHandler.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"
#include "Utilities/HighlightingUtilities.h"
#include "Widgets/HcomWidget.h"
#include "ModelHandler.h"
#include "Widgets/PyDockWidget.h"
#include "MainWindow.h"
#include "OptimizationHandler.h"
#include "GUIObjects/GUISystem.h"
#include "Widgets/ModelWidget.h"

class CentralTabWidget;


//! @brief Constructor
OptimizationDialog::OptimizationDialog(QWidget *parent)
    : QWizard(parent)
{
        //Set the name and size of the main window
    this->resize(1024,768);
    this->setWindowTitle("Optimization");
    this->setPalette(gpConfig->getPalette());

    //Settings tab
    QLabel *pSettingsLabel = new QLabel("Please choose general settings for optimization algorithm.");
    QFont boldFont = pSettingsLabel->font();
    boldFont.setBold(true);
    pSettingsLabel->setFont(boldFont);

    QLabel *pAlgorithmLabel = new QLabel("Optimiation algorithm:");
    mpAlgorithmBox = new QComboBox(this);
    mpAlgorithmBox->addItems(QStringList() << "Complex" << "Particle Swarm");
    connect(mpAlgorithmBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setAlgorithm(int)));

    QLabel *pIterationsLabel = new QLabel("Number of iterations:");
    mpIterationsSpinBox = new QSpinBox(this);
    mpIterationsSpinBox->setRange(0, std::numeric_limits<int>::max());
    mpIterationsSpinBox->setValue(100);

    mpSearchPointsLabel = new QLabel("Number of search points:" );
    mpSearchPointsSpinBox = new QSpinBox(this);
    mpSearchPointsSpinBox->setRange(1, std::numeric_limits<int>::max());
    mpSearchPointsSpinBox->setValue(8);

    mpParticlesLabel = new QLabel("Number of particles:" );
    mpParticlesSpinBox = new QSpinBox(this);
    mpParticlesSpinBox->setRange(1, std::numeric_limits<int>::max());
    mpParticlesSpinBox->setValue(20);

    mpAlphaLabel = new QLabel("Reflection coefficient: ");
    mpAlphaLineEdit = new QLineEdit("1.3", this);
    mpAlphaLineEdit->setValidator(new QDoubleValidator());

    mpBetaLabel = new QLabel("Randomization factor: ");
    mpBetaLineEdit = new QLineEdit("0.3", this);
    mpBetaLineEdit->setValidator(new QDoubleValidator());

    mpGammaLabel = new QLabel("Forgetting factor: ");
    mpGammaLineEdit = new QLineEdit("0.3", this);
    mpGammaLineEdit->setValidator(new QDoubleValidator());

    mpOmegaLabel = new QLabel("Inertia Weight: ");
    mpOmegaLineEdit = new QLineEdit("1", this);
    mpOmegaLineEdit->setValidator(new QDoubleValidator());

    mpC1Label = new QLabel("Learning factor 1: ");
    mpC1LineEdit = new QLineEdit("2", this);
    mpC1LineEdit->setValidator(new QDoubleValidator());

    mpC2Label = new QLabel("Learning Factor 2: ");
    mpC2LineEdit = new QLineEdit("2", this);
    mpC2LineEdit->setValidator(new QDoubleValidator());

    QLabel *pEpsilonFLabel = new QLabel("Tolerance for function convergence: ");
    mpEpsilonFLineEdit = new QLineEdit("0.00001", this);
    mpEpsilonFLineEdit->setValidator(new QDoubleValidator());

    QLabel *pEpsilonXLabel = new QLabel("Tolerance for parameter convergence: ");
    mpEpsilonXLineEdit = new QLineEdit("0.0001", this);
    mpEpsilonXLineEdit->setValidator(new QDoubleValidator());

    mpPlotBestWorstCheckBox = new QCheckBox("Plot best and worst objective values", this);
    mpPlotBestWorstCheckBox->setChecked(false);

    mpPlotParticlesCheckBox = new QCheckBox("Plot particles", this);
    mpPlotParticlesCheckBox->setChecked(false);

    mpPlottingCheckBox = new QCheckBox("Plot each iteration", this);
    mpPlottingCheckBox->setChecked(true);

    mpExport2CSVBox= new QCheckBox("Export trace data to CSV file", this);
    mpExport2CSVBox->setChecked(false);

    QGridLayout *pSettingsLayout = new QGridLayout(this);
    pSettingsLayout->addWidget(pSettingsLabel,        0, 0);
    pSettingsLayout->addWidget(pAlgorithmLabel,       1, 0);
    pSettingsLayout->addWidget(mpAlgorithmBox,         1, 1);
    connect(mpAlgorithmBox, SIGNAL(currentIndexChanged(int)), this, SLOT(recreateCoreProgressBars()));
    pSettingsLayout->addWidget(pIterationsLabel,      2, 0);
    pSettingsLayout->addWidget(mpIterationsSpinBox,    2, 1);
    pSettingsLayout->addWidget(mpSearchPointsLabel,    3, 0);
    pSettingsLayout->addWidget(mpSearchPointsSpinBox,  3, 1);
    connect(mpSearchPointsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(recreateCoreProgressBars()));
    connect(mpSearchPointsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(recreateParameterOutputLineEdits()));
    pSettingsLayout->addWidget(mpParticlesLabel,       3, 0);
    pSettingsLayout->addWidget(mpParticlesSpinBox,     3, 1);
    connect(mpParticlesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(recreateCoreProgressBars()));
    connect(mpParticlesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(recreateParameterOutputLineEdits()));
    pSettingsLayout->addWidget(mpAlphaLabel,           4, 0);
    pSettingsLayout->addWidget(mpAlphaLineEdit,        4, 1);
    pSettingsLayout->addWidget(mpOmegaLabel,           4, 0);
    pSettingsLayout->addWidget(mpOmegaLineEdit,        4, 1);
    pSettingsLayout->addWidget(mpBetaLabel,            5, 0);
    pSettingsLayout->addWidget(mpBetaLineEdit,         5, 1);
    pSettingsLayout->addWidget(mpC1Label,              5, 0);
    pSettingsLayout->addWidget(mpC1LineEdit,           5, 1);
    pSettingsLayout->addWidget(mpGammaLabel,           6, 0);
    pSettingsLayout->addWidget(mpGammaLineEdit,        6, 1);
    pSettingsLayout->addWidget(mpC2Label,              6, 0);
    pSettingsLayout->addWidget(mpC2LineEdit,           6, 1);
    pSettingsLayout->addWidget(pEpsilonFLabel,        7, 0);
    pSettingsLayout->addWidget(mpEpsilonFLineEdit,     7, 1);
    pSettingsLayout->addWidget(pEpsilonXLabel,        8, 0);
    pSettingsLayout->addWidget(mpEpsilonXLineEdit,     8, 1);
    pSettingsLayout->addWidget(mpPlottingCheckBox,     9, 0, 1, 2);
    pSettingsLayout->addWidget(mpPlotBestWorstCheckBox,10, 0, 1, 2);
    pSettingsLayout->addWidget(mpPlotParticlesCheckBox,11, 0, 1, 2);
    pSettingsLayout->addWidget(mpExport2CSVBox,        12, 0, 1, 2);
    pSettingsLayout->addWidget(new QWidget(this),      13, 0, 1, 2);    //Dummy widget for stretching the layout
    pSettingsLayout->setRowStretch(13, 1);
    QWizardPage *pSettingsWidget = new QWizardPage(this);
    pSettingsWidget->setLayout(pSettingsLayout);
    setAlgorithm(0);

    //Parameter tab
    QLabel *pParametersLabel = new QLabel("Choose optimization parameters, and specify their minimum and maximum values.");
    pParametersLabel->setFont(boldFont);
    mpParametersLogCheckBox = new QCheckBox("Use logarithmic parameter scaling", this);
    mpParametersLogCheckBox->setChecked(false);
    mpParametersList = new QTreeWidget(this);
    QLabel *pParameterMinLabel = new QLabel("Min Value");
    pParameterMinLabel->setAlignment(Qt::AlignCenter);
    QLabel *pParameterNameLabel = new QLabel("Parameter Name");
    pParameterNameLabel->setAlignment(Qt::AlignCenter);
    QLabel *pParameterMaxLabel = new QLabel("Max Value");
    pParameterMaxLabel->setAlignment(Qt::AlignCenter);
    pParameterMinLabel->setFont(boldFont);
    pParameterNameLabel->setFont(boldFont);
    pParameterMaxLabel->setFont(boldFont);
    mpParametersLayout = new QGridLayout(this);
    mpParametersLayout->addWidget(pParametersLabel,        0, 0, 1, 4);
    mpParametersLayout->addWidget(mpParametersLogCheckBox,  1, 0, 1, 4);
    mpParametersLayout->addWidget(mpParametersList,         2, 0, 1, 4);
    mpParametersLayout->addWidget(pParameterMinLabel,      3, 0, 1, 1);
    mpParametersLayout->addWidget(pParameterNameLabel,     3, 1, 1, 1);
    mpParametersLayout->addWidget(pParameterMaxLabel,      3, 2, 1, 1);
    QWizardPage *pParametersWidget = new QWizardPage(this);
    pParametersWidget->setLayout(mpParametersLayout);



    //Objective function tab
    QLabel *pObjectiveLabel = new QLabel("Create an objective function by first choosing variables in the list and then choosing a function below.");
    pObjectiveLabel->setFont(boldFont);
    mpVariablesList = new QTreeWidget(this);
    mpMinMaxComboBox = new QComboBox(this);
    mpMinMaxComboBox->addItems(QStringList() << "Minimize" << "Maximize");
    mpFunctionsComboBox = new QComboBox(this);
    mpFunctionsComboBox->setStyleSheet("QComboBox { combobox-popup: 10; }");
    mpAddFunctionButton = new QPushButton("Add Function");
    QLabel *pWeightLabel = new QLabel("Weight");
    QLabel *pNormLabel = new QLabel("Norm. Factor");
    QLabel *pExpLabel = new QLabel("Exp. Factor");
    QLabel *pDescriptionLabel = new QLabel("Description");
    QLabel *pDataLabel = new QLabel("Data");
    pWeightLabel->setFont(boldFont);
    pNormLabel->setFont(boldFont);
    pExpLabel->setFont(boldFont);
    pDescriptionLabel->setFont(boldFont);
    pDataLabel->setFont(boldFont);
    mpObjectiveLayout = new QGridLayout(this);
    mpObjectiveLayout->addWidget(pObjectiveLabel,          0, 0, 1, 7);
    mpObjectiveLayout->addWidget(mpVariablesList,           1, 0, 1, 7);
    mpObjectiveLayout->addWidget(mpMinMaxComboBox,          2, 0, 1, 1);
    mpObjectiveLayout->addWidget(mpFunctionsComboBox,       2, 1, 1, 4);
    mpObjectiveLayout->addWidget(mpAddFunctionButton,       2, 5, 1, 2);
    mpObjectiveLayout->addWidget(pWeightLabel,             3, 0, 1, 1);
    mpObjectiveLayout->addWidget(pNormLabel,               3, 1, 1, 1);
    mpObjectiveLayout->addWidget(pExpLabel,                3, 2, 1, 1);
    mpObjectiveLayout->addWidget(pDescriptionLabel,        3, 3, 1, 2);
    mpObjectiveLayout->addWidget(pDataLabel,               3, 5, 1, 2);
    mpObjectiveLayout->addWidget(new QWidget(this),         4, 0, 1, 7);
    mpObjectiveLayout->setRowStretch(0, 0);
    mpObjectiveLayout->setRowStretch(1, 0);
    mpObjectiveLayout->setRowStretch(2, 0);
    mpObjectiveLayout->setRowStretch(3, 0);
    mpObjectiveLayout->setRowStretch(4, 1);
    mpObjectiveLayout->setColumnStretch(0, 0);
    mpObjectiveLayout->setColumnStretch(1, 0);
    mpObjectiveLayout->setColumnStretch(2, 0);
    mpObjectiveLayout->setColumnStretch(3, 0);
    mpObjectiveLayout->setColumnStretch(4, 1);
    mpObjectiveLayout->setColumnStretch(5, 0);
    mpObjectiveLayout->setColumnStretch(6, 0);
    QWizardPage *pObjectiveWidget = new QWizardPage(this);
    pObjectiveWidget->setLayout(mpObjectiveLayout);

    //Output box tab
    mpOutputBox = new QTextEdit(this);
    HcomHighlighter *pHighligter = new HcomHighlighter(mpOutputBox->document());
    Q_UNUSED(pHighligter);
    QFont monoFont = mpOutputBox->font();
    monoFont.setFamily("Courier");
    monoFont.setPointSize(11);
    mpOutputBox->setFont(monoFont);
    mpOutputBox->setMinimumWidth(450);
    QGridLayout *pOutputLayout = new QGridLayout(this);
    pOutputLayout->addWidget(mpOutputBox);
    QWizardPage *pOutputWidget = new QWizardPage(this);
    pOutputWidget->setLayout(pOutputLayout);

    //Toolbar
    QToolButton *pHelpButton = new QToolButton(this);
    pHelpButton->setToolTip(tr("Show context help"));
    pHelpButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Help.png"));
    this->setButton(QWizard::HelpButton, pHelpButton);
    this->setOption(QWizard::HaveHelpButton);
    pHelpButton->setObjectName("optimizationHelpButton");

    //Run tab
    mpStartButton = new QPushButton("Start Optimization", this);
    connect(mpStartButton, SIGNAL(clicked(bool)), mpStartButton, SLOT(setEnabled(bool)));
    mpTotalProgressBar = new QProgressBar(this);
    mpTotalProgressBar->hide();
    mpCoreProgressBarsLayout = new QGridLayout();
    mpParametersOutputTextEditsLayout = new QGridLayout();
    mpTerminal = new TerminalWidget(this);
    QGridLayout *pRunLayout = new QGridLayout(this);
    pRunLayout->addWidget(mpStartButton,                         0,0,1,1);
    pRunLayout->addLayout(mpParametersOutputTextEditsLayout,    1,0,1,1);
    pRunLayout->addLayout(mpCoreProgressBarsLayout,             1,1,1,1);
    pRunLayout->addWidget(mpTerminal,                           2,0,1,2);
    pRunLayout->setRowStretch(2,1);
   // pRunLayout->addWidget(mpTotalProgressBar,           3,0,1,2);
    QWizardPage *pRunWidget = new QWizardPage(this);
    pRunWidget->setLayout(pRunLayout);

    this->addPage(pSettingsWidget);
    this->addPage(pParametersWidget);
    this->addPage(pObjectiveWidget);
    this->addPage(pOutputWidget);
    this->addPage(pRunWidget);

    setButtonText(QWizard::FinishButton, tr("&Close Dialog"));
    setButtonText(QWizard::CustomButton1, tr("&Save To Script File"));
    setOption(QWizard::HaveCustomButton1, true);
    setOption(QWizard::CancelButtonOnLeft, false);
    button(QWizard::CustomButton1)->setDisabled(true);
    button(QWizard::FinishButton)->setEnabled(true);
    button(QWizard::FinishButton)->setHidden(true);

    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), this, SLOT(updateCoreProgressBars()));
    mpTimer->setSingleShot(false);

    //Connections
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(update(int)));
    connect(mpAddFunctionButton,            SIGNAL(clicked()),      this,                   SLOT(addFunction()));
    connect(pHelpButton,                   SIGNAL(clicked()),    gpMainWindow,           SLOT(openContextHelp()));
    connect(mpStartButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(saveScriptFile()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveConfiguration()));
}

void OptimizationDialog::updateParameterOutputs(QVector< QVector<double> > &values, int bestId, int worstId)
{
    if(!this->isVisible()) return;

    for(int i=0; i<values.size(); ++i)
    {
        QString output="[ ";
        for(int j=0; j<values[i].size(); ++j)
        {
            output.append(QString::number(values[i][j])+" ");
        }
        output.append("]");
        QPalette palette;
        if(i == bestId)
            palette.setColor(QPalette::Text,Qt::darkGreen);
        else if(i == worstId)
            palette.setColor(QPalette::Text,Qt::darkRed);
        else
            palette.setColor(QPalette::Text,Qt::black);
        mParametersOutputLineEditPtrs[i]->setPalette(palette);
        mParametersOutputLineEditPtrs[i]->setText(output);
    }
}

void OptimizationDialog::updateTotalProgressBar(double progress)
{
    if(!this->isVisible()) return;

    mpTotalProgressBar->setValue(progress);
}

void OptimizationDialog::setOptimizationFinished()
{
    mpStartButton->setEnabled(true);
    for(int i=0; i<mParametersApplyButtonPtrs.size(); ++i)
    {
        mParametersApplyButtonPtrs[i]->setEnabled(true);
    }
}


void OptimizationDialog::loadConfiguration()
{
    OptimizationSettings optSettings;
    mpSystem->getOptimizationSettings(optSettings);

    mpIterationsSpinBox->setValue(optSettings.mNiter);
    mpSearchPointsSpinBox->setValue(optSettings.mNsearchp);
    mpAlphaLineEdit->setText(QString().setNum(optSettings.mRefcoeff));
    mpBetaLineEdit->setText(QString().setNum(optSettings.mRandfac));
    mpGammaLineEdit->setText(QString().setNum(optSettings.mForgfac));
    mpEpsilonFLineEdit->setText(QString().setNum(optSettings.mFunctol));
    mpEpsilonXLineEdit->setText(QString().setNum(optSettings.mPartol));
    mpPlottingCheckBox->setChecked(optSettings.mPlot);
    mpExport2CSVBox->setChecked(optSettings.mSavecsv);
    mpParametersLogCheckBox->setChecked(optSettings.mlogPar);

    //Parameters
    for(int i=0; i<optSettings.mParamters.size(); ++i)
    {
        //Check if component and parameter exists before checking the tree item (otherwise tree item does not exist = crash)
        if(gpModelHandler->getCurrentViewContainerObject()->hasModelObject(optSettings.mParamters.at(i).mComponentName) &&
           gpModelHandler->getCurrentViewContainerObject()->getModelObject(optSettings.mParamters.at(i).mComponentName)->getParameterNames().contains(optSettings.mParamters.at(i).mParameterName))
        {
            QTreeWidgetItem *pItem = findParameterTreeItem(optSettings.mParamters.at(i).mComponentName, optSettings.mParamters.at(i).mParameterName);
            if(!pItem->isDisabled())
            {
                pItem->setCheckState(0, Qt::Checked);
            }
        }
    }
    //Objectives
    for(int i=0; i<optSettings.mObjectives.size(); ++i)
    {
        //! @todo Find a good way of setting the objective functions

        int idx = mpFunctionsComboBox->findText(optSettings.mObjectives.at(i).mFunctionName);
        if(idx > -1) //found!
        {//Lägg till variabel i XML -> compname, portname, varname, ska vara i mSelectedVariables
            mpFunctionsComboBox->setCurrentIndex(idx);
            addObjectiveFunction(idx, optSettings.mObjectives.at(i).mWeight, optSettings.mObjectives.at(i).mNorm, optSettings.mObjectives.at(i).mExp, optSettings.mObjectives.at(i).mVariableInfo, optSettings.mObjectives.at(i).mData);
        }
    }
}


void OptimizationDialog::saveConfiguration()
{
    if(!mpSystem)
    {
        return;
    }

    OptimizationSettings optSettings;

    //Settings
    optSettings.mNiter = mpIterationsSpinBox->value();
    optSettings.mNsearchp = mpSearchPointsSpinBox->value();
    optSettings.mRefcoeff = mpAlphaLineEdit->text().toDouble();
    optSettings.mRandfac = mpBetaLineEdit->text().toDouble();
    optSettings.mForgfac = mpGammaLineEdit->text().toDouble();
    optSettings.mFunctol = mpEpsilonFLineEdit->text().toDouble();
    optSettings.mPartol = mpEpsilonXLineEdit->text().toDouble();
    optSettings.mPlot = mpPlottingCheckBox->isChecked();
    optSettings.mSavecsv = mpExport2CSVBox->isChecked();
    optSettings.mlogPar = mpParametersLogCheckBox->isChecked();

    //Parameters
    for(int i=0; i < mpParameterLabels.size(); ++i)
    {
        qDebug() << mSelectedParameters.at(i) << "   " << mSelectedComponents.at(i) << "   " << mpParameterMinLineEdits.at(i)->text();
        OptParameter parameter;
        parameter.mComponentName = mSelectedComponents.at(i);
        parameter.mParameterName = mSelectedParameters.at(i);
        parameter.mMax = mpParameterMaxLineEdits.at(i)->text().toDouble();
        parameter.mMin = mpParameterMinLineEdits.at(i)->text().toDouble();
        optSettings.mParamters.append(parameter);
    }
    //Objective functions
    for(int i=0; i < mWeightLineEditPtrs.size(); ++i)
    {
        Objectives objective;
        objective.mFunctionName = mFunctionName.at(i);
        objective.mWeight = mWeightLineEditPtrs.at(i)->text().toDouble();
        objective.mNorm   = mNormLineEditPtrs.at(i)->text().toDouble();
        objective.mExp    = mExpLineEditPtrs.at(i)->text().toDouble();

        for(int j=0; j < mFunctionComponents.at(i).size(); ++j)
        {
            QStringList variableInfo;

            variableInfo << mFunctionComponents.at(i).at(j);
            variableInfo << mFunctionPorts.at(i).at(j);
            variableInfo << mFunctionVariables.at(i).at(j);

            objective.mVariableInfo.append(variableInfo);
        }

        for(int j=0; j < mDataLineEditPtrs.at(i).size(); ++j)
        {
            objective.mData.append(mDataLineEditPtrs.at(i).at(j)->text());
        }
        optSettings.mObjectives.append(objective);
    }

    mpSystem->setOptimizationSettings(optSettings);
}


//! @brief Reimplementation of open() slot, used to initialize the dialog
void OptimizationDialog::open()
{
    mpSystem = gpModelHandler->getCurrentTopLevelSystem();
    connect(mpSystem, SIGNAL(destroyed()), this, SLOT(close()));

    //Load the objective functions
    if(!loadObjectiveFunctions())
        return;

    mpFunctionsComboBox->clear();
    mpFunctionsComboBox->addItems(mObjectiveFunctionDescriptions);

    //Clear all parameters
    for(int c=0; c<mpParametersList->topLevelItemCount(); ++c)      //Uncheck all parameters (will "remove" them)
    {
        for(int p=0; p<mpParametersList->topLevelItem(c)->childCount(); ++p)
        {
            if(mpParametersList->topLevelItem(c)->child(p)->checkState(0) == Qt::Checked)
            {
                mpParametersList->topLevelItem(c)->child(p)->setCheckState(0, Qt::Unchecked);
            }
        }
    }
    mpParametersList->clear();

    //Populate parameters list
    QStringList componentNames = mpSystem->getModelObjectNames();
    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpParametersList->insertTopLevelItem(0, pComponentItem);
        QStringList parameterNames = mpSystem->getModelObject(componentNames.at(c))->getParameterNames();
        for(int p=0; p<parameterNames.size(); ++p)
        {
            QTreeWidgetItem *pParameterItem = new QTreeWidgetItem(QStringList() << parameterNames.at(p));
            pParameterItem->setCheckState(0, Qt::Unchecked);
            Port *pPort = mpSystem->getModelObject(componentNames.at(c))->getPort(parameterNames[p].remove("#Value"));
            if (pPort && pPort->isConnected())
            {
                pParameterItem->setTextColor(0, QColor("gray"));
                pParameterItem->setDisabled(true);
                QFont italicFont = pParameterItem->font(0);
                italicFont.setItalic(true);
                pParameterItem->setFont(0, italicFont);
            }
            pComponentItem->insertChild(0, pParameterItem);
        }
    }
    QTreeWidgetItem *pSystemParametersItem = new QTreeWidgetItem(QStringList() << "_System Parameters");
    QFont componentFont = pSystemParametersItem->font(0);
    componentFont.setBold(true);
    pSystemParametersItem->setFont(0, componentFont);
    mpParametersList->insertTopLevelItem(0, pSystemParametersItem);
    QStringList parameterNames = mpSystem->getParameterNames();
    for(int p=0; p<parameterNames.size(); ++p)
    {
        QTreeWidgetItem *pParameterItem = new QTreeWidgetItem(QStringList() << parameterNames.at(p));
        pParameterItem->setCheckState(0, Qt::Unchecked);
        pSystemParametersItem->insertChild(0, pParameterItem);
    }
    mpParametersList->sortItems(0,Qt::AscendingOrder);
    mpParametersList->sortItems(1,Qt::AscendingOrder);
    connect(mpParametersList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(updateChosenParameters(QTreeWidgetItem*,int)), Qt::UniqueConnection);

    //Clear all objective functions
    mpVariablesList->clear();
    QTreeWidgetItem *pAliasItem = new QTreeWidgetItem(QStringList() << "_Alias");
    QFont aliasFont = pAliasItem->font(0);
    aliasFont.setBold(true);
    pAliasItem->setFont(0, aliasFont);
    mpVariablesList->insertTopLevelItem(0, pAliasItem);
    QStringList aliasNames = mpSystem->getAliasNames();
    for(int a=0; a<aliasNames.size(); ++a)
    {
        QTreeWidgetItem *pVariableItem = new QTreeWidgetItem(QStringList() << aliasNames.at(a));
        pVariableItem->setCheckState(0, Qt::Unchecked);
        pAliasItem->insertChild(0, pVariableItem);
    }

    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpVariablesList->insertTopLevelItem(0, pComponentItem);
        QList<Port*> ports = mpSystem->getModelObject(componentNames.at(c))->getPortListPtrs();
        for(int p=0; p<ports.size(); ++p)
        {
            QTreeWidgetItem *pPortItem = new QTreeWidgetItem(QStringList() << ports.at(p)->getName());
            QVector<QString> varNames, portUnits;
            mpSystem->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(componentNames.at(c), ports.at(p)->getName(), varNames, portUnits);
            for(int v=0; v<varNames.size(); ++v)
            {
                QTreeWidgetItem *pVariableItem = new QTreeWidgetItem(QStringList() << varNames.at(v));
                pVariableItem->setCheckState(0, Qt::Unchecked);
                pPortItem->insertChild(0, pVariableItem);
            }
            pComponentItem->insertChild(0, pPortItem);
        }
    }
    mpVariablesList->sortItems(0,Qt::AscendingOrder);
    mpVariablesList->sortItems(1,Qt::AscendingOrder);
    mpVariablesList->sortItems(2,Qt::AscendingOrder);
    connect(mpVariablesList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(updateChosenVariables(QTreeWidgetItem*,int)), Qt::UniqueConnection);

    mSelectedVariables.clear();
    mSelectedFunctionsMinMax.clear();
    mSelectedFunctions.clear();
    mSelectedParameters.clear();
    mSelectedComponents.clear();

    for(int i=0; i<mWeightLineEditPtrs.size(); ++i)
    {
        mpObjectiveLayout->removeWidget(mWeightLineEditPtrs.at(i));
        mpObjectiveLayout->removeWidget(mNormLineEditPtrs.at(i));
        mpObjectiveLayout->removeWidget(mExpLineEditPtrs.at(i));
        mpObjectiveLayout->removeWidget(mFunctionLabelPtrs.at(i));
        mpObjectiveLayout->removeWidget(mRemoveFunctionButtonPtrs.at(i));
        mpObjectiveLayout->removeWidget(mDataWidgetPtrs.at(i));

        for(int j=0; j<mDataLineEditPtrs.at(i).size(); ++j)
        {
            delete(mDataLineEditPtrs.at(i).at(j));
        }
        delete(mWeightLineEditPtrs.at(i));
        delete(mNormLineEditPtrs.at(i));
        delete(mExpLineEditPtrs.at(i));
        delete(mFunctionLabelPtrs.at(i));
        delete(mRemoveFunctionButtonPtrs.at(i));
        delete(mDataWidgetPtrs.at(i));
    }

    mWeightLineEditPtrs.clear();
    mNormLineEditPtrs.clear();
    mExpLineEditPtrs.clear();
    mFunctionLabelPtrs.clear();
    mFunctionName.clear();
    mRemoveFunctionButtonPtrs.clear();
    mDataLineEditPtrs.clear();
    mDataWidgetPtrs.clear();
    mFunctionComponents.clear();
    mFunctionPorts.clear();
    mFunctionVariables.clear();

    for(int i=0; i<mpParameterLabels.size(); ++i)
    {
        mpParametersLayout->removeWidget(mpParameterLabels.at(i));
        mpParametersLayout->removeWidget(mpParameterMinLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterMaxLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterRemoveButtons.at(i));

        delete(mpParameterLabels.at(i));
        delete(mpParameterMinLineEdits.at(i));
        delete(mpParameterMaxLineEdits.at(i));
        delete(mpParameterRemoveButtons.at(i));
    }

    mpParameterLabels.clear();
    mpParameterMinLineEdits.clear();
    mpParameterMaxLineEdits.clear();
    mpParameterRemoveButtons.clear();

    mScript.clear();
    mpOutputBox->clear();
    //mpRunButton->setDisabled(true);

    loadConfiguration();

    recreateCoreProgressBars();
    recreateParameterOutputLineEdits();

    QDialog::show();
}


void OptimizationDialog::okPressed()
{
    saveConfiguration();

    reject();
}


//! @brief Generates the Python script based upon selections made in the dialog
void OptimizationDialog::generateScriptFile()
{
    if(mpParametersLogCheckBox->isChecked())
    {
        bool itsOk = true;
        for(int i=0; i<mpParameterMinLineEdits.size(); ++i)
        {
            if(mpParameterMinLineEdits[i]->text().toDouble() <= 0 || mpParameterMinLineEdits[i]->text().toDouble() <= 0)
            {
                itsOk = false;
            }
        }
        if(!itsOk)
        {
            QMessageBox::warning(this, "Warning", "Logarithmic scaling is selected, but parameters are allowed to be negative or zero. This will probably not work.");
        }
    }

    if(mSelectedParameters.isEmpty())
    {
        gpTerminalWidget->mpConsole->printErrorMessage("No parameters specified for optimization.");
        return;
    }

    if(mSelectedFunctions.isEmpty())
    {
        gpTerminalWidget->mpConsole->printErrorMessage("No objective functions specified for optimization.");
        return;
    }

    switch (mpAlgorithmBox->currentIndex())
    {
    case 0 :
        generateComplexScript();
        break;
    case 1 :
        generateParticleSwarmScript();
        break;
    default :
        gpTerminalWidget->mpConsole->printErrorMessage("Algorithm type undefined.");
    }
}

void OptimizationDialog::generateComplexScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateComplex.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    QString objFuncs;
    QString totalObj;
    QString objPars;
    //QStringList plotVarsList;
    //! @todo Reimplement plotting variables support
    //QString plotVars = "chpv ";
    QString setMinMax;
    QString setPars;
    for(int i=0; i<mFunctionName.size(); ++i)
    {
        QString objFunc = mObjectiveFunctionCalls[mObjectiveFunctionDescriptions.indexOf(mFunctionName[i])];
        objFunc.prepend("    ");
        objFunc.replace("\n", "\n    ");
        objFunc.replace("<<<id>>>", QString::number(i+1));
        for(int j=0; j<mFunctionComponents[i].size(); ++j)
        {
            QString varName;
            if(mFunctionComponents[i][j].isEmpty())   //Alias
            {
                varName = mFunctionVariables[i][j];
            }
            else
            {
                varName = mFunctionComponents[i][j]+"."+mFunctionPorts[i][j]+"."+mFunctionVariables[i][j];
            }
            gpTerminalWidget->mpHandler->toShortDataNames(varName);
            objFunc.replace("<<<var"+QString::number(j+1)+">>>", varName);

//            if(!plotVarsList.contains(varName))
//            {
//                plotVarsList.append(varName);
//                plotVars.append(varName+" ");
//            }
        }
        for(int j=0; j<mDataLineEditPtrs[i].size(); ++j)
        {
            objFunc.replace("<<<arg"+QString::number(j+1)+">>>", mDataLineEditPtrs[i][j]->text());
        }
        objFuncs.append(objFunc+"\n");

        if(mSelectedFunctionsMinMax.at(i) == "Minimize")
        {
            totalObj.append("+");
        }
        else
        {
            totalObj.append("-");
        }
        QString idx = QString::number(i+1);
        totalObj.append(mWeightLineEditPtrs[i]->text()+"*"+mNormLineEditPtrs[i]->text()+"*exp("+mExpLineEditPtrs[i]->text()+")*obj"+idx);

//        totalObj.append("w"+idx+"*r"+idx+"*exp(e"+idx+")*obj"+idx);

//        objPars.append("w"+idx+"="+mWeightLineEditPtrs[i]->text()+"\n");
//        objPars.append("r"+idx+"="+mNormLineEditPtrs[i]->text()+"\n");
//        objPars.append("e"+idx+"="+mExpLineEditPtrs[i]->text()+"\n");
    }
    objFuncs.chop(1);
   // objPars.chop(1);

    for(int p=0; p<mSelectedParameters.size(); ++p)
    {
        QString par;
        if(mSelectedComponents[p] == "_System Parameters")
        {
            par = mSelectedParameters[p];
        }
        else
        {
            par = mSelectedComponents[p]+"."+mSelectedParameters[p];
        }
        gpTerminalWidget->mpHandler->toShortDataNames(par);
        setPars.append("    chpa "+par+" optpar(optvar(evalid),"+QString::number(p)+")\n");

        setMinMax.append("opt set limits "+QString::number(p)+" "+mpParameterMinLineEdits[p]->text()+" "+mpParameterMaxLineEdits[p]->text()+"\n");
    }
    setPars.chop(1);
    setMinMax.chop(1);

    QString extraPlots;
    if(mpPlotParticlesCheckBox->isChecked())
    {
        extraPlots.append("opt set plotpoints on\n");
    }
    if(mpPlotBestWorstCheckBox->isChecked())
    {
        extraPlots.append("opt set plotbestworst on\n");
    }
    extraPlots.chop(1);


    templateCode.replace("<<<objfuncs>>>", objFuncs);
    templateCode.replace("<<<totalobj>>>", totalObj);
   // templateCode.replace("<<<objpars>>>", objPars);
//    if(mpPlottingCheckBox->isChecked())
//    {
//        templateCode.replace("<<<plotvars>>>", plotVars);
//    }
//    else
//    {
        templateCode.replace("<<<plotvars>>>", "");
    //}
    templateCode.replace("<<<extraplots>>>", extraPlots);
    templateCode.replace("<<<setminmax>>>", setMinMax);
    templateCode.replace("<<<setpars>>>", setPars);
    templateCode.replace("<<<npoints>>>", QString::number(mpSearchPointsSpinBox->value()));
    templateCode.replace("<<<nparams>>>", QString::number(mSelectedParameters.size()));
    templateCode.replace("<<<maxevals>>>", QString::number(mpIterationsSpinBox->value()));
    templateCode.replace("<<<alpha>>>", mpAlphaLineEdit->text());
    templateCode.replace("<<<rfak>>>", mpBetaLineEdit->text());
    templateCode.replace("<<<gamma>>>", mpGammaLineEdit->text());
    templateCode.replace("<<<functol>>>", mpEpsilonFLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());

    mScript = templateCode;
}


void OptimizationDialog::generateParticleSwarmScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateParticle.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    QString objFuncs;
    QString totalObj;
    QString objPars;
    //! @todo Reimplement plotting variables support
    //QStringList plotVarsList;
    //QString plotVars="chpv ";
    QString setMinMax;
    QString setPars;
    for(int i=0; i<mFunctionName.size(); ++i)
    {
        QString objFunc = mObjectiveFunctionCalls[mObjectiveFunctionDescriptions.indexOf(mFunctionName[i])];
        objFunc.prepend("    ");
        objFunc.replace("\n", "\n    ");
        objFunc.replace("<<<id>>>", QString::number(i+1));
        for(int j=0; j<mFunctionComponents[i].size(); ++j)
        {
            QString varName;
            if(mFunctionComponents[i][j].isEmpty())   //Alias
            {
                varName = mFunctionVariables[i][j];
            }
            else
            {
                varName = mFunctionComponents[i][j]+"."+mFunctionPorts[i][j]+"."+mFunctionVariables[i][j];
            }
            gpTerminalWidget->mpHandler->toShortDataNames(varName);
            objFunc.replace("<<<var"+QString::number(j+1)+">>>", varName);

//            if(!plotVarsList.contains(varName))
//            {
//                plotVarsList.append(varName);
//                plotVars.append(varName+" ");
//            }
        }
        for(int j=0; j<mDataLineEditPtrs[i].size(); ++j)
        {
            objFunc.replace("<<<arg"+QString::number(j+1)+">>>", mDataLineEditPtrs[i][j]->text());
        }
        objFuncs.append(objFunc+"\n");

        if(mSelectedFunctionsMinMax.at(i) == "Minimize")
        {
            totalObj.append("+");
        }
        else
        {
            totalObj.append("-");
        }
        QString idx = QString::number(i+1);
        totalObj.append("w"+idx+"*r"+idx+"*exp(e"+idx+")*obj"+idx);

        objPars.append("w"+idx+"="+mWeightLineEditPtrs[i]->text()+"\n");
        objPars.append("r"+idx+"="+mNormLineEditPtrs[i]->text()+"\n");
        objPars.append("e"+idx+"="+mExpLineEditPtrs[i]->text()+"\n");

    }

    for(int p=0; p<mSelectedParameters.size(); ++p)
    {
        QString par = mSelectedComponents[p]+"."+mSelectedParameters[p];
        gpTerminalWidget->mpHandler->toShortDataNames(par);
        setPars.append("    chpa "+par+" optpar(optvar(evalid),"+QString::number(p)+")\n");

        setMinMax.append("opt set limits "+QString::number(p)+" "+mpParameterMinLineEdits[p]->text()+" "+mpParameterMaxLineEdits[p]->text()+"\n");
    }

    QString extraPlots;
    if(mpPlotParticlesCheckBox->isChecked())
    {
        extraPlots.append("opt set plotpoints on\n");
    }
    if(mpPlotBestWorstCheckBox->isChecked())
    {
        extraPlots.append("opt set plotbestworst on\n");
    }

    templateCode.replace("<<<objfuncs>>>", objFuncs);
    templateCode.replace("<<<totalobj>>>", totalObj);
    templateCode.replace("<<<objpars>>>", objPars);
//    if(mpPlottingCheckBox->isChecked())
//    {
//        templateCode.replace("<<<plotvars>>>", plotVars);
//    }
//    else
//    {
        templateCode.replace("<<<plotvars>>>", "");
   // }
    templateCode.replace("<<<extraplots>>>", extraPlots);
    templateCode.replace("<<<setminmax>>>", setMinMax);
    templateCode.replace("<<<setpars>>>", setPars);
    templateCode.replace("<<<npoints>>>", QString::number(mpParticlesSpinBox->value()));
    templateCode.replace("<<<nparams>>>", QString::number(mSelectedParameters.size()));
    templateCode.replace("<<<maxevals>>>", QString::number(mpIterationsSpinBox->value()));
    templateCode.replace("<<<omega>>>", mpOmegaLineEdit->text());
    templateCode.replace("<<<c1>>>", mpC1LineEdit->text());
    templateCode.replace("<<<c2>>>", mpC2LineEdit->text());
    templateCode.replace("<<<functol>>>", mpEpsilonFLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());

    mScript = templateCode;
}


void OptimizationDialog::setAlgorithm(int i)
{
    //Complex
    mpSearchPointsLabel->setVisible(i==0);
    mpSearchPointsSpinBox->setVisible(i==0);
    mpAlphaLabel->setVisible(i==0);
    mpAlphaLineEdit->setVisible(i==0);
    mpBetaLabel->setVisible(i==0);
    mpBetaLineEdit->setVisible(i==0);
    mpGammaLabel->setVisible(i==0);
    mpGammaLineEdit->setVisible(i==0);

    //Particle swarm
    mpParticlesLabel->setVisible(i==1);
    mpParticlesSpinBox->setVisible(i==1);
    mpOmegaLabel->setVisible(i==1);
    mpOmegaLineEdit->setVisible(i==1);
    mpC1Label->setVisible(i==1);
    mpC1LineEdit->setVisible(i==1);
    mpC2Label->setVisible(i==1);
    mpC2LineEdit->setVisible(i==1);
}


//! @brief Adds a new parameter to the list of selected parameter, and displays it in dialog
//! @param item Tree widget item which represents parameter
void OptimizationDialog::updateChosenParameters(QTreeWidgetItem* item, int /*i*/)
{
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedComponents.append(item->parent()->text(0));
        mSelectedParameters.append(item->text(0));
        QString currentValue;
        if(item->parent()->text(0) == "_System Parameters")
        {
            currentValue = mpSystem->getParameterValue(item->text(0));
        }
        else
        {
            currentValue = mpSystem->getModelObject(item->parent()->text(0))->getParameterValue(item->text(0));
        }

        QLabel *pLabel = new QLabel(trUtf8(" <  ") + item->parent()->text(0) + ", " + item->text(0) + " (" + currentValue + trUtf8(")  < "));
        pLabel->setAlignment(Qt::AlignCenter);

        OptimizationSettings optSettings;
        mpSystem->getOptimizationSettings(optSettings);
        QString min, max;
        for(int i=0; i<optSettings.mParamters.size(); ++i)
        {
            if(item->parent()->text(0) == optSettings.mParamters.at(i).mComponentName)
            {
                if(item->text(0) == optSettings.mParamters.at(i).mParameterName)
                {
                    min.setNum(optSettings.mParamters.at(i).mMin);
                    max.setNum(optSettings.mParamters.at(i).mMax);
                }
            }
        }
        if(min == "")
        {
            min = "0.0";
        }
        if(max == "")
        {
            max = "1.0";
        }


        QLineEdit *pMinLineEdit = new QLineEdit(min, this);
        pMinLineEdit->setValidator(new QDoubleValidator());
        QLineEdit *pMaxLineEdit = new QLineEdit(max, this);
        pMaxLineEdit->setValidator(new QDoubleValidator());
        QToolButton *pRemoveButton = new QToolButton(this);
        pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
        pRemoveButton->setToolTip("Remove Parameter");
        connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeParameter()));

        mpParameterLabels.append(pLabel);
        mpParameterMinLineEdits.append(pMinLineEdit);
        mpParameterMaxLineEdits.append(pMaxLineEdit);
        mpParameterRemoveButtons.append(pRemoveButton);

        int row = mpParametersLayout->rowCount();
        mpParametersLayout->addWidget(pMinLineEdit, row, 0);
        mpParametersLayout->addWidget(pLabel, row, 1);
        mpParametersLayout->addWidget(pMaxLineEdit, row, 2);
        mpParametersLayout->addWidget(pRemoveButton, row, 3);
    }
    else
    {
        int i=0;
        for(; i<mSelectedParameters.size(); ++i)
        {
            if(mSelectedComponents.at(i) == item->parent()->text(0) &&
               mSelectedParameters.at(i) == item->text(0))
            {
                break;
            }
        }
        mpParametersLayout->removeWidget(mpParameterLabels.at(i));
        mpParametersLayout->removeWidget(mpParameterMinLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterMaxLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterRemoveButtons.at(i));

        delete(mpParameterLabels.at(i));
        delete(mpParameterMinLineEdits.at(i));
        delete(mpParameterMaxLineEdits.at(i));
        delete(mpParameterRemoveButtons.at(i));

        mpParameterLabels.removeAt(i);
        mpParameterMinLineEdits.removeAt(i);
        mpParameterMaxLineEdits.removeAt(i);
        mpParameterRemoveButtons.removeAt(i);

        mSelectedParameters.removeAt(i);
        mSelectedComponents.removeAt(i);
    }
}


QTreeWidgetItem* OptimizationDialog::findParameterTreeItem(QString componentName, QString parameterName)
{
    QTreeWidgetItem* foundItem=0;

    for(int c=0; c<mpParametersList->topLevelItemCount(); ++c)      //Uncheck the parameter in the list before removing it
    {
        if(mpParametersList->topLevelItem(c)->text(0) == componentName)
        {
            bool doBreak = false;
            for(int p=0; p<mpParametersList->topLevelItem(c)->childCount(); ++p)
            {
                if(mpParametersList->topLevelItem(c)->child(p)->text(0) == parameterName)
                {
                    foundItem = mpParametersList->topLevelItem(c)->child(p);
                    doBreak = true;
                    break;
                }
            }
            if(doBreak)
                return foundItem;
        }
    }

    return 0;
}


//! @brief Removes an objevtive function from the selected functions
void OptimizationDialog::removeParameter()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mpParameterRemoveButtons.indexOf(button);

    QTreeWidgetItem *selectedItem = findParameterTreeItem(mSelectedComponents.at(i), mSelectedParameters.at(i));
    if(selectedItem)
    {
        selectedItem->setCheckState(0, Qt::Unchecked);
    }
    else
    {

    //Parameter is not in list (should not really happen), so remove it here instead
    mpParametersLayout->removeWidget(mpParameterLabels.at(i));
    mpParametersLayout->removeWidget(mpParameterMinLineEdits.at(i));
    mpParametersLayout->removeWidget(mpParameterMaxLineEdits.at(i));
    mpParametersLayout->removeWidget(mpParameterRemoveButtons.at(i));

    delete(mpParameterLabels.at(i));
    delete(mpParameterMinLineEdits.at(i));
    delete(mpParameterMaxLineEdits.at(i));
    delete(mpParameterRemoveButtons.at(i));

    mpParameterLabels.removeAt(i);
    mpParameterMinLineEdits.removeAt(i);
    mpParameterMaxLineEdits.removeAt(i);
    mpParameterRemoveButtons.removeAt(i);

    mSelectedParameters.removeAt(i);
    mSelectedComponents.removeAt(i);
    }
}


//! @brief Adds a new variable to the list of selected variables
//! @param item Tree widget item which represents variable
void OptimizationDialog::updateChosenVariables(QTreeWidgetItem *item, int /*i*/)
{
    QStringList variable;
    if(item->parent()->text(0) == "_Alias")
    {
        variable << "" << "" << item->text(0);
    }
    else
    {
        variable << item->parent()->parent()->text(0) << item->parent()->text(0) << item->text(0);
    }
    mSelectedVariables.removeAll(variable);
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedVariables.append(variable);
    }
}


//! @brief Adds a new objective function from combo box and selected variables
void OptimizationDialog::addFunction()
{
    int idx = mpFunctionsComboBox->currentIndex();
    addObjectiveFunction(idx, 1.0, 1.0, 2.0, mSelectedVariables, QStringList());
}


//! @brief Adds a new objective function
void OptimizationDialog::addObjectiveFunction(int idx, double weight, double norm, double exp, QList<QStringList> selectedVariables, QStringList objData)
{
    if(!verifyNumberOfVariables(idx, selectedVariables.size()))
        return;

    QStringList data = mObjectiveFunctionDataLists.at(idx);

    mSelectedFunctionsMinMax.append(mpMinMaxComboBox->currentText());
    mSelectedFunctions.append(idx);
    mFunctionComponents.append(QStringList());
    mFunctionPorts.append(QStringList());
    mFunctionVariables.append(QStringList());
    for(int i=0; i<selectedVariables.size(); ++i)
    {
        mFunctionComponents.last().append(selectedVariables.at(i).at(0));
        mFunctionPorts.last().append(selectedVariables.at(i).at(1));
        mFunctionVariables.last().append(selectedVariables.at(i).at(2));
    }

    QLineEdit *pWeightLineEdit = new QLineEdit(QString().setNum(weight), this);
    pWeightLineEdit->setValidator(new QDoubleValidator());
    QLineEdit *pNormLineEdit = new QLineEdit(QString().setNum(norm), this);
    pNormLineEdit->setValidator(new QDoubleValidator());
    QLineEdit *pExpLineEdit = new QLineEdit(QString().setNum(exp), this);
    pExpLineEdit->setValidator(new QDoubleValidator());

    QString variablesText;
    if(mFunctionComponents.last().first().isEmpty())
    {
        variablesText = "<b>"+mFunctionVariables.last().first()+"</b>";
    }
    else
    {
        variablesText = "<b>"+mFunctionComponents.last().first()+", "+mFunctionPorts.last().first()+", "+mFunctionVariables.last().first()+"</b>";
    }
    for(int i=1; i<mFunctionVariables.last().size(); ++i)
    {
        if(mFunctionComponents.last().at(i).isEmpty())
        {
            variablesText.append(" and <b>"+mFunctionVariables.last().at(i)+"</b>");
        }
        else
        {
            variablesText.append(" and <b>" + mFunctionComponents.last().at(i)+", "+mFunctionPorts.last().at(i)+", "+mFunctionVariables.last().at(i)+"</b>");
        }
    }
    QLabel *pFunctionLabel = new QLabel(mpMinMaxComboBox->currentText() + " " + mObjectiveFunctionDescriptions.at(idx)+" for "+variablesText, this);
    //QLabel::setTextFormat(Qt::AutoText);
    mFunctionName.append(mObjectiveFunctionDescriptions.at(idx));
    pFunctionLabel->setWordWrap(true);
    QWidget *pDataWidget = new QWidget(this);
    QGridLayout *pDataGrid = new QGridLayout(this);
    pDataWidget->setLayout(pDataGrid);
    QList<QLineEdit*> dummyList;
    for(int i=0; i<data.size(); ++i)
    {
        QString thisData;
        if(objData.size()<=i)
            thisData = "1.0";
        else
            thisData = objData.at(i);

        QLabel *pDataLabel = new QLabel(data.at(i), this);
        QLineEdit *pDataLineEdit = new QLineEdit(thisData, this);
        pDataLineEdit->setValidator(new QDoubleValidator());
        pDataGrid->addWidget(pDataLabel, i, 0);
        pDataGrid->addWidget(pDataLineEdit, i, 1);
        dummyList.append(pDataLineEdit);
    }
    mDataLineEditPtrs.append(dummyList);
    QToolButton *pRemoveButton = new QToolButton(this);
    pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
    pRemoveButton->setToolTip("Remove Function");
    pWeightLineEdit->setMaximumWidth(60);
    pNormLineEdit->setMaximumWidth(60);
    pExpLineEdit->setMaximumWidth(60);
    mWeightLineEditPtrs.append(pWeightLineEdit);
    mNormLineEditPtrs.append(pNormLineEdit);
    mExpLineEditPtrs.append(pExpLineEdit);
    mFunctionLabelPtrs.append(pFunctionLabel);
    mDataWidgetPtrs.append(pDataWidget);
    mRemoveFunctionButtonPtrs.append(pRemoveButton);

    int row = mpObjectiveLayout->rowCount()-1;
    mpObjectiveLayout->addWidget(pWeightLineEdit, row,   0, 1, 1);
    mpObjectiveLayout->addWidget(pNormLineEdit, row,     1, 1, 1);
    mpObjectiveLayout->addWidget(pExpLineEdit, row,      2, 1, 1);
    mpObjectiveLayout->addWidget(pFunctionLabel, row,   3, 1, 2);
    mpObjectiveLayout->addWidget(pDataWidget, row,      5, 1, 1);
    mpObjectiveLayout->addWidget(pRemoveButton, row,    6, 1, 1);
    mpObjectiveLayout->setRowStretch(row, 0);
    mpObjectiveLayout->setRowStretch(row+1, 1);
    mpObjectiveLayout->setColumnStretch(0, 0);
    mpObjectiveLayout->setColumnStretch(1, 0);
    mpObjectiveLayout->setColumnStretch(2, 0);
    mpObjectiveLayout->setColumnStretch(3, 1);
    mpObjectiveLayout->setColumnStretch(4, 0);
    mpObjectiveLayout->setColumnStretch(5, 0);
    mpObjectiveLayout->setColumnStretch(6, 0);

    connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeFunction()));
}


//! @brief Removes an objevtive function from the selected functions
void OptimizationDialog::removeFunction()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mRemoveFunctionButtonPtrs.indexOf(button);

    mpObjectiveLayout->removeWidget(mWeightLineEditPtrs.at(i));
    mpObjectiveLayout->removeWidget(mNormLineEditPtrs.at(i));
    mpObjectiveLayout->removeWidget(mExpLineEditPtrs.at(i));
    mpObjectiveLayout->removeWidget(mFunctionLabelPtrs.at(i));
    mpObjectiveLayout->removeWidget(mRemoveFunctionButtonPtrs.at(i));
    mpObjectiveLayout->removeWidget(mDataWidgetPtrs.at(i));

    for(int j=0; j<mDataLineEditPtrs.at(i).size(); ++j)
    {
        delete(mDataLineEditPtrs.at(i).at(j));
    }
    delete(mWeightLineEditPtrs.at(i));
    delete(mNormLineEditPtrs.at(i));
    delete(mExpLineEditPtrs.at(i));
    delete(mFunctionLabelPtrs.at(i));
    delete(mRemoveFunctionButtonPtrs.at(i));
    delete(mDataWidgetPtrs.at(i));

    mWeightLineEditPtrs.removeAt(i);
    mNormLineEditPtrs.removeAt(i);
    mExpLineEditPtrs.removeAt(i);
    mFunctionLabelPtrs.removeAt(i);
    mFunctionName.removeAt(i);
    mRemoveFunctionButtonPtrs.removeAt(i);
    mDataLineEditPtrs.removeAt(i);
    mDataWidgetPtrs.removeAt(i);
    mSelectedFunctions.removeAt(i);
    mSelectedFunctionsMinMax.removeAt(i);
    mFunctionComponents.removeAt(i);
    mFunctionPorts.removeAt(i);
    mFunctionVariables.removeAt(i);

}


//! @brief Generates the script code and shows it in the output box
void OptimizationDialog::update(int idx)
{
    //Finished parameters tab
    if(idx == 2)
    {
        if(mSelectedParameters.isEmpty())
        {
            gpTerminalWidget->mpConsole->printErrorMessage("No parameters specified for optimization.");
            this->back();
            return;
        }
    }

    //Finished objective function tab
    if(idx == 3)
    {
        if(mSelectedFunctions.isEmpty())
        {
            gpTerminalWidget->mpConsole->printErrorMessage("No objective functions specified for optimization.");
            this->back();
            return;
        }
        else
        {
            button(QWizard::CustomButton1)->setDisabled(false);
            generateScriptFile();
            mpOutputBox->clear();
            mpOutputBox->insertPlainText(mScript);
            saveConfiguration();
            return;
        }
    }
}



//! @brief Saves the generated script code to file and executes the script
void OptimizationDialog::run()
{
    saveConfiguration();

    recreateCoreProgressBars();
    recreateParameterOutputLineEdits();

    for(int i=0; i<mParametersApplyButtonPtrs.size(); ++i)
    {
        mParametersApplyButtonPtrs[i]->setEnabled(false);
    }

    QStringList commands = mpOutputBox->toPlainText().split("\n");
    bool *abort = new bool;
    mpTerminal->setEnabledAbortButton(true);
    mpTimer->start(10);
    mpTerminal->mpHandler->setModelPtr(gpModelHandler->getCurrentModel());
    mpTerminal->mpHandler->runScriptCommands(commands, abort);
    mpTerminal->setEnabledAbortButton(false);
    mpTimer->stop();
    delete(abort);
}


//! @brief Saves generated script to a script file
void OptimizationDialog::saveScriptFile()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Script File"),
                                                 gpConfig->getScriptDir(),
                                                 this->tr("HCOM Script (*.hcom)"));

    if(filePath.isEmpty())     //Don't save anything if user presses cancel
    {
        return;
    }

    QFileInfo fileInfo = QFileInfo(filePath);
    gpConfig->setScriptDir(fileInfo.absolutePath());

    QFile file(filePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&file);
    out << mpOutputBox->toPlainText();
    file.close();
}

void OptimizationDialog::updateCoreProgressBars()
{
    for(int p=0; p<mCoreProgressBarPtrs.size(); ++p)
    {
        OptimizationHandler *pOptHandler = mpTerminal->mpHandler->mpOptHandler;
        if(pOptHandler)
        {
            if(pOptHandler->mModelPtrs.size() > p)
            {
                ModelWidget *pOptModel = pOptHandler->mModelPtrs[p];
                double stopT = pOptModel->getStopTime().toDouble();
                if(pOptModel)
                {
                    CoreSystemAccess *pCoreSystem = pOptModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr();
                    mCoreProgressBarPtrs[p]->setValue(pCoreSystem->getCurrentTime() / stopT *100);
                }
            }
        }
    }
}

void OptimizationDialog::recreateCoreProgressBars()
{
    //Clear all previous stuff
    QLayoutItem *item;
    while((item = mpCoreProgressBarsLayout->takeAt(0)))
    {
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }
    mCoreProgressBarPtrs.clear();

    //Add new stuff depending on algorithm and number of threads
    switch (mpAlgorithmBox->currentIndex())
    {
    case 0 :    //Complex
        mCoreProgressBarPtrs.append(new QProgressBar(this));
        mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
        mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        break;
    case 1 :    //Particle swarm
        if(gpConfig->getUseMulticore())
        {
            for(int n=0; n<mpParticlesSpinBox->value(); ++n)
            {
                mCoreProgressBarPtrs.append(new QProgressBar(this));
                mpCoreProgressBarsLayout->addWidget(new QLabel("Particle "+QString::number(n)+":", this), n, 0);
                mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(), n, 1);
            }
        }
        else
        {
            mCoreProgressBarPtrs.append(new QProgressBar(this));
            mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
            mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        }
        break;
    default:
        break;
    }

    mpTotalProgressBar = new QProgressBar(this);
    mpCoreProgressBarsLayout->addWidget(new QLabel("Total: ", this),mCoreProgressBarPtrs.size(), 0);
    mpCoreProgressBarsLayout->addWidget(mpTotalProgressBar, mCoreProgressBarPtrs.size(), 1);
    mpCoreProgressBarsLayout->addWidget(new QWidget(this), mCoreProgressBarPtrs.size()+1, 0, 1, 2);
    mpCoreProgressBarsLayout->setRowStretch(mCoreProgressBarPtrs.size()+1, 1);
    return;
}

void OptimizationDialog::recreateParameterOutputLineEdits()
{
    int nPoints;
    switch(mpAlgorithmBox->currentIndex())
    {
    case 0:     //Complex
        nPoints=mpSearchPointsSpinBox->value();
        break;
    case 1:
        nPoints=mpParticlesSpinBox->value();
        break;
    default:
        nPoints=0;
    }

    while(mParametersOutputLineEditPtrs.size() < nPoints)
    {
        mParametersOutputLineEditPtrs.append(new QLineEdit(this));
        mParametersApplyButtonPtrs.append(new QPushButton("Apply", this));
        mpParametersOutputTextEditsLayout->addWidget(mParametersApplyButtonPtrs.last(), mParametersOutputLineEditPtrs.size(), 0);
        mpParametersOutputTextEditsLayout->addWidget(mParametersOutputLineEditPtrs.last(), mParametersOutputLineEditPtrs.size(), 1);
        mParametersApplyButtonPtrs.last()->setEnabled(false);
        connect(mParametersApplyButtonPtrs.last(), SIGNAL(clicked()), this, SLOT(applyParameters()));
    }
    while(mParametersOutputLineEditPtrs.size() > nPoints)
    {
        mpParametersOutputTextEditsLayout->removeWidget(mParametersOutputLineEditPtrs.last());
        mpParametersOutputTextEditsLayout->removeWidget(mParametersApplyButtonPtrs.last());
        delete mParametersOutputLineEditPtrs.last();
        delete mParametersApplyButtonPtrs.last();
        mParametersOutputLineEditPtrs.removeLast();
        mParametersApplyButtonPtrs.removeLast();
    }
}


//! @brief Slot that applies the parameters in a point to the original model. Index of the point is determined by the sender of the signal.
void OptimizationDialog::applyParameters()
{
    QPushButton *pSender = qobject_cast<QPushButton*>(QObject::sender());
    if(!pSender) return;
    int idx = mParametersApplyButtonPtrs.indexOf(pSender);

    if(gpModelHandler->count() == 0 || !gpModelHandler->getCurrentModel())
    {
        QMessageBox::critical(this, "Error", "No model is open.");
        return;
    }

     //Temporary switch optimization handler in global HCOM handler to this
    OptimizationHandler *pOrgOptHandler = gpTerminalWidget->mpHandler->mpOptHandler;
    gpTerminalWidget->mpHandler->mpOptHandler = mpTerminal->mpHandler->mpOptHandler;

    QStringList code;
    mpTerminal->mpHandler->getFunctionCode("setpars", code);
    bool abort;
    gpTerminalWidget->mpHandler->runScriptCommands(QStringList() << "optvar(evalid) = "+QString::number(idx), &abort);
    gpTerminalWidget->mpHandler->runScriptCommands(code, &abort);

    //Switch back HCOM handler
    gpTerminalWidget->mpHandler->mpOptHandler = pOrgOptHandler;
}


//! @brief Checks if number of selected variables is correct. Gives error messages if they are too many or too low.
//! @param i Selected objective function
bool OptimizationDialog::verifyNumberOfVariables(int idx, int nSelVar)
{
    int nVar = mObjectiveFunctionNumberOfVariables.at(idx);

    if(nSelVar > nVar)
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Too many variables selected for this function.");
        return false;
    }
    else if(nSelVar < nVar)
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Too few variables selected for this function.");
        return false;
    }
    return true;
}


bool OptimizationDialog::loadObjectiveFunctions()
{
    mObjectiveFunctionDescriptions.clear();
    mObjectiveFunctionCalls.clear();
    mObjectiveFunctionDataLists.clear();
    mObjectiveFunctionNumberOfVariables.clear();
    mObjectiveFunctionUsesTimeVector.clear();

    // Look in both local and global scripts directory in case they are different

    //QDir scriptsDir(gDesktopHandler.getExecPath()+"../Scripts/HCOM/objFuncTemplates");
    QDir scriptsDir(gpDesktopHandler->getScriptsPath()+"/HCOM/objFuncTemplates");
    QStringList files = scriptsDir.entryList(QStringList() << "*.hcom");
    int f=0;
    for(; f<files.size(); ++f)
    {
        files[f].prepend(scriptsDir.absolutePath()+"/");
    }
    QDir localScriptsDir(gpDesktopHandler->getExecPath()+"/../Scripts/HCOM/objFuncTemplates");
    files.append(localScriptsDir.entryList(QStringList() << "*.hcom"));
    for(int g=f; g<files.size(); ++g)
    {
        files[g].prepend(localScriptsDir.absolutePath()+"/");
    }
    files.removeDuplicates();

    Q_FOREACH(const QString fileName, files)
    {
        QFile templateFile(fileName);
        templateFile.open(QFile::ReadOnly | QFile::Text);
        QString code = templateFile.readAll();
        templateFile.close();

        //Get description
        if(code.startsWith("#"))
        {
            mObjectiveFunctionDescriptions << code.section("#",1,1).section("\n",0,0);
        }
        else
        {
            mObjectiveFunctionDescriptions << QFileInfo(templateFile).fileName();
        }

        //Count variables
        int varCounter=0;
        for(int i=1; ; ++i)
        {
            if(code.contains("var"+QString::number(i)))
            {
                ++varCounter;
            }
            else
            {
                break;
            }
        }
        mObjectiveFunctionNumberOfVariables << varCounter;

        //Count arguments
        QStringList args;
        for(int i=1; ; ++i)
        {
            if(code.contains("arg"+QString::number(i)))
            {
                args << "arg"+QString::number(i);
            }
            else
            {
                break;
            }
        }
        mObjectiveFunctionDataLists << args;

        mObjectiveFunctionUsesTimeVector << false;
        mObjectiveFunctionCalls << code;
    }

    return true;
}


//! @brief Returns the Python calling code to one of the selected functions
//! @param i Index of selected function
QString OptimizationDialog::generateFunctionCode(int i)
{
    QString retval;
    if(mSelectedFunctionsMinMax.at(i) == "Minimize")
    {
        retval.append("+");
    }
    else
    {
        retval.append("-");
    }

    int fnc = mSelectedFunctions.at(i);

    retval.append("w"+QString().setNum(i)+"*("+mObjectiveFunctionCalls.at(fnc)+"(data"+QString().setNum(i)+"0");
    for(int v=1; v<mObjectiveFunctionNumberOfVariables.at(fnc); ++v)
        retval.append(", data"+QString().setNum(i)+QString().setNum(v));
    if(mObjectiveFunctionUsesTimeVector.at(fnc))
        retval.append(", time");
    for(int d=0; d<mObjectiveFunctionDataLists.at(fnc).size(); ++d)
    {
        double num = mDataLineEditPtrs.at(i).at(d)->text().toDouble();
        retval.append(", "+QString().setNum(num));
    }
    retval.append(")/n"+QString().setNum(i)+")**g"+QString().setNum(i));

    return retval;
}
