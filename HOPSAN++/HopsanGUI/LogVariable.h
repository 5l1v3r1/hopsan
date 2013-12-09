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
//! @file   LogDataHandler.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2012-12-18
//!
//! @brief Contains the LogData classes
//!
//$Id$

#ifndef LOGVARIABLE_H
#define LOGVARIABLE_H

#include <QSharedPointer>
#include <QVector>
#include <QString>
#include <QColor>
#include <QObject>
#include <QPointer>
#include <QTextStream>

#include "CachableDataVector.h"
#include "common.h"
#include "UnitScale.h"

#define TIMEVARIABLENAME "Time"

// Forward declaration
class LogVariableData;
class LogDataHandler;

QString makeConcatName(const QString componentName, const QString portName, const QString dataName);
void splitConcatName(const QString fullName, QString &rCompName, QString &rPortName, QString &rVarName);

//! @brief This enum describes where a variable come from, the order signifies importance (ModelVariables most important)
enum VariableSourceTypeT {ModelVariableType, ImportedVariableType, ScriptVariableType, TempVariableType, UndefinedVariableType};
//! @brief This function converts a VariableSourceTypeT enum into a string
QString getVariableSourceTypeString(const VariableSourceTypeT type);

//! @class VariableCommonDescription
//! @brief Container class for strings describing a log variable (common data for all generations)
class VariableCommonDescription
{
public:
    VariableCommonDescription() : mVariableSourceType(UndefinedVariableType) {}
    QString mModelPath;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QString mDataDescription;
    QString mAliasName;
    VariableSourceTypeT mVariableSourceType;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);

    bool operator==(const VariableCommonDescription &other) const;
};

//! @class VariableUniqueDescription
//! @brief Container class for strings describing a log variable (unique for each data)
class VariableUniqueDescription
{
public:
    VariableSourceTypeT mVariableSourceType;
    QString mImportFileName;
};

typedef QSharedPointer<VariableCommonDescription> SharedVariableCommonDescriptionT;
typedef QSharedPointer<VariableUniqueDescription> SharedVariableUniqueDescriptionT;
typedef QSharedPointer<LogVariableData> SharedLogVariableDataPtrT;

SharedLogVariableDataPtrT createFreeTimeVariabel(const QVector<double> &rTime);

class LogVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, SharedLogVariableDataPtrT> GenerationMapT;

    //! @todo also need qucik constructor for creating a container with one generation directly
    LogVariableContainer(const VariableCommonDescription &rVarDesc, LogDataHandler *pParentLogDataHandler);
    ~LogVariableContainer();
    SharedLogVariableDataPtrT addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData);
    SharedLogVariableDataPtrT addDataGeneration(const int generation, const SharedLogVariableDataPtrT time, const QVector<double> &rData);
    void addDataGeneration(const int generation, SharedLogVariableDataPtrT pData);
    bool removeDataGeneration(const int generation, const bool force=false);
    bool purgeOldGenerations(const int purgeEnd, const int nGensToKeep);
    void removeAllGenerations();
    void removeAllImportedGenerations();

    SharedLogVariableDataPtrT getDataGeneration(const int gen=-1) const;
    QList<SharedLogVariableDataPtrT> getAllDataGenerations() const;
    bool hasDataGeneration(const int gen);
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    QList<int> getGenerations() const;

    void setVariableCommonDescription(const VariableCommonDescription &rNewDescription);
    SharedVariableCommonDescriptionT getVariableCommonDescription() const;
    const QString &getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getSmartName() const;
    const QString &getModelPath() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataUnit() const;

    void preventAutoRemove(const int gen);
    void allowAutoRemove(const int gen);

    LogDataHandler *getLogDataHandler();

    void setAliasName(const QString alias);

signals:
    void nameChanged();
    void logVariableBeingRemoved(SharedLogVariableDataPtrT);

private:
    LogDataHandler *mpParentLogDataHandler;
    SharedVariableCommonDescriptionT mVariableCommonDescription;
    GenerationMapT mDataGenerations;
    QList<int> mKeepGenerations;
};


class LogVariableData : public QObject
{
    Q_OBJECT
    friend class LogVariableContainer;
    friend class LogDataHandler;

public:
    LogVariableData(const int generation, SharedLogVariableDataPtrT time, const QVector<double> &rData, SharedVariableCommonDescriptionT varDesc,
                    SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
    ~LogVariableData();

    const SharedVariableCommonDescriptionT getVariableCommonDescription() const;
    const SharedVariableUniqueDescriptionT getVariableUniqueDescription() const;
    void setVariableUniqueDescription(const VariableUniqueDescription &rDesc);
    const SharedLogVariableDataPtrT getTimeVector() const;
    VariableSourceTypeT getVariableSource() const;
    const QString &getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getSmartName() const;
    const QString &getModelPath() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataUnit() const;
    bool hasAliasName() const;
    int getGeneration() const;
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    bool isImported() const;
    QString getImportedFromFileName() const;

    void setCustomUnitScale(const UnitScale &rUnitScale);
    const UnitScale &getCustomUnitScale() const;
    void removeCustomUnitScale();
    const QString &getPlotScaleDataUnit() const;
    const QString &getActualPlotDataUnit() const;
    double getPlotScale() const;
    double getPlotOffset() const;

    const SharedLogVariableDataPtrT getSharedTimePointer() const;
    QVector<double> getDataVectorCopy();
    void sendDataToStream(QTextStream &rStream, QString separator);
    int getDataSize() const;
    double first() const;
    double last() const;

    void addToData(const SharedLogVariableDataPtrT pOther);
    void addToData(const double other);
    void subFromData(const SharedLogVariableDataPtrT pOther);
    void subFromData(const double other);
    void multData(const SharedLogVariableDataPtrT pOther);
    void multData(const double other);
    void divData(const SharedLogVariableDataPtrT pOther);
    void divData(const double other);
    void absData();
    void diffBy(const SharedLogVariableDataPtrT pOther);
    void lowPassFilter(const SharedLogVariableDataPtrT pTime, const double w);
    void frequencySpectrum(const SharedLogVariableDataPtrT pTime, const bool doPowerSpectrum);
    void assignFrom(const SharedLogVariableDataPtrT pOther);
    void assignFrom(const QVector<double> &rSrc);
    void assignFrom(const double src);
    void assignFrom(SharedLogVariableDataPtrT time, const QVector<double> &rData);
    void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    double pokeData(const int index, const double value, QString &rErr);
    double peekData(const int index, QString &rErr) const;
    bool indexInRange(const int idx) const;
    double averageOfData() const;
    double minOfData(int &rIdx) const;
    double minOfData() const;
    double maxOfData(int &rIdx) const;
    double maxOfData() const;
    void elementWiseGt(QVector<double> &rResult, const double threshold) const;
    void elementWiseLt(QVector<double> &rResult, const double threshold) const;
    void append(const double t, const double y);
    void append(const double y);

    void preventAutoRemoval();
    void allowAutoRemoval();

    void setCacheDataToDisk(const bool toDisk);
    bool isCachingDataToDisk() const;

    LogVariableContainer *getLogVariableContainer();
    LogDataHandler *getLogDataHandler();

public slots:
    void setTimePlotScaleAndOffset(const double scale, const double offset);
    void setTimePlotScale(double scale);
    void setTimePlotOffset(double offset);
    void setPlotScale(double scale);
    void setPlotOffset(double offset);
    void setPlotScaleAndOffset(const double scale, const double offset);


signals:
    void dataChanged();
    void nameChanged();

protected:
    double peekData(const int idx) const;

private:
    typedef QVector<double> DataVectorT;
    SharedLogVariableDataPtrT mSharedTimeVectorPtr;
    CachableDataVector *mpCachedDataVector;
    QPointer<LogVariableContainer> mpParentVariableContainer;
    SharedVariableCommonDescriptionT mpVariableCommonDescription;
    SharedVariableUniqueDescriptionT mpVariableUniqueDescription;

    UnitScale mCustomUnitScale;
    double mDataPlotScale;
    double mDataPlotOffset;
    int mGeneration;
};

#endif // LOGVARIABLE_H
