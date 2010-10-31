//$Id$

#ifndef PlotWindow_H
#define PlotWindow_H

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

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>

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
    PlotWindow(PlotParameterTree *PlotParameterTree, MainWindow *parent);
    void addPlotCurve(QVector<double> xarray, QVector<double> yarray, QString componentName, QString portName, QString dataName, QString dataUnit, QwtPlot::Axis axisY);
    void changeXVector(QVector<double> xarray, QString componentName, QString portName, QString dataName, QString dataUnit);
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
    QToolButton *mpDiscardGenerationButton;
    QToolBar *mpSizeButton;
    QSpinBox *mpSizeSpinBox;
    QToolButton *mpColorButton;
    QToolButton *mpBackgroundColorButton;
    QCheckBox *mpAutoUpdateCheckBox;
    QLabel *mpSizeLabel;
    QLabel *mpGenerationLabel;

    QRubberBand *mpHoverRect;
    //QPainter *mpHoverRect;

    int nCurves;
    QStringList mCurveColors;
    bool mHasSpecialXAxis;
    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;
    bool mAutoUpdate;

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void closeEvent(QCloseEvent *);

public slots:
    void discardGeneration();

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
    void setAutoUpdate(bool value);
    void stepBack();
    void stepForward();

private:
    PlotParameterTree *mpPlotParameterTree;
    QMap<QString, QString> mCurrentUnitsLeft;
};

#endif // PlotWidget_H
