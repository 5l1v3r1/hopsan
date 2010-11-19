//$Id: OptionsWidget.h 1195 2010-04-01 09:25:58Z robbr48 $

#ifndef OptionsWidget_H
#define OptionsWidget_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QToolButton>
#include <QComboBox>

class MainWindow;

class OptionsWidget : public QDialog
{
    Q_OBJECT

public:
    OptionsWidget(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

public slots:
    void updateValues();
    void colorDialog();
    void show();

private slots:
    void addValueUnit();
    void addPressureUnit();
    void addFlowUnit();
    void addForceUnit();
    void addPositionUnit();
    void addVelocityUnit();
    void addCustomUnitDialog(QString physicalQuantity);
    void addCustomUnit();
    void updateCustomUnits();

private:
    QColor mPickedBackgroundColor;

    QCheckBox *mpInvertWheelCheckBox;
    QCheckBox *mpAntiAliasingCheckBox;
    QCheckBox *mpSnappingCheckBox;
    QLabel *mpBackgroundColorLabel;
    QToolButton *mpBackgroundColorButton;
    QGroupBox *mpInterfaceGroupBox;
    QGridLayout *mpInterfaceLayout;

    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpThreadsLabel;
    QSpinBox *mpThreadsSpinBox;
    QLabel *mpThreadsWarningLabel;
    QLabel *mpProgressBarLabel;
    QSpinBox *mpProgressBarSpinBox;
    QGroupBox *mpSimulationGroupBox;
    QGridLayout *mpSimulationLayout;

    QLabel *mpValueUnitLabel;
    QComboBox *mpValueUnitComboBox;
    QPushButton *mpAddValueUnitButton;
    QLabel *mpPressureUnitLabel;
    QComboBox *mpPressureUnitComboBox;
    QPushButton *mpAddPressureUnitButton;
    QLabel *mpFlowUnitLabel;
    QComboBox *mpFlowUnitComboBox;
    QPushButton *mpAddFlowUnitButton;
    QLabel *mpForceUnitLabel;
    QComboBox *mpForceUnitComboBox;
    QPushButton *mpAddForceUnitButton;
    QLabel *mpPositionUnitLabel;
    QComboBox *mpPositionUnitComboBox;
    QPushButton *mpAddPositionUnitButton;
    QLabel *mpVelocityUnitLabel;
    QComboBox *mpVelocityUnitComboBox;
    QPushButton *mpAddVelocityUnitButton;
    QGroupBox *mpPlottingGroupBox;
    QGridLayout *mpPlottingLayout;

    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;
    QDialogButtonBox *mpButtonBox;

    QWidget *mpCentralwidget;

    QDialog *mpAddUnitDialog;
    QLabel *mpNameLabel;
    QLineEdit *mpUnitNameBox;
    QLabel *mpScaleLabel;
    QLineEdit *mpScaleBox;
    QPushButton *mpDoneInUnitDialogButton;
    QPushButton *mpCancelInUnitDialogButton;
    QString mPhysicalQuantityToModify;
};

#endif // OptionsWidget_H
