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
//! @file   SensitivityAnalysisDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the sensitivity analysis dialog
//!
//$Id$

#ifndef SENSITIVITYANALYSISDIALOG_H
#define SENSITIVITYANALYSISDIALOG_H

#include <QtGui>

class ModelWidget;
class SensitivityAnalysisSettings;
class SystemContainer;

class SensitivityAnalysisDialog : public QDialog
{
    Q_OBJECT

public:
    SensitivityAnalysisDialog(QWidget *parent = 0);
    enum DistributionEnumT {UniformDistribution, NormalDistribution};

public slots:
    void open();
    void loadSettings();
    void saveSettings();

private slots:
    void updateChosenParameters(QTreeWidgetItem* item, int i);
    void updateChosenVariables(QTreeWidgetItem* item, int i);
    void run();

private:
    //Parameters
    QTreeWidget *mpParametersList;
    //QLabel *mpParametersLabel;
    //QLabel *mpParameterNameLabel;
    //QLabel *mpParameterAverageLabel;
    //QLabel *mpParameterSigmaLabel;
    QGridLayout *mpParametersLayout;
    //QGroupBox *mpParametersGroupBox;

    //Output
    QTreeWidget *mpOutputList;
    //QLabel *mpOutputLabel;
    //QLabel *mpOutputNameLabel;
    QGridLayout *mpOutputLayout;
    //QGroupBox *mpOutputGroupBox;

    //Member widgets
    QSpinBox *mpStepsSpinBox;
    QRadioButton *mpUniformDistributionRadioButton;
    QRadioButton *mpNormalDistributionRadioButton;
    QProgressBar *mpProgressBar;

    //Member variables
    SensitivityAnalysisSettings *mpSettings;
    QVector<ModelWidget *> mModelPtrs;
    QStringList mSelectedComponents;
    QStringList mSelectedParameters;
    QList<QLabel*> mpParameterLabels;
    QList<QLineEdit*> mpParameterAverageLineEdits;
    QList<QLineEdit*> mpParameterSigmaLineEdits;
    QList<QLineEdit*> mpParameterMinLineEdits;
    QList<QLineEdit*> mpParameterMaxLineEdits;

    QList<QStringList> mOutputVariables;
    QList<QLabel*> mpOutputLabels;
};

#endif // SENSITIVITYANALYSISDIALOG_H
