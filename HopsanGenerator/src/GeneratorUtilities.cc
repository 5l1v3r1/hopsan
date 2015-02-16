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
//! @file   ComponentGeneratorUtilities.cc
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utilities
//!
//$Id$

#include <QStringList>
#include <QProcess>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QApplication>
#include <QPushButton>
#include <QProgressDialog>
#include <QDomElement>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDirIterator>

#include <cassert>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "GeneratorUtilities.h"
#include "generators/HopsanGenerator.h"
#include "SymHop.h"

#include "HopsanEssentials.h"
#include "ComponentSystem.h"
#include "Port.h"
#include "version.h"


using namespace std;
using namespace SymHop;
using namespace hopsan;


FMIPortSpecification::FMIPortSpecification(QString varName, QString portName, QString mpndName, QString valueRef, QString portType, QString nodeType, QString dataType, QString causality)
{
    this->varName = varName;
    this->portName = portName;
    this->mpndName = mpndName;
    this->valueRef = valueRef;
    this->portType = portType;
    this->nodeType = nodeType;
    this->dataType = dataType;
    this->causality = causality;
}


FMIParameterSpecification::FMIParameterSpecification(QString varName, QString parName, QString description, QString initValue, QString valueRef)
{
    this->varName = varName;
    this->parName = parName;
    this->description = description;
    this->initValue = initValue;
    this->valueRef = valueRef;
}


PortSpecification::PortSpecification(QString porttype, QString nodetype, QString name, bool notrequired, QString defaultvalue)
{
    this->porttype = porttype;
    this->nodetype = nodetype;
    this->name = name;
    this->notrequired = notrequired;
    this->defaultvalue = defaultvalue;
}


ParameterSpecification::ParameterSpecification(QString name, QString displayName, QString description, QString unit, QString init)
{
    this->name = name;
    this->displayName = displayName;
    this->description = description;
    this->unit = unit;
    this->init = init;
}


VariableSpecification::VariableSpecification(QString name, QString init)
{
    this->name = name;
    this->init = init;
}


UtilitySpecification::UtilitySpecification(QString utility, QString name)
{
    this->utility = utility;
    this->name = name;
}


StaticVariableSpecification::StaticVariableSpecification(QString datatype, QString name)
{
    this->datatype = datatype;
    this->name = name;
}



ComponentSpecification::ComponentSpecification(QString typeName, QString displayName, QString cqsType)
{
    this->typeName = typeName;
    this->displayName = displayName;
    if(cqsType == "S")
        cqsType = "Signal";
    this->cqsType = cqsType;

    this->auxiliaryFunctions = QStringList();
}






//! @brief Function for loading an XML DOM Document from file
//! @param[in] rFile The file to load from
//! @param[in] rDomDocument The DOM Document to load into
//! @param[in] rootTagName The expected root tag name to extract from the Dom Document
//! @returns The extracted DOM root element from the loaded DOM document
QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName)
{
    QString errorStr;
    int errorLine, errorColumn;
    if (!rDomDocument.setContent(&rFile, false, &errorStr, &errorLine, &errorColumn))
    {
        //! @todo Error message somehow
    }
    else
    {
        QDomElement xmlRoot = rDomDocument.documentElement();
        if (xmlRoot.tagName() != rootTagName)
        {
            //! @todo Error message somehow
        }
        else
        {
            return xmlRoot;
        }
    }
    return QDomElement(); //NULL
}



bool removeDir(QString path)
{
    QDir dir;
    dir.setPath(path);
    Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            removeDir(info.absoluteFilePath());
        }
        else
        {
            if(QFile::remove(info.absoluteFilePath()))
            {
                qDebug() << "Successfully removed " << info.absoluteFilePath();
            }
            else
            {
                qDebug() << "Failed to remove " << info.absoluteFilePath();
            }
        }
    }
    return dir.rmdir(path);     //If removing files fails, this will fail, so we only need to check this
}


//! @brief Copy a directory with contents
//! @param [in] fromPath The absolute path to the directory to copy
//! @param [in] toPath The absolute path to the destination (including resulting dir name)
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
void copyDir(const QString fromPath, QString toPath)
{
    QDir toDir(toPath);
    toDir.mkpath(toPath);
    if (toPath.endsWith('/'))
    {
        toPath.chop(1);
    }

    QDir fromDir(fromPath);
    foreach(QFileInfo info, fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            copyDir(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
        else
        {
            if(QFile::exists(toPath+"/"+info.fileName()))
            {
                QFile::remove(toPath+"/"+info.fileName());
            }
            QFile::copy(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
    }
}



// Operators
QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}



bool compileComponentLibrary(QString path, HopsanGenerator *pGenerator, QString extraLinks, QString extraIncludes)
{
    pGenerator->printMessage("Writing compilation script...");

    QStringList ccFiles;
    QString name;
    if(QFileInfo(path).isFile())
    {
        QFile xmlFile(path);
        xmlFile.open(QFile::ReadOnly);
        QString xmlCode = xmlFile.readAll();
        xmlFile.close();
        QStringList xmlLines = xmlCode.split("\n");

        QFileInfo xmlFileInfo(xmlFile);
        path = xmlFileInfo.path();

        for(int l=0; l<xmlLines.size(); ++l)
        {
            if(xmlLines[l].trimmed().startsWith("<source>"))
            {
                ccFiles.append(xmlLines[l].section("<source>",1,1).section("</source>",0,0));
            }
            else if(xmlLines[l].trimmed().startsWith("<lib>"))
            {
                name = QString(LIBPREFIX)+xmlLines[l].section("<lib>",1,1).section("</lib>",0,0).section(".",0,0);
            }
        }
    }
    else
    {
        name = QString(LIBPREFIX)+QDir(path).dirName();
        ccFiles = QDir(path).entryList(QStringList() << "*.cc");
    }

    QString c;
    Q_FOREACH(const QString &file, ccFiles)
        c.append(file+" ");
    c.chop(1);

    QString i = pGenerator->getCoreIncludePath();
    i.prepend("-I\"");
    i.append("\"");
    i.append(" "+extraIncludes);

    QString binDir = pGenerator->getBinPath();
    QString l = "-L\""+binDir+"\" -lHopsanCore "+extraLinks;

    pGenerator->printMessage("\nCalling compiler utility:");
    pGenerator->printMessage("Path: "+path);
    pGenerator->printMessage("Objective: "+name);
    qDebug() << "Objective: " << name;
    pGenerator->printMessage("Source files: "+c);
    pGenerator->printMessage("Includes: "+i);
    pGenerator->printMessage("Links: "+l+"\n");

    QString output;
    QString gccPath = pGenerator->getGccPath();
    bool success = compile(path, gccPath, name, c, i, l, "-Dhopsan=hopsan -fPIC -w -Wl,--rpath -Wl,\""+path+"\" -shared ", output);
    pGenerator->printMessage(output);
    return success;
}


//! @brief Calls GCC or MinGW compiler with specified parameters
//! @param path Absolute path where compiler shall be run
//! @param o Objective file name (without file extension)
//! @param c List with source files, example: "file1.cpp file2.cc"
//! @param i Include command, example: "-Ipath1 -Ipath2"
//! @param l Link command, example: "-Lpath1 -lfile1 -lfile2"
//! @param flags Compiler flags
//! @param output Reference to string where output messages are stored
bool compile(QString path, QString gccPath, QString o, QString c, QString i, QString l, QString flags, QString &output)
{
    //Create compilation script file
#ifdef _WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(path + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output = "Could not open compile.bat for writing.";
        return false;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "call "+gccPath+"/g++.exe "+flags;
    clBatchStream << c+" -o "+o+".dll "+i+" "+l+"\n";
    clBatchFile.close();
#endif

    //Call compilation script file
#ifdef _WIN32
    QProcess gccProcess;
    gccProcess.setWorkingDirectory(path);
    gccProcess.start("cmd.exe", QStringList() << "/c" << "compile.bat");
    gccProcess.waitForFinished();
    QByteArray gccResult = gccProcess.readAllStandardOutput();
    QByteArray gccError = gccProcess.readAllStandardError();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        output = gccResultList.at(i);
        //output = output.remove(output.size()-1, 1);
    }
    QList<QByteArray> gccErrorList = gccError.split('\n');
    for(int i=0; i<gccErrorList.size(); ++i)
    {
        if(gccErrorList.at(i).trimmed() == "")
        {
            gccErrorList.removeAt(i);
            --i;
            continue;
        }
        output = output+ gccErrorList.at(i);
        //output = output.remove(output.size()-1, 1);
    }
#elif linux
    QString gccCommand = "cd \""+path+"\" && gcc "+flags+" ";
    gccCommand.append(c+" -fpermissive -o "+o+".so "+i+" "+l);
    //qDebug() << "Command = " << gccCommand;
    gccCommand +=" 2>&1";
    FILE *fp;
    char line[130];
    output.append("Compiler command: \""+gccCommand+"\"\n");
    qDebug() << "Compiler command: \"" << gccCommand << "\"";
    fp = popen(  (const char *) gccCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        output.append("Could not execute '" + gccCommand + "'! err=%d\n");
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            output.append(QString::fromUtf8(line));
        }
    }
#endif

    QDir targetDir(path);
#ifdef _WIN32

    if(!targetDir.exists(o + ".dll") || !gccErrorList.isEmpty())
    {
        output.append("Compilation failed.");
        return false;
    }
#elif linux
    if(!targetDir.exists(o + ".so"))
    {
        qDebug() << targetDir.absolutePath();
        qDebug() << o + ".so";
        output.append("Compilation failed.");
        return false;
    }
#endif

    output.append("Compilation successful.");
    return true;
}



//! @brief Removes all illegal characters from the string, so that it can be used as a variable name.
//! @param org Original string
//! @returns String without illegal characters
QString toVarName(const QString org)
{
    QString ret = org;
    while(!ret.isEmpty() && !ret[0].isLetter())
    {
        ret = ret.right(ret.size()-1);
    }
    for(int i=1; i<ret.size(); ++i)
    {
        if(!ret[i].isLetterOrNumber())
        {
            ret.remove(i,1);
            i--;
        }
    }
    return ret;
}


QString extractTaggedSection(QString str, QString tag)
{
    QString startStr = ">>>"+tag+">>>";
    QString endStr = "<<<"+tag+"<<<";
    if(!str.contains(startStr) || !str.contains(endStr))
    {
        return QString();
    }
    else
    {
        int i = str.indexOf(startStr)+startStr.size();
        int n = str.indexOf(endStr)-i;
        return str.mid(i, n);
    }
}


void replaceTaggedSection(QString &str, QString tag, QString replacement)
{
    QString taggedSection = ">>>"+tag+">>>"+extractTaggedSection(str, tag)+"<<<"+tag+"<<<";
    str.replace(taggedSection, replacement);
}


QString replaceTag(QString str, QString tag, QString replacement)
{
    QString retval = str;
    retval.replace("<<<"+tag+">>>", replacement);
    return retval;
}


QString replaceTags(QString str, QStringList tags, QStringList replacements)
{
    QString retval = str;
    for(int i=0; i<tags.size(); ++i)
    {
        retval.replace("<<<"+tags[i]+">>>", replacements[i]);
    }
    return retval;
}


//! @brief Verifies that a system of equations is solvable (number of equations = number of unknowns etc)
bool verifyEquationSystem(QList<Expression> equations, QList<Expression> stateVars, HopsanGenerator *pGenerator)
{
    bool retval = true;

    if(equations.size() != stateVars.size())
    {
        QStringList equationList;
        for(int s=0; s<equations.size(); ++s)
        {
            equationList.append(equations[s].toString());
        }
        qDebug() << "Equations: " << equationList;

        QStringList stateVarList;
        for(int s=0; s<stateVars.size(); ++s)
        {
            stateVarList.append(stateVars[s].toString());
        }
        qDebug() << "State vars: " << stateVarList;

        pGenerator->printErrorMessage("Number of equations = " + QString::number(equations.size()) + ", number of state variables = " + QString::number(stateVars.size()));
        retval = false;
    }

    return retval;
}




void findAllFilesInFolderAndSubFolders(QString path, QString ext, QStringList &files)
{
    QDir dir(path);
    QDirIterator iterator(dir.absolutePath(), QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        iterator.next();
        if (!iterator.fileInfo().isDir())
        {
            QString fileName = iterator.filePath();
            if (fileName.endsWith("."+ext))
                files.append(fileName);
        }
    }
}



QStringList getHopsanCoreSourceFiles()
{
    QStringList srcFiles;
    srcFiles << "HopsanCore/src/Component.cc" <<
                "HopsanCore/src/ComponentSystem.cc" <<
                "HopsanCore/src/HopsanEssentials.cc" <<
                "HopsanCore/src/HopsanTypes.cc" <<
                "HopsanCore/src/Node.cc" <<
                "HopsanCore/src/Nodes.cc" <<
                "HopsanCore/src/Parameters.cc" <<
                "HopsanCore/src/Port.cc" <<
                "HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc" <<
                "HopsanCore/src/ComponentUtilities/CSVParser.cc" <<
                "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc" <<
                "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc" <<
                "HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp" <<
                "HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc" <<
                "HopsanCore/src/ComponentUtilities/HopsanPowerUser.cc" <<
                "HopsanCore/src/ComponentUtilities/Integrator.cc" <<
                "HopsanCore/src/ComponentUtilities/IntegratorLimited.cc" <<
                "HopsanCore/src/ComponentUtilities/ludcmp.cc" <<
                "HopsanCore/src/ComponentUtilities/matrix.cc" <<
                "HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc" <<
                "HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc" <<
                "HopsanCore/src/ComponentUtilities/PLOParser.cc" <<
                "HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp" <<
                "HopsanCore/src/CoreUtilities/GeneratorHandler.cpp" <<
                "HopsanCore/src/CoreUtilities/HmfLoader.cc" <<
                "HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc" <<
                "HopsanCore/src/CoreUtilities/LoadExternal.cc" <<
                "HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp" <<
                "HopsanCore/src/CoreUtilities/StringUtilities.cpp" <<
                "componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc" <<
                "Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp" <<
                "Dependencies/IndexingCSVParser/IndexingCSVParser.cpp";

    return srcFiles;
}


QStringList getHopsanCoreIncludeFiles(bool skipDependencies)
{
    QStringList includeFiles;
    includeFiles << "HopsanCore/include/Component.h" <<
                    "HopsanCore/include/ComponentEssentials.h" <<
                    "HopsanCore/include/ComponentSystem.h" <<
                    "HopsanCore/include/ComponentUtilities.h" <<
                    "HopsanCore/include/HopsanCore.h" <<
                    "HopsanCore/include/HopsanCoreMacros.h" <<
                    "HopsanCore/include/HopsanEssentials.h" <<
                    "HopsanCore/include/HopsanTypes.h" <<
                    "HopsanCore/include/Node.h" <<
                    "HopsanCore/include/Nodes.h" <<
                    "HopsanCore/include/Parameters.h" <<
                    "HopsanCore/include/Port.h" <<
                    "HopsanCore/include/svnrevnum.h" <<
                    "HopsanCore/include/version.h" <<
                    "HopsanCore/include/win32dll.h" <<
                    "HopsanCore/include/Components/DummyComponent.hpp" <<
                    "HopsanCore/include/Components/ModelicaComponent.hpp" <<
                    "HopsanCore/include/ComponentUtilities/AuxiliaryMathematicaWrapperFunctions.h" <<
                    "HopsanCore/include/ComponentUtilities/AuxiliarySimulationFunctions.h" <<
                    "HopsanCore/include/ComponentUtilities/CSVParser.h" <<
                    "HopsanCore/include/ComponentUtilities/Delay.hpp" <<
                    "HopsanCore/include/ComponentUtilities/DoubleIntegratorWithDamping.h" <<
                    "HopsanCore/include/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h" <<
                    "HopsanCore/include/ComponentUtilities/EquationSystemSolver.h" <<
                    "HopsanCore/include/ComponentUtilities/FirstOrderTransferFunction.h" <<
                    "HopsanCore/include/ComponentUtilities/HopsanPowerUser.h" <<
                    "HopsanCore/include/ComponentUtilities/Integrator.h" <<
                    "HopsanCore/include/ComponentUtilities/IntegratorLimited.h" <<
                    "HopsanCore/include/ComponentUtilities/ludcmp.h" <<
                    "HopsanCore/include/ComponentUtilities/LookupTable.h" <<
                    "HopsanCore/include/ComponentUtilities/matrix.h" <<
                    "HopsanCore/include/ComponentUtilities/num2string.hpp" <<
                    "HopsanCore/include/ComponentUtilities/SecondOrderTransferFunction.h" <<
                    "HopsanCore/include/ComponentUtilities/TurbulentFlowFunction.h" <<
                    "HopsanCore/include/ComponentUtilities/ValveHysteresis.h" <<
                    "HopsanCore/include/ComponentUtilities/WhiteGaussianNoise.h" <<
                    "HopsanCore/include/ComponentUtilities/PLOParser.h" <<
                    "HopsanCore/include/CoreUtilities/ClassFactory.hpp" <<
                    "HopsanCore/include/CoreUtilities/ClassFactoryStatusCheck.hpp" <<
                    "HopsanCore/include/CoreUtilities/CoSimulationUtilities.h" <<
                    "HopsanCore/include/CoreUtilities/GeneratorHandler.h" <<
                    "HopsanCore/include/CoreUtilities/HmfLoader.h" <<
                    "HopsanCore/include/CoreUtilities/HopsanCoreMessageHandler.h" <<
                    "HopsanCore/include/CoreUtilities/LoadExternal.h" <<
                    "HopsanCore/include/CoreUtilities/MultiThreadingUtilities.h" <<
                    "HopsanCore/include/CoreUtilities/StringUtilities.h" <<
                    "HopsanCore/include/HopsanTypes.h" <<
                    "HopsanCore/include/ComponentUtilities/HopsanPowerUser.h" <<
                    "HopsanCore/include/HopsanCoreMacros.h" <<
                    //"componentLibraries/defaultLibrary/code/defaultComponents.h" <<
                    //"componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.h" <<
                    "HopsanCore/include/compiler_info.h";

    if (!skipDependencies)
    {
        includeFiles << "Dependencies/IndexingCSVParser/IndexingCSVParser.h" <<
                        "Dependencies/IndexingCSVParser/IndexingCSVParserImpl.hpp" <<
                        "Dependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp" <<
                        "Dependencies/rapidxml-1.13/hopsan_rapidxml.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml_iterators.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml_print.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml_utils.hpp";
    }

    return includeFiles;
}

QStringList getHopsanCoreIncludePaths(bool skipDependencies)
{
    QStringList includePaths;
    includePaths << "HopsanCore/include" <<
                    "componentLibraries/defaultLibrary";
    if (!skipDependencies)
    {
        includePaths << "Dependencies/libcsv_parser++-1.0.0/include/csv_parser/" <<
                        "Dependencies/rapidxml-1.13/" <<
                        "Dependencies/IndexingCSVParser/";
    }

    return includePaths;
}



GeneratorNodeInfo::GeneratorNodeInfo(QString nodeType)
{
    hopsan::HopsanEssentials hopsanCore;
    Node *pNode = hopsanCore.createNode(nodeType.toStdString().c_str());

    niceName = pNode->getNiceName().c_str();
    for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
    {
        NodeDataVariableTypeEnumT varType = pNode->getDataDescription(i)->varType;
        if(varType == DefaultType || varType == FlowType || varType == IntensityType)        //Q variable
        {
            qVariables << pNode->getDataDescription(i)->shortname.c_str();
            variableLabels << QString(pNode->getDataDescription(i)->name.c_str());
            varIdx << pNode->getDataDescription(i)->id;
        }
    }
    for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
    {
        if(pNode->getDataDescription(i)->varType == TLMType)        //C variable
        {
            cVariables << pNode->getDataDescription(i)->shortname.c_str();
            variableLabels << QString(pNode->getDataDescription(i)->name.c_str());
            varIdx << pNode->getDataDescription(i)->id;
        }
    }

    hopsanCore.removeNode(pNode);
}

void GeneratorNodeInfo::getNodeTypes(QStringList &nodeTypes)
{
    nodeTypes << "NodeMechanic" << "NodeMechanicRotational" << "NodeHydraulic" << "NodePneumatic" << "NodeElectric";
}


void hopsanLogger(jm_callbacks *c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
    if(log_level == jm_log_level_error || log_level == jm_log_level_fatal)
    {
        //printErrorMessage(QString("Module = %s, log level = %d: %s\n").arg(module).arg(log_level).arg(message));
    }
    else if(log_level == jm_log_level_error || log_level == jm_log_level_fatal)
    {
        //printWarningMessage(QString("Module = %s, log level = %d: %s\n").arg(module).arg(log_level).arg(message));
    }
    else
    {
        //printMessage(QString("Module = %s, log level = %d: %s\n").arg(module).arg(log_level).arg(message));
    }
}




InterfacePortSpec::InterfacePortSpec(InterfaceTypesEnumT type, QString component, QString port, QStringList path)
{
    this->type = type;
    this->path = path;
    this->component = component;
    this->port = port;

    QStringList inputDataNames;
    QStringList outputDataNames;
    QList<size_t> dataIds;

    switch(type)
    {
    case InterfacePortSpec::Input:
        inputDataNames << "";
        dataIds << 0;
        break;
    case InterfacePortSpec::Output:
        outputDataNames << "";
        dataIds << 0;
        break;
    case InterfacePortSpec::MechanicQ:
        inputDataNames << GeneratorNodeInfo("NodeMechanic").qVariables;
        outputDataNames << GeneratorNodeInfo("NodeMechanic").cVariables;
        dataIds << GeneratorNodeInfo("NodeMechanic").varIdx;
        break;
    case InterfacePortSpec::MechanicC:
        inputDataNames << GeneratorNodeInfo("NodeMechanic").cVariables;
        outputDataNames << GeneratorNodeInfo("NodeMechanic").qVariables;
        dataIds << GeneratorNodeInfo("NodeMechanic").varIdx;
        break;
    case InterfacePortSpec::MechanicRotationalQ:
        inputDataNames << GeneratorNodeInfo("NodeMechanicRotational").qVariables;
        outputDataNames << GeneratorNodeInfo("NodeMechanicRotational").cVariables;
        dataIds << GeneratorNodeInfo("NodeMechanicRotational").varIdx;
        break;
    case InterfacePortSpec::MechanicRotationalC:
        inputDataNames << GeneratorNodeInfo("NodeMechanicRotational").cVariables;
        outputDataNames << GeneratorNodeInfo("NodeMechanicRotational").qVariables;
        dataIds << GeneratorNodeInfo("NodeMechanicRotational").varIdx;
        break;
    case InterfacePortSpec::HydraulicQ:
        inputDataNames << GeneratorNodeInfo("NodeHydraulic").qVariables;
        outputDataNames << GeneratorNodeInfo("NodeHydraulic").cVariables;
        dataIds << GeneratorNodeInfo("NodeHydraulic").varIdx;
        break;
    case InterfacePortSpec::HydraulicC:
        inputDataNames << GeneratorNodeInfo("NodeHydraulic").cVariables;
        outputDataNames << GeneratorNodeInfo("NodeHydraulic").qVariables;
        dataIds << GeneratorNodeInfo("NodeHydraulic").varIdx;
        break;
    case InterfacePortSpec::PneumaticQ:
        inputDataNames << GeneratorNodeInfo("NodePneumatic").qVariables;
        outputDataNames << GeneratorNodeInfo("NodePneumatic").cVariables;
        dataIds << GeneratorNodeInfo("NodePneumatic").varIdx;
        break;
    case InterfacePortSpec::PneumaticC:
        inputDataNames << GeneratorNodeInfo("NodePneumatic").cVariables;
        outputDataNames << GeneratorNodeInfo("NodePneumatic").qVariables;
        dataIds << GeneratorNodeInfo("NodePneumatic").varIdx;
        break;
    case InterfacePortSpec::ElectricQ:
        inputDataNames << GeneratorNodeInfo("NodeElectric").qVariables;
        outputDataNames << GeneratorNodeInfo("NodeElectric").cVariables;
        dataIds << GeneratorNodeInfo("NodeElectric").varIdx;
        break;
    case InterfacePortSpec::ElectricC:
        inputDataNames << GeneratorNodeInfo("NodeElectric").cVariables;
        outputDataNames << GeneratorNodeInfo("NodeElectric").qVariables;
        dataIds << GeneratorNodeInfo("NodeElectric").varIdx;
        break;
    default:
        break;
    }

    foreach(const QString &dataName, inputDataNames)
    {
        vars.append(InterfaceVarSpec(dataName, dataIds.takeFirst(), InterfaceVarSpec::Input));
    }
    foreach(const QString &dataName, outputDataNames)
    {
        vars.append(InterfaceVarSpec(dataName, dataIds.takeFirst(), InterfaceVarSpec::Output));
    }
}



void getInterfaces(QList<InterfacePortSpec> &interfaces, ComponentSystem *pSystem, QStringList &path)
{
    std::vector<HString> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        Component *pComponent = pSystem->getSubComponent(names[i]);
        HString typeName = pComponent->getTypeName();
        if(typeName == "SignalInputInterface")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::Input, names[i].c_str(), "out", path));
        }
        else if(typeName == "SignalOutputInterface")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::Output, names[i].c_str(), "in", path));
        }
        else if(typeName == "MechanicInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicRotationalInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicRotationalQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicRotationalInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicRotationalC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "HydraulicQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::HydraulicQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "HydraulicC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::HydraulicC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PneumaticQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PneumaticQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PneumaticC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PneumaticC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "ElectricQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::ElectricQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "ElectricC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::ElectricC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "Subsystem")
        {
            getInterfaces(interfaces, dynamic_cast<hopsan::ComponentSystem *>(pComponent), path << pComponent->getName().c_str());
        }
    }
}



InterfaceVarSpec::InterfaceVarSpec(QString dataName, int dataId, InterfaceVarSpec::CausalityEnumT causality)
{
    this->dataName = dataName;
    this->dataId = dataId;
    this->causality = causality;
}
