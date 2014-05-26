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
//! @brief Contains component generation utiluties
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

#include <cassert>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "generators/HopsanGenerator.h"
#include "SymHop.h"

#include "ComponentSystem.h"
#include "Port.h"
#include "version.h"


using namespace std;
using namespace SymHop;
using namespace hopsan;


HopsanGenerator::HopsanGenerator(const QString coreIncludePath, const QString binPath, const bool showDialog)
{
#ifdef WIN32
    mOutputPath = "C:/HopsanGeneratorTempFiles/output/";
    mTempPath = "C:/HopsanGeneratorTempFiles/temp/";
#else
    mOutputPath = QDir::currentPath()+"/output/";
    mTempPath = QDir::currentPath()+"/temp/";
#endif
    mTarget = "";
    mCoreIncludePath = coreIncludePath;
    mBinPath = binPath;

    mShowDialog = showDialog;

    if(mShowDialog)
    {
        mpTextEdit = new QTextEdit();
        mpTextEdit->setReadOnly(true);
        QFont monoFont = QFont("Monospace", 10, 50);
        monoFont.setStyleHint(QFont::TypeWriter);
        mpTextEdit->setFont(monoFont);

        mpDoneButton = new QPushButton("Close");
        mpDoneButton->setFixedWidth(200);

        mpLayout = new QVBoxLayout();
        mpLayout->addWidget(mpTextEdit);
        mpLayout->addWidget(mpDoneButton);
        mpLayout->setAlignment(mpDoneButton, Qt::AlignCenter);

        mpDialog = new QWidget(0);
        mpDialog->setWindowModality(Qt::ApplicationModal);
        mpDialog->setLayout(mpLayout);
        mpDialog->setMinimumSize(640, 480);
        mpDialog->setWindowTitle("HopsanGenerator");

        mpDoneButton->connect(mpDoneButton, SIGNAL(clicked()), mpDialog, SLOT(hide()));

        mpDialog->show();
        QApplication::processEvents();

        printMessage("##########################\n# Loaded HopsanGenerator #\n##########################\n");
    }
}


void HopsanGenerator::printMessage(const QString &msg) const
{
    if(mShowDialog)
    {
        mpTextEdit->setTextColor(QColor("Black"));
        mpTextEdit->append(msg);
        QApplication::processEvents();
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }
    else
    {
        //qDebug() << msg;
    }
}


void HopsanGenerator::printErrorMessage(const QString &msg) const
{
    if(mShowDialog)
    {
        mpTextEdit->setTextColor(QColor("Red"));
        mpTextEdit->append(msg);
        QApplication::processEvents();
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }
    else
    {
        //qDebug() << msg;
    }
}















QString HopsanGenerator::generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues) const
{
    if(comp.cqsType == "S") { comp.cqsType = "Signal"; }


    //Declare variables
    QString varDeclarations;
    for(int i=0; i<comp.parNames.size(); ++i)
    {
        varDeclarations.append("        double "+comp.parNames[i]+";\n");
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        varDeclarations.append("        "+comp.varTypes[i]+" "+comp.varNames[i]+";\n");
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            varDeclarations.append("        "+comp.varTypes[i]+" *mpOUTPUT_"+comp.varNames[i]+";\n");
    }
    for(int i=0; i<comp.utilities.size(); ++i)
    {
        varDeclarations.append("        "+comp.utilities[i]+" "+comp.utilityNames[i]+";\n");
    }
    int portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }
        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            varDeclarations.append("        double "+varName+";\n");
        }
        ++portId;
    }


    //Declare node data pointers
    QString dataPtrDeclarations = "        double ";
    portId=1;
    QStringList allVarNames;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QString id = QString::number(portId);
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            allVarNames << comp.portNames[i];
        }
        else
        {
            QStringList vars;
            vars << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;

            for(int v=0; v<vars.size(); ++v)
            {
                allVarNames << vars[v]+id;
            }
        }
        ++portId;
    }
    if(!allVarNames.isEmpty())
    {
        dataPtrDeclarations.append("*mpND_"+allVarNames[0]);
        for(int i=1; i<allVarNames.size(); ++i)
        {
            dataPtrDeclarations.append(", *mpND_"+allVarNames[i]);
        }
    }
    dataPtrDeclarations.append(";\n");


    //Declare ports
    QString portDeclarations = "        Port ";
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        portDeclarations.append("*mp"+comp.portNames[i]);
        if(i<comp.portNames.size()-1)
        {
            portDeclarations.append(", ");
        }
    }
    portDeclarations.append(";\n");


//    //Initialize parameters
//    QString parameterInit;
//    for(int i=0; i<comp.parNames.size(); ++i)
//    {
//        parameterInit.append("            "+comp.parNames[i]+" = "+comp.parInits[i]+";\n");
//    }


    //Register parameters
    QString registerParameters;
    for(int i=0; i<comp.parNames.size(); ++i)
    {
        registerParameters.append("            addConstant(\""+comp.parDisplayNames[i]+"\", \""
                  +comp.parDescriptions[i]+"\", \""+comp.parUnits[i]+"\", "+comp.parInits[i]+", "+comp.parNames[i]+");\n");
    }


    //Add ports
    QString addPorts;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            QString init = comp.portDefaults[i];
            if(init.isEmpty())
            {
                init = "0";
            }
            if(comp.portTypes[i] == "ReadPort")
            {
                addPorts.append("            addInputVariable(\""+comp.portNames[i]+"\", \"\", \"\", "+init+", &mpND_"+comp.portNames[i]+");\n");
            }
            else if(comp.portTypes[i] == "WritePort")
            {
                addPorts.append("            addOutputVariable(\""+comp.portNames[i]+"\", \"\", \"\", "+init+", &mpND_"+comp.portNames[i]+");\n");
            }
        }
        else
        {
            addPorts.append("            mp"+comp.portNames[i]+" = add"+comp.portTypes[i]
                      +"(\""+comp.portNames[i]+"\", \""+comp.portNodeTypes[i]+"\"");
            if(comp.portNotReq[i])
            {
                addPorts.append(", Port::NotRequired);\n");
            }
            else
            {
                addPorts.append(");\n");
            }
        }
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            addPorts.append("            addOutputVariable(\""+comp.varNames[i]+"\", \"\", \"\", 0, &mpOUTPUT_"+comp.varNames[i]+");\n");
    }


    //Initialize variables
    QString initializeVariables;
    for(int i=0; i<comp.varInits.size(); ++i)
    {
        if(!comp.varInits[i].isEmpty())
        {
            initializeVariables.append("            "+comp.varNames[i]+" = "+comp.varInits[i]+";\n");
        }
    }

    //Get data pointers
    QString getDataPtrs;
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        QStringList varLabels;
        if(comp.portNodeTypes[i] != "NodeSignal")
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
            varLabels << GeneratorNodeInfo(comp.portNodeTypes[i]).variableLabels;
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v]+QString::number(portId);
            getDataPtrs.append("            mpND_"+varName+" = getSafeNodeDataPtr(mp"+comp.portNames[i]+", "+comp.portNodeTypes[i]+"::"+varLabels[v]);
            if(comp.portNotReq[i])
            {
                getDataPtrs.append(", "+comp.portDefaults[i]);
            }
            getDataPtrs.append(");\n");
        }
        ++portId;
    }


    //Read input variables
    QString readInputs;
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            readInputs.append("            "+varName+" = (*mpND_"+varName+");\n");
        }
        ++portId;
    }


    //Configure code
    QString confCode;
    for(int i=0; i<comp.confEquations.size(); ++i)
    {
        confCode.append("            "+comp.confEquations[i]+"\n");
    }

    //Initialize code
    QString initCode;
    for(int i=0; i<comp.initEquations.size(); ++i)
    {
        initCode.append("            "+comp.initEquations[i]+"\n");
    }


    //Write back values
    QString writeOutputs;
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "WritePort")
        {
            varNames << comp.portNames[i];
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "Q" || comp.cqsType == "S"))
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables;
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }
        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            writeOutputs.append("            (*mpND_"+varName+") = "+varName+";\n");
        }
        ++portId;
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            writeOutputs.append("            (*mpOUTPUT_"+comp.varNames[i]+") = "+comp.varNames[i]+";\n");
    }

    QString writeStartValues = "";
    if(overwriteStartValues)
    {
        writeStartValues = writeOutputs;
    }


    //Simulate code
    QString simCode;
    for(int i=0; i<comp.simEquations.size(); ++i)
    {
        simCode.append("            "+comp.simEquations[i]+"\n");
    }


    //Finalize code
    QString finalCode;
    for(int i=0; i<comp.finalEquations.size(); ++i)
    {
        finalCode.append("            "+comp.finalEquations[i]+"\n");
    }


    //Deconfiguration code
    QString deconfCode;
    for(int i=0; i<comp.deconfEquations.size(); ++i)
    {
        deconfCode.append("            "+comp.deconfEquations[i]+"\n");
    }


    //Auxiliary functions
    QString auxiliaryFunctions;
    for(int i=0; i<comp.auxiliaryFunctions.size(); ++i)
    {
        auxiliaryFunctions.append("        "+comp.auxiliaryFunctions[i]+"\n");
    }


    QFile compTemplateFile(":templates/generalComponentTemplate.hpp");
    if(!compTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open generalComponentTemplate.hpp for writing.");
        return "";
    }

    QString code;
    QTextStream t(&compTemplateFile);
    code = t.readAll();
    compTemplateFile.close();
    if(code.isEmpty())
    {
        printErrorMessage("Failed to generate code for "+comp.typeName+".hpp.");
        return "";
    }

    code.replace("<<<uppertypename>>>", comp.typeName.toUpper());
    code.replace("<<<typename>>>", comp.typeName);
    code.replace("<<<cqstype>>>", comp.cqsType);
    code.replace("<<<vardecl>>>", varDeclarations);
    code.replace("<<<dataptrdecl>>>", dataPtrDeclarations);
    code.replace("<<<portdecl>>>", portDeclarations);
    //code.replace("<<<parinit>>>", parameterInit);
    code.replace("<<<regpar>>>", registerParameters);
    code.replace("<<<addports>>>", addPorts);
    code.replace("<<<initvars>>>", initializeVariables);
    code.replace("<<<getdataptrs>>>", getDataPtrs);
    code.replace("<<<readinputs>>>", readInputs);
    code.replace("<<<confcode>>>", confCode);
    code.replace("<<<initcode>>>", initCode);
    code.replace("<<<writestartvalues>>>", writeStartValues);
    code.replace("<<<simulatecode>>>", simCode);
    code.replace("<<<writeoutputs>>>", writeOutputs);
    code.replace("<<<finalcode>>>", finalCode);
    code.replace("<<<deconfcode>>>", deconfCode);
    code.replace("<<<auxiliaryfunctions>>>", auxiliaryFunctions);

    return code;
}


//! @brief Generates appearance file from a component specification object
//! @param path Path to generated (or existing) .xml file
//! @param comp Component specification object
void HopsanGenerator::generateOrUpdateComponentAppearanceFile(QString path, ComponentSpecification comp, QString sourceFile)
{
    QFile file(path);
    QString code;
    if(file.exists())   //An xml file already exists
    {
        file.open(QFile::ReadOnly);
        code = file.readAll();
        file.close();

        QStringList lines = code.split("\n");

        //! @todo Update typename and displayname if they have changed

        //Remove non-existing ports
        for(int l=0; l<lines.size(); ++l)
        {
            if(lines[l].trimmed().startsWith("<port "))
            {
                QString portName = lines[l].section("name=\"",1,1).section("\"",0,0);
                if(!comp.portNames.contains(portName))
                {
                    lines.removeAt(l);
                    --l;
                }
            }
        }

        //Add existing ports missing in XML
        for(int p=0; p<comp.portNames.size(); ++p)
        {
            bool found=false;
            for(int l=0; l<lines.size(); ++l)
            {
                if(lines[l].trimmed().startsWith("<port "))
                {
                    QString portName = lines[l].section("name=\"",1,1).section("\"",0,0);
                    if(portName == comp.portNames[p])
                    {
                        found=true;
                    }
                }
            }

            if(!found)
            {
                for(int l=0; l<lines.size(); ++l)
                {
                    if(lines[l].trimmed().startsWith("</ports>"))
                    {
                        lines.insert(l, "      <port name=\""+comp.portNames[p]+"\" x=\"0.0\" y=\"0.0\" a=\"0.0\"/>");
                        break;
                    }
                }
            }
        }

        code = lines.join("\n");

        file.open(QFile::ReadOnly | QFile::Text | QFile::Truncate);
        file.write(code.toUtf8());
        file.close();
    }
    else    //No existing XML file
    {
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            printErrorMessage("Failed to open " + comp.typeName + ".xml  for writing.");
            return;
        }
        QTextStream xmlStream(&file);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
        QString sourceString;
        if(!sourceFile.isEmpty())
        {
            sourceString = " sourcecode=\""+sourceFile+"\"";
        }
        xmlStream << "  <modelobject typename=\"" << comp.typeName << "\" displayname=\"" << comp.displayName << "\" libpath=\"\""+sourceString+">\n";
        xmlStream << "    <icons/>\n";
        xmlStream << "    <ports>\n";
        double xDelay = 1.0/(comp.portNames.size()+1.0);
        double xPos = xDelay;
        double yPos = 0;
        for(int i=0; i<comp.portNames.size(); ++i)
        {
            xmlStream << "      <port name=\"" << comp.portNames[i] << "\" x=\"" << xPos << "\" y=\"" << yPos << "\" a=\"" << 270 << "\"/>\n";
            xPos += xDelay;
        }
        xmlStream << "    </ports>\n";
        xmlStream << "  </modelobject>\n";
        xmlStream << "</hopsanobjectappearance>\n";
        file.close();
    }
}







//! @brief Generates and compiles component source code from a ComponentSpecification object
//! @param outputFile Name of output file
//! @param comp Component specification object
//! @param overwriteStartValues Tells whether or not this components overrides the built-in start values or not
void HopsanGenerator::compileFromComponentObject(const QString &outputFile, const ComponentSpecification &comp, const bool overwriteStartValues, const QString customSourceFile)
{
    QString code;

    if(comp.plainCode.isEmpty())
    {
        code = generateSourceCodefromComponentObject(comp, overwriteStartValues);
    }
    else
    {
        code = comp.plainCode;
    }

    //qDebug() << "Code: " << code;

    if(!QDir(mTempPath).exists())
    {
        QDir().mkpath(mTempPath);
    }

    if(!QDir(mOutputPath).exists())
    {
        QDir().mkpath(mOutputPath);
    }

    printMessage("Writing "+outputFile+".hpp...");

    //Initialize the file stream
    QFileInfo fileInfo;
    QFile file;
    fileInfo.setFile(QString(mTempPath)+outputFile+".hpp");
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open file for writing: " + outputFile+".hpp");
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    fileStream << code;

    file.close();

    printMessage("Writing tempLib.cc...");

    QFile ccLibFile;
    ccLibFile.setFileName(QString(mTempPath)+"tempLib.cc");
    if(!ccLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open tempLib.cc for writing.");
        return;
    }
    QTextStream ccLibStream(&ccLibFile);
    ccLibStream << "#include \"" << outputFile << ".hpp\"\n";
    ccLibStream << "#include \"ComponentEssentials.h\"\n\n";
    ccLibStream << "using namespace hopsan;\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* /*nfact_ptr*/)\n";
    ccLibStream << "{\n";
    ccLibStream << "    cfact_ptr->registerCreatorFunction(\"" << comp.typeName<< "\", " << comp.typeName << "::Creator);\n";
    ccLibStream << "}\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n";
    ccLibStream << "{\n";
    ccLibStream << "    pHopsanExternalLibInfo->libName = (char*)\"HopsanGeneratedComponent_"+comp.typeName+"\";\n";
    ccLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    printMessage("Writing " + comp.typeName + ".xml...");

    QFile xmlFile;
    xmlFile.setFileName(QString(mOutputPath)+comp.typeName+".xml");
    if(!xmlFile.exists())
    {
        if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            printErrorMessage("Failed to open " + comp.typeName + ".xml  for writing.");
            return;
        }
        QTextStream xmlStream(&xmlFile);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
        QString sourceFile = customSourceFile;
        if(sourceFile.isEmpty()) sourceFile = outputFile+".hpp";
        xmlStream << "  <modelobject typename=\"" << comp.typeName << "\" displayname=\"" << comp.displayName << "\" sourcecode=\""+sourceFile+"\" libpath=\"\" recompilable=\"true\">\n";
        xmlStream << "    <icons/>\n";
        xmlStream << "    <ports>\n";
        double xDelay = 1.0/(comp.portNames.size()+1.0);
        double xPos = xDelay;
        double yPos = 0;
        for(int i=0; i<comp.portNames.size(); ++i)
        {
            xmlStream << "      <port name=\"" << comp.portNames[i] << "\" x=\"" << xPos << "\" y=\"" << yPos << "\" a=\"" << 270 << "\"/>\n";
            xPos += xDelay;
        }
        xmlStream << "    </ports>\n";
        xmlStream << "  </modelobject>\n";
        xmlStream << "</hopsanobjectappearance>\n";
        xmlFile.close();
    }

    QString libFileName = outputFile;
#ifdef WIN32
    libFileName.append(".dll");
#else
    libFileName.append(".so");
#endif

    //! @todo Make this work again!
    //compileComponentLibrary(mTempPath, outputFile, this);

    printMessage("Moving files to output directory...");

    QFile soFile(mTempPath+libFileName);
    QFile targetSoFile(mOutputPath+libFileName);
    if(targetSoFile.exists() && !targetSoFile.remove())
    {
        printErrorMessage("Failed to remove existing binary file:"+targetSoFile.errorString());
        return;
    }
    targetSoFile.close();

    //qDebug() << "Copying "+soFile.fileName()+" to "+mOutputPath+libFileName;
    if(!soFile.copy(mOutputPath + libFileName))
    {
        printErrorMessage("Failed to copy new binary to output directory: "+soFile.errorString());
        return;
    }
    if(!file.copy(mOutputPath+fileInfo.fileName()))
    {
        printErrorMessage("Failed to copy source code file to output directory: "+file.errorString());
    }
    QFileInfo ccLibFileInfo(ccLibFile);
    if(!ccLibFile.copy(mOutputPath+ccLibFileInfo.fileName()))
    {
        printMessage("Unable to copy library source file to output directory: "+ccLibFile.errorString() + " (not required)");
    }
}


//! @brief Generates library .cc and .xml files for a folder with .hpp files
//! @param path Path to library
//! @param hppFiles Relative path to hpp files
void HopsanGenerator::generateNewLibrary(QString path, QStringList hppFiles)
{
    printMessage("Creating new component library...");

    QString libName = QDir(path).dirName();

    QStringList typeNames;
    Q_FOREACH(const QString &file, hppFiles)
    {
        QFile hppFile(path+file);
        hppFile.open(QFile::ReadOnly);
        QString code = hppFile.readAll();
        hppFile.close();

        typeNames.append(code.section("class ",1,1).section(" : public",0,0).trimmed());
    }

    printMessage("Writing "+libName+".cc...");

    QFile ccLibFile;
    ccLibFile.setFileName(path+libName+".cc");
    if(!ccLibFile.open(QFile::WriteOnly | QFile::Text))
    {
        printErrorMessage("Failed to open "+libName+".cc for writing.");
        return;
    }
    QTextStream ccLibStream(&ccLibFile);
    Q_FOREACH(const QString &file, hppFiles)
    {
        ccLibStream << "#include \"" << file << "\"\n";
    }
    ccLibStream << "#include \"ComponentEssentials.h\"\n\n";
    ccLibStream << "using namespace hopsan;\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* /*nfact_ptr*/)\n";
    ccLibStream << "{\n";
    Q_FOREACH(const QString &type, typeNames)
    {
        ccLibStream << "    cfact_ptr->registerCreatorFunction(\"" << type << "\", " << type << "::Creator);\n";
    }
    ccLibStream << "}\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n";
    ccLibStream << "{\n";
    ccLibStream << "    pHopsanExternalLibInfo->libName = (char*)\"" << libName << "\";\n";
    ccLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    printMessage("Writing " + libName + "_lib.xml...");

    QFile xmlFile;
    xmlFile.setFileName(path+libName+"_lib.xml");
    if(!xmlFile.exists())
    {
        if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            printErrorMessage("Failed to open " + libName + "_lib.xml  for writing.");
            return;
        }
        QTextStream xmlStream(&xmlFile);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        xmlStream << "<hopsancomponentlibrary xmlversion=\"0.1\" libversion=\"1\" name=\""+libName+"\">\n";
        xmlStream << "  <lib>" << libName  << "</lib>\n";
        xmlStream << "  <source>" << libName << ".cc</source>\n";
        xmlStream << "</hopsancomponentlibrary>\n";
        xmlFile.close();
    }

    printMessage("Finished.");
}



void HopsanGenerator::setOutputPath(const QString path)
{
    mOutputPath = path;
    printMessage("Setting output path: " + path);
}


void HopsanGenerator::setTarget(const QString fileName)
{
    mTarget = fileName;
    printMessage("Setting target: " + fileName);
}


QString HopsanGenerator::getCoreIncludePath() const
{
    return mCoreIncludePath;
}


QString HopsanGenerator::getBinPath() const
{
    return mBinPath;
}


bool HopsanGenerator::assertFilesExist(const QString &path, const QStringList &files) const
{
    Q_FOREACH(const QString file, files)
    {
        if(!QFile::exists(path+"/"+file))
        {
            printErrorMessage("File not found: "+file+".");
            return false;
        }
    }
    printMessage("All required files found.");
    return true;
}


void HopsanGenerator::callProcess(const QString &name, const QStringList &args, const QString workingDirectory) const
{
    QProcess p;
    if(!workingDirectory.isEmpty())
    {
        p.setWorkingDirectory(workingDirectory);
    }
    p.start(name, args);
    p.waitForFinished(60000);
    printMessage(p.readAll());
    printMessage(p.readAllStandardError());
    printMessage(p.readAllStandardOutput());
}


bool HopsanGenerator::runUnixCommand(QString cmd) const
{
    char line[130];
    cmd +=" 2>&1";
    FILE *fp = popen(  (const char *) cmd.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + cmd + "'! err=%d");
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
           printMessage((const QString &)line);
        }
    }
    return true;
}


//! @warning May delete contents in file if it fails to open in write mode
bool HopsanGenerator::replaceInFile(const QString &fileName, const QStringList &before, const QStringList &after) const
{
    Q_ASSERT(before.size() == after.size());

    QFile file(fileName);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        printErrorMessage("Unable to open file: "+fileName+" for reading.");
        return false;
    }
    QString contents = file.readAll();
    file.close();

    bool didSomething = false;
    for(int i=0; i<before.size(); ++i)
    {
        if(contents.contains(before[i]))
        {
            didSomething = true;
            contents.replace(before[i], after[i]);
        }
    }

    if(!didSomething)
    {
        return true;
    }

    if(!file.open(QFile::ReadWrite | QFile::Truncate | QFile::Text))
    {
        printErrorMessage("Unable to open file "+fileName+" for writing.");
    }
    file.write(contents.toLatin1());
    file.close();

    return true;
}



//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
//! @todo Weird name because of name conflict with HopsanGUI
bool HopsanGenerator::copyIncludeFilesToDir(QString path, bool skipDependencies) const
{
    printMessage("Copying HopsanCore include files...");

    if(!path.endsWith("/") && !path.endsWith("\\"))
        path.append("/");

    //Make sure HopsanCore include files are available
    QStringList includeFiles = getHopsanCoreIncludeFiles(skipDependencies);

    if(!assertFilesExist(mBinPath, includeFiles))
        return false;

    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("HopsanCore/include");
    saveDir.mkpath("HopsanCore/include/Components");
    saveDir.mkpath("HopsanCore/include/ComponentUtilities");
    saveDir.mkpath("HopsanCore/include/CoreUtilities");
    saveDir.mkpath("componentLibraries/defaultLibrary/code");
    saveDir.mkpath("HopsanCore/Dependencies/libcsv_parser++-1.0.0/include/csv_parser");
    saveDir.mkpath("HopsanCore/Dependencies/rapidxml-1.13");

    Q_FOREACH(const QString &file, includeFiles)
    {
        if(!copyFile(mBinPath+file, path+file.right(file.size()-3))) return false;
        QFile::setPermissions(path+file.right(file.size()-3), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser | QFile::ReadOther | QFile::WriteOther);
    }

    return true;
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
bool HopsanGenerator::copySourceFilesToDir(QString path) const
{
    printMessage("Copying HopsanCore source files...");

    if(!path.endsWith("/") && !path.endsWith("\\"))
        path.append("/");

    //Make sure HopsanCore source files are available
    QStringList srcFiles = getHopsanCoreSourceFiles();
    if(!assertFilesExist(mBinPath, srcFiles))
        return false;

    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("HopsanCore/src/ComponentUtilities");
    saveDir.mkpath("HopsanCore/src/CoreUtilities");
    saveDir.mkpath("componentLibraries/defaultLibrary/code");
    saveDir.mkpath("HopsanCore/Dependencies/libcsv_parser++-1.0.0");

    Q_FOREACH(const QString &file, srcFiles)
    {
        if(!copyFile(mBinPath+file, path+file.right(file.size()-3))) return false;
        QFile::setPermissions(path+file.right(file.size()-3), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser | QFile::ReadOther | QFile::WriteOther);
    }

    return true;
}



//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
//! @todo Error checking
bool HopsanGenerator::copyDefaultComponentCodeToDir(const QString &path) const
{
    printMessage("Copying default component library...");

    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("componentLibraries/defaultLibrary");
    saveDir.cd("componentLibraries");
    saveDir.cd("defaultLibrary");

    copyDir( QString(mBinPath+"../componentLibraries/defaultLibrary"), saveDir.path() );

    QStringList allFiles;
    findAllFilesInFolderAndSubFolders(saveDir.path(),"hpp",allFiles);
    findAllFilesInFolderAndSubFolders(saveDir.path(),"h",allFiles);
    findAllFilesInFolderAndSubFolders(saveDir.path(),"cc",allFiles);
    findAllFilesInFolderAndSubFolders(saveDir.path(),"cpp",allFiles);

    Q_FOREACH(const QString file, allFiles)
    {
        QFile::setPermissions(file, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser | QFile::ReadOther | QFile::WriteOther);
    }

    return true;
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
//! @todo Error checking
bool HopsanGenerator::copyBoostIncludeFilesToDir(const QString &path) const
{
    printMessage("Copying Boost include files...");

    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("include/boost");
    saveDir.cd("include");
    saveDir.cd("boost");

    copyDir( QString("../HopsanCore/Dependencies/boost"), saveDir.path() );

    saveDir.cd("bin");
    QStringList binFiles = saveDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    Q_FOREACH(const QString &fileName, binFiles)
    {
        QFile file(path+"/include/boost/bin/"+fileName);
        //qDebug() << "File: " << path+"/include/boost/bin/"+fileName;
        file.copy(path+"/"+fileName);
        //qDebug() << "Target: " << path << "/" << fileName;
    }

    return true;
}


//! @brief Copies a file to a target and informs user of the outcome
//! @param source Source file
//! @param target Target where to copy file
//! @returns True if copy successful, otherwise false
bool HopsanGenerator::copyFile(const QString &source, const QString &target) const
{
    QFile sourceFile;
    sourceFile.setFileName(source);
    QFileInfo sourceFileInfo(sourceFile);
    if(QFile::exists(target))
    {
        QFile::remove(target);
    }
    if(!sourceFile.copy(target))
    {
        printErrorMessage("Unable to copy file: " +sourceFile.fileName() + " to " + target+".");
        return false;
    }
    printMessage("Copying " + sourceFileInfo.fileName()+"...");
    return true;
}


//! @brief Cleans up after import/export operation by removing all specified files and sub directories
//! @param path Path to directory to clean up
//! @param files List of files to remove
//! @param subDirs List of sub directories to remove
void HopsanGenerator::cleanUp(const QString &path, const QStringList &files, const QStringList &subDirs) const
{
    printMessage("Cleaning up directory: " + path);

    QDir cleanDir(path);
    Q_FOREACH(const QString &file, files)
    {
        cleanDir.remove(file);
    }
    Q_FOREACH(const QString &subDir, subDirs)
    {
        removeDir(path+"/"+subDir);
    }
}



void HopsanGenerator::getNodeAndCqTypeFromInterfaceComponent(const QString &compType, QString &nodeType, QString &cqType)
{
    if(compType == "HydraulicInterfaceC")
    {
        nodeType = "NodeHydraulic";
        cqType = "c";
    }
    else if(compType == "HydraulicInterfaceQ")
    {
        nodeType = "NodeHydraulic";
        cqType = "q";
    }
    if(compType == "MechanicInterfaceC")
    {
        nodeType = "NodeMechanic";
        cqType = "c";
    }
    else if(compType == "MechanicInterfaceQ")
    {
        nodeType = "NodeMechanic";
        cqType = "q";
    }
    if(compType == "MechanicRotationalInterfaceC")
    {
        nodeType = "NodeMechanicRotational";
        cqType = "c";
    }
    else if(compType == "MechanicRotationalInterfaceQ")
    {
        nodeType = "NodeMechanicRotational";
        cqType = "q";
    }
    if(compType == "PneumaticInterfaceC")
    {
        nodeType = "NodePneumatic";
        cqType = "c";
    }
    else if(compType == "PneumaticInterfaceQ")
    {
        nodeType = "NodePneumatic";
        cqType = "q";
    }
    if(compType == "ElectricInterfaceC")
    {
        nodeType = "NodeElectric";
        cqType = "c";
    }
    else if(compType == "ElectricInterfaceQ")
    {
        nodeType = "NodeElectric";
        cqType = "q";
    }
}
