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
//! @file   Configuration.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for the configuration object
//!
//$Id$

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QList>
#include <QFileInfo>
#include <QPen>
#include <QPalette>
#include <QFont>
#include <QDomElement>

#include "common.h"
#include "UnitScale.h"

class Configuration : public QObject
{
    Q_OBJECT

public:
    void saveToXml();
    void loadFromXml();
    void loadDefaultsFromXml();

    bool getShowPopupHelp();
    bool getInvertWheel();
    bool getToggleNamesButtonCheckedLastSession();
    bool getTogglePortsButtonCheckedLastSession();
    int getProgressBarStep();
    bool getEnableProgressBar();
    bool getSnapping();

    bool getUseMulticore();
    int getNumberOfThreads();

    int getLibraryStyle();
    bool getUseNativeStyleSheet();
    QColor getBackgroundColor();
    QPalette getPalette();
    QFont getFont();
    QString getStyleSheet();
    QPen getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, QString situation);
    bool getAntiAliasing();

    QStringList getUserLibs();
    QList<LibraryTypeEnumT> getUserLibTypes();

    QStringList getRecentModels();
    QStringList getRecentGeneratorModels();
    QStringList getLastSessionModels();

    QString getLastPyScriptFile();

    QStringList getUnitQuantities() const;
    QString getDefaultUnit(const QString &rPhysicalQuantity) const;
    QMap<QString, double> getCustomUnits(const QString &rPhysicalQuantity);
    bool hasUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    double getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    QStringList getPhysicalQuantitiesForUnit(const QString &rUnit) const;
    QString getSIUnit(const QString &rQuantity);
    void removeUnitScale(const QString &rQuantity, const QString &rUnit);

    int getPLOExportVersion() const;
    bool getShowHiddenNodeDataVariables() const;

    bool getGroupMessagesByTag();
    QStringList getTerminalHistory();
    QString getHcomWorkingDirectory() const;
    int getGenerationLimit() const;
    bool getCacheLogData() const;

    bool getAutoLimitLogDataGenerations();

    QString getLoadModelDir();
    QString getModelGfxDir();
    QString getPlotDataDir();
    QString getPlotGfxDir();
    QString getSimulinkExportDir();
    QString getSubsystemDir();
    QString getModelicaModelsDir();
    QString getExternalLibDir();
    QString getScriptDir();
    QString getPlotWindowDir();
    QString getFmuImportDir();
    QString getFmuExportDir();
    QString getLabViewExportDir();

    int getParallelAlgorithm();

    void setLibraryStyle(int value);
    void setShowPopupHelp(bool value);
    void setUseNativeStyleSheet(bool value);
    void setInvertWheel(bool value);
    void setUseMultiCore(bool value);
    void setNumberOfThreads(size_t value);
    void setProgressBarStep(int value);
    void setEnableProgressBar(bool value);
    void setBackgroundColor(QColor value);
    void setAntiAliasing(bool value);
    void addUserLib(QString value, LibraryTypeEnumT type);
    void removeUserLib(QString value);
    bool hasUserLib(QString value) const;
    void setSnapping(bool value);
    void addRecentModel(QString value);
    void removeRecentModel(QString value);
    void addRecentGeneratorModel(QString value);
    void addLastSessionModel(QString value);
    void clearLastSessionModels();
    void setDefaultUnit(QString key, QString value);
    void addCustomUnit(QString quantity, QString unitname, double scale);
    void setLastPyScriptFile(QString file);
    void setGroupMessagesByTag(bool value);
    void setGenerationLimit(int value);
    void setCacheLogData(const bool value);
    void setAutoLimitLogDataGenerations(const bool value);
    void setShowHiddenNodeDataVariables(const bool value);
    void setLoadModelDir(QString value);
    void setModelGfxDir(QString value);
    void setPlotDataDir(QString value);
    void setPlotGfxDir(QString value);
    void setSimulinkExportDir(QString value);
    void setSubsystemDir(QString value);
    void setModelicaModelsDir(QString value);
    void setExternalLibDir(QString value);
    void setScriptDir(QString value);
    void setPlotWindowDir(QString value);
    void storeTerminalHistory(QStringList value);
    void setHcomWorkingDirectory(QString value);
    void setFmuImportDir(QString value);
    void setFmuExportDir(QString value);
    void setLabViewExportDir(QString value);

    void setParallelAlgorithm(int value);

private:
    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);
    void loadUnitSettings(QDomElement &rDomElement);
    void loadUnitScales(QDomElement &rDomElement);
    void loadLibrarySettings(QDomElement &rDomElement);
    void loadModelSettings(QDomElement &rDomElement);
    void loadScriptSettings(QDomElement &rPythonElement, QDomElement &rHcomElement);

    class QuantityUnitScale
    {
    public:
        QString siunit;
        QString selectedDefaultUnit;
        QMap<QString, UnitScale> customScales;
    };

    int mLibraryStyle;
    bool mShowPopupHelp;
    bool mUseNativeStyleSheet;
    bool mInvertWheel;
    bool mUseMulticore;
    int mNumberOfThreads;
    bool mToggleNamesButtonCheckedLastSession;
    bool mTogglePortsButtonCheckedLastSession;
    int mProgressBarStep;
    bool mEnableProgressBar;
    QColor mBackgroundColor;
    bool mAntiAliasing;
    QList<QFileInfo> mUserLibs;
    QList<LibraryTypeEnumT> mUserLibTypes;
    bool mSnapping;
    QStringList mRecentModels;
    QStringList mLastSessionModels;
    QStringList mRecentGeneratorModels;
    QMap<QString, QString> mSelectedDefaultUnits;
    QMap<QString, QuantityUnitScale> mUnitScales;
    QPalette mPalette;
    QFont mFont;
    QString mStyleSheet;
    QString mLastPyScriptFile;
    bool mGroupMessagesByTag;
    int mGenerationLimit;
    bool mCacheLogData;
    bool mAutoLimitLogDataGenerations;
    QString mLoadModelDir;
    QString mModelGfxDir;
    QString mPlotDataDir;
    QString mPlotGfxDir;
    QString mSimulinkExportDir;
    QString mSubsystemDir;
    QString mModelicaModelsDir;
    QString mExternalLibDir;
    QString mScriptDir;
    QString mPlotWindowDir;
    QStringList mTerminalHistory;
    QString mHcomWorkingDirectory;
    QString mFmuImportDir;
    QString mFmuExportDir;
    QString mLabViewExportDir;
    int mPLOExportVersion;
    bool mShowHiddenNodeDataVariables;

    int mParallelAlgorighm;

    QMap < ConnectorStyleEnumT, QMap< QString, QMap<QString, QPen> > > mPenStyles;

signals:
    void recentModelsListChanged();
};

#endif // CONFIGURATION_H
