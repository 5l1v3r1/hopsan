/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Link�ping University,
 * Department of Computer and Information Science,
 * SE-58183 Link�ping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL).
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Link�ping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Link�ping University
 * Main Authors 2009-2010:  Robert Braun, Bj�rn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$

#ifndef PlotWindow_H
#define PlotWindow_H


#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_data.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_item.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_picker.h>
#include <QGridLayout>
#include <iostream>
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVector>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHash>
#include <QToolBar>
#include <QToolButton>
#include <QMainWindow>
#include <QColor>
#include <QMouseEvent>
#include <QApplication>
#include <QDragMoveEvent>
#include <qwt_legend.h>
#include <QFileDialog>
#include <QSvgGenerator>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>


class MainWindow;
class VariablePlot;
class VariablePlotZoomer;
class PlotParameterTree;
class PlotWidget;
class GUISystem;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
public:
    PlotWindow(QVector<double> xarray, QVector<double> yarray, PlotParameterTree *PlotParameterTree, MainWindow *parent);
    void addPlotCurve(QVector<double> xarray, QVector<double> yarray, QString title, QString xLabel, QString yLabel, QwtPlot::Axis axisY);
    void changeXVector(QVector<double> xarray, QString xLabel, QString componentName, QString portName, QString dataName);
    void insertMarker(QwtPlotCurve *curve);
    void setActiveMarker(QwtPlotMarker *marker);
    void setGeneration(int gen);

    MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentGUISystem;

    QVector<QwtPlotCurve *> mpCurves;
    QList<QStringList> mCurveParameters;
    QStringList mSpecialXParameter;
    QList< QList< QVector<double> > > mVectorX;
    QList< QList< QVector<double> > > mVectorY;

    QwtPlot *mpVariablePlot;
    QwtPlotGrid *mpGrid;
    QwtSymbol *mpMarkerSymbol;
    QwtPlotMarker *mpActiveMarker;
    QVector <QwtPlotMarker *> mpMarkers;
    QHash <QwtPlotCurve *, QwtPlotMarker *> mCurveToMarkerMap;
    QHash <QwtPlotMarker *, QwtPlotCurve *> mMarkerToCurveMap;
    QwtPlotCurve *tempCurve;
    QwtPlotZoomer *mpZoomer;
    QwtPlotMagnifier *mpMagnifier;
    QwtPlotPanner *mpPanner;
    int mCurrentGeneration;

    QToolBar *mpToolBar;
    QToolButton *mpZoomButton;
    QToolButton *mpPanButton;
    QToolButton *mpSVGButton;
    QToolButton *mpExportGNUPLOTButton;
    QToolButton *mpImportGNUPLOTButton;
    QToolButton *mpGridButton;
    QToolButton *mpPreviousButton;
    QToolButton *mpNextButton;
    QToolBar *mpSizeButton;
    QSpinBox *mpSizeSpinBox;
    QToolButton *mpColorButton;
    QToolButton *mpBackgroundColorButton;
    QCheckBox *mpHoldCheckBox;
    QLabel *mpSizeLabel;
    QLabel *mpGenerationLabel;

    QRubberBand *mpHoverRect;
    //QPainter *mpHoverRect;

    int nCurves;
    QStringList mCurveColors;
    bool mHasSpecialXAxis;
    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;
    bool mHold;

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void closeEvent(QCloseEvent *);

private slots:
    void enableZoom(bool);
    void enablePan(bool);
    void exportSVG();
    void exportGNUPLOT();
    void importGNUPLOT();
    void enableGrid(bool);
    void setLineWidth(int);
    void setLineColor();
    void setBackgroundColor();
    void checkNewValues();
    void setHold(bool value);
    void stepBack();
    void stepForward();

private:
    PlotParameterTree *mpPlotParameterTree;
};

#endif // PlotWidget_H
