#include "generators/HopsanFMIGenerator.h"
#include "GeneratorUtilities.h"
#include "ComponentSystem.h"
#include <QApplication>
#include <cassert>
#include <QProcess>
#include <QUuid>

//#include "../ThirdParty/FMILibrary-2.0.1/Config.cmake/fmilib.h"

#include <stddef.h>
//#include "fmilib_config.h"

#include "FMI/fmi_import_context.h"
#include <FMI1/fmi1_import.h>
#include <FMI2/fmi2_import.h>

#include <JM/jm_portability.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace hopsan;



HopsanFMIGenerator::HopsanFMIGenerator(QString coreIncludePath, QString binPath, bool showDialog)
    : HopsanGenerator(coreIncludePath, binPath, showDialog)
{

}


//! @brief Generates a Hopsan component from a Functional Mockup Unit using FMI 2.0 standard
//! @param path Path to .FMU file
//! @param targetPath Path where to extract files and generate component
//! @param rTypeName Reference to string containing the type name of the generated component
//! @param rHppPath Reference to string containing the path to the generated .HPP file
bool HopsanFMIGenerator::generateFromFmu2(QString path, QString targetPath, QString &rTypeName, QString &rHppPath)
{
    //----------------------------------------//
    printMessage("Initializing FMU 2.0 import");
    printMessage("path = "+path);
    printMessage("targetPath = "+targetPath);
    //----------------------------------------//

    fmi2_callback_functions_t callBackFunctions;
    QByteArray pathArray = path.toLocal8Bit();
    const char* FMUPath = pathArray.data();
    targetPath = targetPath+QFileInfo(path).baseName();
    if(!QFileInfo(targetPath).exists())
    {
        QDir().mkpath(targetPath);
    }
    QByteArray targetArray = targetPath.toLocal8Bit();
    const char* tmpPath = targetArray.data();
    jm_callbacks callbacks;
    fmi_import_context_t* context;
    fmi_version_enu_t version;
    jm_status_enu_t status;
    //int k;

    fmi2_import_t* fmu;

    callbacks.malloc = malloc;
    callbacks.calloc = calloc;
    callbacks.realloc = realloc;
    callbacks.free = free;
    callbacks.logger = hopsanLogger;
    callbacks.log_level = jm_log_level_debug;
    callbacks.context = 0;

    context = fmi_import_allocate_context(&callbacks);

    version = fmi_import_get_fmi_version(context, FMUPath, tmpPath);

    if(version != fmi_version_2_0_enu) {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage("The code only supports version 2.0\n");

        return false;
    }

    //-----------------------------------------//
    printMessage("Reading modelDescription.xml");
    //-----------------------------------------//

    fmu = fmi2_import_parse_xml(context, tmpPath, 0);

    if(!fmu)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage("Error parsing XML, exiting\n");
        return false;
    }

    if(fmi2_import_get_fmu_kind(fmu) == fmi2_fmu_kind_me)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage("Only CS 2.0 is supported by this code\n");
        return false;
    }

    callBackFunctions.logger = fmi2_log_forwarding;
    callBackFunctions.allocateMemory = calloc;
    callBackFunctions.freeMemory = free;
    callBackFunctions.componentEnvironment = fmu;

    status = fmi2_import_create_dllfmu(fmu, fmi2_fmu_kind_cs, &callBackFunctions);
    if (status == jm_status_error)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage(QString("Could not create the DLL loading mechanism(C-API) (error: %1).\n").arg(fmi2_import_get_last_error(fmu)));
        return false;
    }

    //size_t nVars = fmi2_import_get_number_of_continuous_states(fmu);
    //qDebug() << QString("FMU has %1 variables!").arg(nVars);

    //Declare lists for parameters, input variables and output variables
    QStringList parNames, parVars, parRefs;
    QStringList inputNames, inputVars, inputRefs;
    QStringList outputNames, outputVars, outputRefs;

    //Loop through variables in FMU and generate the lists
    fmi2_import_variable_list_t *pVarList = fmi2_import_get_variable_list(fmu,0);
    for(size_t i=0; i<fmi2_import_get_variable_list_size(pVarList); ++i)
    {
        fmi2_import_variable_t *pVar = fmi2_import_get_variable(pVarList, i);
        QString name = fmi2_import_get_variable_name(pVar);
        fmi2_causality_enu_t causality = fmi2_import_get_causality(pVar);
        fmi2_base_type_enu_t type = fmi2_import_get_variable_base_type(pVar);
        fmi2_value_reference_t vr = fmi2_import_get_variable_vr(pVar);
        qDebug() << "Found variable: " << QString(name) << ", causality: " << causality << ", type: " << type << ", vr: " << vr;
        name = toVarName(name);//name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));
        if(causality == fmi2_causality_enu_parameter)
        {
            parNames.append(name);
            parVars.append("m"+name);
            parRefs.append(QString::number(vr));
        }
        if(causality == fmi2_causality_enu_input)
        {
            inputNames.append(name);
            inputVars.append("mp"+name);
            inputRefs.append(QString::number(vr));
        }
        if(causality == fmi2_causality_enu_output || causality == fmi2_causality_enu_local) //! @todo Local should be removed from here!
        {

            outputNames.append(name);
            outputVars.append("mp"+name);
            outputRefs.append(QString::number(vr));
        }
    }

    //Get name of FMU
    QString fmuName = fmi2_import_get_model_name(fmu);
    fmuName = toVarName(fmuName);//.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));

    //--------------------------------------------//
    printMessage("Creating " + fmuName + ".hpp...");
    //--------------------------------------------//

    //Create <fmuname>.hpp
    QString hppPath = targetPath + "/" + fmuName + "/component_code";
    if(!QFileInfo(hppPath).exists())
    {
        QDir().mkpath(hppPath);
    }
    QFile fmuComponentHppFile;
    fmuComponentHppFile.setFileName(hppPath+"/"+fmuName+".hpp");
    if(!fmuComponentHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".hpp for writing.");
        removeDir(targetPath);
        return false;
    }

    //-------------------------------------------//
    printMessage("Writing " + fmuName + ".hpp...");
    //-------------------------------------------//

    //Generate HPP file
    QFile fmuComponentTemplateFile;
    fmuComponentTemplateFile.setFileName(":templates/fmi2ComponentTemplate.hpp");
    assert(fmuComponentTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuComponentCode;
    QTextStream t2(&fmuComponentTemplateFile);
    fmuComponentCode = t2.readAll();
    fmuComponentTemplateFile.close();
    if(fmuComponentCode.isEmpty())
    {
        printErrorMessage("Unable to generate code for "+fmuName+".hpp.");
        removeDir(targetPath);
        return false;
    }

    QString headerGuard = fmuName.toUpper()+"_HPP_INCLUDED";
    QString className = "FMU_"+fmuName;

    QString localVars;
    if(!parVars.isEmpty())
    {
        localVars.append("double ");
        foreach(const QString &varName, parVars)
        {
            localVars.append(varName+", ");
        }
        if(localVars.endsWith(", "))
        {
            localVars.chop(2);
        }
        localVars.append(";\n");
    }
    if(!inputVars.isEmpty()  || !outputVars.isEmpty())
    {
        localVars.append("double ");
        foreach(const QString &varName, inputVars)
        {
            localVars.append("*"+varName+", ");
        }
        foreach(const QString &varName, outputVars)
        {
            localVars.append("*"+varName+", ");
        }
        if(localVars.endsWith(", "))
        {
            localVars.chop(2);
        }
        localVars.append(";\n");
    }

    QString addConstants;
    for(int i=0; i<parNames.size(); ++i)
    {
        if(i!=0)
        {
            addConstants.append("        ");
        }
        addConstants.append("addConstant(\""+parNames.at(i)+"\", \"\", \"\", 0, "+parVars.at(i)+");\n");
    }

    QString addInputs;
    for(int i=0; i<inputNames.size(); ++i)
    {
        if(i!=0)
        {
            addInputs.append("        ");
        }
        addInputs.append("addInputVariable(\""+inputNames.at(i)+"\", \"\", \"\", 0, &"+inputVars.at(i)+");\n");
    }

    QString addOutputs;
    for(int i=0; i<outputNames.size(); ++i)
    {
        if(i!=0)
        {
            addOutputs.append("        ");
        }
        addOutputs.append("addOutputVariable(\""+outputNames.at(i)+"\", \"\", \"\", 0, &"+outputVars.at(i)+");\n");
    }

    QString setPars;
    QString temp = extractTaggedSection(fmuComponentCode, "setpars");
    for(int i=0; i<parNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", parRefs.at(i));
        tempVar.replace("<<<var>>>", parVars.at(i));
        setPars.append(tempVar+"\n");
    }

    QString readVars;
    temp = extractTaggedSection(fmuComponentCode, "readvars");
    for(int i=0; i<inputNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", inputRefs.at(i));
        tempVar.replace("<<<var>>>", inputVars.at(i));
        readVars.append(tempVar+"\n");
    }

    QString writeVars;
    temp = extractTaggedSection(fmuComponentCode, "writevars");
    for(int i=0; i<outputNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", outputRefs.at(i));
        tempVar.replace("<<<var>>>", outputVars.at(i));
        writeVars.append(tempVar+"\n");
    }

    fmuComponentCode.replace("<<<headerguard>>>", headerGuard);
    fmuComponentCode.replace("<<<className>>>", className);
    fmuComponentCode.replace("<<<localvars>>>", localVars);
    fmuComponentCode.replace("<<<addconstants>>>", addConstants);
    fmuComponentCode.replace("<<<addinputs>>>", addInputs);
    fmuComponentCode.replace("<<<addoutputs>>>", addOutputs);
    fmuComponentCode.replace("<<<fmupath>>>", path);
    QDir().mkpath(targetPath+"/temp");
    fmuComponentCode.replace("<<<temppath>>>", targetPath+"/temp/");
    replaceTaggedSection(fmuComponentCode, "setpars", setPars);
    replaceTaggedSection(fmuComponentCode, "readvars", readVars);
    replaceTaggedSection(fmuComponentCode, "writevars", writeVars);

    QTextStream fmuComponentHppStream(&fmuComponentHppFile);
    fmuComponentHppStream << fmuComponentCode;
    fmuComponentHppFile.close();


    //-------------------------------------------//
    printMessage("Writing " + fmuName + ".xml...");
    //-------------------------------------------//

    //Create <fmuname>.xml
    QString cafPath = targetPath + "/" + fmuName;
    QFile fmuCafFile;
    fmuCafFile.setFileName(cafPath + "/" + fmuName + ".xml");
    if(!fmuCafFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".xml for writing.");
        removeDir(targetPath);
        return false;
    }

    //Pre-calculated things
    int nInputPorts = inputNames.size();
    int nOutputPorts = outputNames.size();
    double scale = qMax(qMax(nInputPorts, nOutputPorts)/2.0, 1.0);
    QString scaleStr = QString::number(scale);

    //Create DOM tree
    QDomDocument domDocument;

    QDomNode xmlProcessingInstruction = domDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    domDocument.appendChild(xmlProcessingInstruction);

    QDomElement cafRoot = domDocument.createElement("hopsanobjectappearance");
    cafRoot.setAttribute("version", "0.3");
    domDocument.appendChild(cafRoot);

    QDomElement modelElement = domDocument.createElement("modelobject");
    modelElement.setAttribute("typename", "FMU_"+fmuName);
    modelElement.setAttribute("displayname", "FMU_"+fmuName);
    modelElement.setAttribute("sourcecode", "component_code/"+fmuName+".hpp");
    modelElement.setAttribute("libpath", "");
    modelElement.setAttribute("recompilable", "true");
    cafRoot.appendChild(modelElement);

    QDomElement iconsElement = domDocument.createElement("icons");
    modelElement.appendChild(iconsElement);

    QDomElement iconElement = domDocument.createElement("icon");
    iconElement.setAttribute("type", "user");
    iconElement.setAttribute("path", "fmucomponent.svg");
    iconElement.setAttribute("iconrotation", "ON");
    iconElement.setAttribute("scale", scaleStr);
    iconsElement.appendChild(iconElement);

    QDomElement portsElement = domDocument.createElement("ports");
    modelElement.appendChild(portsElement);

    double inputPosStep=1.0/(nInputPorts+1.0);      //These 4 variables are used for input/output port positioning
    double outputPosStep=1.0/(nOutputPorts+1.0);
    double inputPos=0;
    double outputPos=0;

    QString numStr;
    for(int i=0; i<nInputPorts; ++i)
    {
        inputPos += inputPosStep;
        numStr.setNum(inputPos);

        QDomElement portElement = domDocument.createElement("port");
        portElement.setAttribute("name", inputNames.at(i));
        portElement.setAttribute("x", "0.0");
        portElement.setAttribute("y", numStr);
        portElement.setAttribute("a", "180");
        portsElement.appendChild(portElement);
    }
    for(int i=0; i<nOutputPorts; ++i)
    {
        outputPos += outputPosStep;
        numStr.setNum(outputPos);

        QDomElement portElement = domDocument.createElement("port");
        portElement.setAttribute("name", outputNames.at(i));
        portElement.setAttribute("x", "1.0");
        portElement.setAttribute("y", numStr);
        portElement.setAttribute("a", "0.0");
        portsElement.appendChild(portElement);
    }

    //Save to file
    QTextStream out(&fmuCafFile);
    domDocument.save(out, 2);
    fmuCafFile.close();


    rTypeName = fmuName;
    rHppPath = QFileInfo(fmuComponentHppFile).absoluteFilePath();

    return true;

//    //Instantiate FMU
//    fmi2_string_t instanceName = "Test CS model instance";
//    fmi2_string_t fmuLocation = "";
//    fmi2_boolean_t visible = fmi2_false;
//    jm_status_enu_t jmstatus = fmi2_import_instantiate(fmu, instanceName, fmi2_cosimulation, fmuLocation, visible);
//    if (jmstatus == jm_status_error)
//    {
//        printErrorMessage("fmi2_import_instantiate() failed!");
//        return;
//    }

//    //Setup experiment
//    fmi2_real_t relativeTol = 1e-4;
//    double dT = 0.001;
//    double stopT = 3;
//    double time = 0;
//    fmi2_status_t fmistatus = fmi2_import_setup_experiment(fmu, fmi2_true, relativeTol, time, fmi2_false, stopT);
//    if(fmistatus != fmi2_status_ok)
//    {
//        printErrorMessage("fmi2_import_setup_experiment() failed!");
//        return;
//    }

//    //Enter initialization mode
//    fmistatus = fmi2_import_enter_initialization_mode(fmu);
//    if(fmistatus != fmi2_status_ok)
//    {
//        printErrorMessage("fmi2_import_enter_initialization_mode() failed!");
//        return;
//    }

//    fmistatus = fmi2_import_exit_initialization_mode(fmu);
//    if(fmistatus != fmi2_status_ok)
//    {
//        printErrorMessage("fmi2_import_exit_initialization_mode() failed!");
//        return;
//    }

//    QVector<double> tVector, hVector;
//    while(time <= stopT)
//    {
//        fmi2_import_do_step(fmu, time, dT, true);

//        double rValue;
//        fmi2_value_reference_t vr = 0;

//        tVector.append(time);
//        fmistatus = fmi2_import_get_real(fmu, &vr, 1, &rValue);
//        hVector.append(rValue);

//        time += dT;
//    }

//    QFile testFile("c:/users/robbr48/Documents/Slask/height.csv");
//    testFile.open(QFile::Text | QFile::WriteOnly);
//    for(int i=0; i<tVector.size(); ++i)
//    {
//        testFile.write(QString::number(tVector.at(i)).toLocal8Bit() + ", " + QString::number(hVector.at(i)).toLocal8Bit()+"\n");
//    }
//    testFile.close();

//    fmistatus = fmi2_import_terminate(fmu);

//    fmi2_import_free_instance(fmu);

//    fmi2_import_destroy_dllfmu(fmu);

//    fmi2_import_free(fmu);
//    fmi_import_free_context(context);

//    printMessage("Everything seems to be OK since you got this far=)!");

}



void HopsanFMIGenerator::generateFromFmu(QString path, QString targetPath)
{
    printMessage("Initializing FMU import");

    QFileInfo fmuFileInfo = QFileInfo(path);
    fmuFileInfo.setFile(path);

    QDir zipDir;
    zipDir = QDir::cleanPath(mBinPath + "../ThirdParty/7z");

    QDir gccDir;
    gccDir = QDir::cleanPath(mBinPath + "../ThirdParty/mingw32/bin");

    QString fmuName = fmuFileInfo.fileName();
    fmuName.chop(4);

    QString cqsType="Signal";

    //Create import directory if it does not exist
    if(!QDir(targetPath).exists())
        QDir().mkpath(targetPath);

    //Remove output directory if it already exists
    if(QDir(targetPath+"/"+fmuName).exists() && !removeDir(targetPath+"/"+fmuName))
    {
        printErrorMessage("Unable to remove output directory: "+QDir().cleanPath(targetPath+"/"+fmuName)+". Please remove it manually and try again.");
        return;
    }

    //Create output directory
    if(!QDir().mkdir(targetPath+"/" + fmuName))
    {
        printErrorMessage("Unable to create output directory: "+QDir().cleanPath(targetPath+"/"+fmuName)+". Please remove it manually and try again.");
        return;
    }

    QString fmuPath = targetPath+"/" + fmuName;
    QDir fmuDir = QDir::cleanPath(fmuPath);

    printMessage("Unpacking files");

#ifdef WIN32
    QStringList arguments;
    arguments << "x" << fmuFileInfo.filePath() << "-o" + fmuDir.path() << "-aoa";
    callProcess(zipDir.path()+"/7z.exe", arguments, zipDir.path());
#else
    QStringList arguments = QStringList() << fmuFileInfo.filePath() << "-d" << fmuDir.path();
    callProcess("unzip", arguments, fmuDir.path());
#endif

    //Move all binary files to FMU directory
#ifdef WIN32
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/win32");
#elif WIN64
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/win64");
#elif linux && __i386__
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/linux32");
#elif linux && __x86_64__
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/linux64");
#elif __APPLE__
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/macx64");
#endif
    if(!binaryDir.exists())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: Unable to unpack files");
        return;
    }
    QFileInfoList binaryFiles = binaryDir.entryInfoList(QDir::Files);
    for(int i=0; i<binaryFiles.size(); ++i)
    {
        QFile tempFile;
        tempFile.setFileName(binaryFiles.at(i).filePath());
        tempFile.copy(fmuDir.path() + "/" + binaryFiles.at(i).fileName());
        printMessage("Copying " + tempFile.fileName() + " to " + fmuDir.path() + "/" + binaryFiles.at(i).fileName());
        tempFile.remove();
    }


    //Move all resource files to FMU directory
    QDir resDir = QDir::cleanPath(fmuDir.path() + "/resources");
    QFileInfoList resFiles = resDir.entryInfoList(QDir::Files);
    for(int i=0; i<resFiles.size(); ++i)
    {
        QFile tempFile;
        tempFile.setFileName(resFiles.at(i).filePath());
        tempFile.copy(fmuDir.path() + "/" + resFiles.at(i).fileName());
        printMessage("Copying " + tempFile.fileName() + " to " + fmuDir.path() + "/" + resFiles.at(i).fileName());
        tempFile.remove();
    }

    //Move hmf files to execution path (ugly hack for importing hopsan fmu to hopsan)
    QStringList filters;
    filters << "*.hmf";
    fmuDir.setNameFilters(filters);
    QStringList hmfList = fmuDir.entryList();
    for (int i = 0; i < hmfList.size(); ++i)
    {
        QFile hmfFile;
        hmfFile.setFileName(fmuDir.path() + "/" + hmfList.at(i));
        if(hmfFile.exists())
        {
            QFile().remove(mBinPath+hmfList.at(i));
            if(QFile().exists(mBinPath+hmfList.at(i)))
            {
                printErrorMessage("Unable to copy "+hmfFile.fileName()+" to /bin directory: File already exists.");
                return;
            }
            hmfFile.copy(mBinPath + hmfList.at(i));
            hmfFile.remove();
            hmfFile.setFileName(mBinPath + hmfList.at(i));
            printMessage("Copying " + hmfFile.fileName() + " to " + mBinPath + hmfList.at(i)+"...");
        }
    }
    fmuDir.setFilter(QDir::NoFilter);

    printMessage("Parsing modelDescription.xml...");

    //Load XML data from modelDescription.xml
    //Copy xml-file to this directory
    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(fmuDir.path() + "/modelDescription.xml");
    if(!binaryDir.exists())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: modelDescription.xml not found.");
        return;
    }
    QDomDocument fmuDomDocument;
    QDomElement fmuRoot = loadXMLDomDocument(modelDescriptionFile, fmuDomDocument, "fmiModelDescription");
    modelDescriptionFile.close();

    if(fmuRoot == QDomElement())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: Could not parse modelDescription.xml.");
        return;
    }

    printMessage("Defining variables...");


    bool isModelExchangeFMU = fmuRoot.firstChildElement("Implementation").isNull();

    //Define lists with input and output variables

    QStringList inVarValueRefs, inVarPortNames;
    QStringList outVarValueRefs, outVarPortNames;
    QStringList inoutVarValueRefs, inoutVarPortNames;
    QDomElement variablesElement = fmuRoot.firstChildElement("ModelVariables");
    QDomElement varElement = variablesElement.firstChildElement("ScalarVariable");
    int i=0;
    //! @todo We cannot use value reference as port names, because several ports can point to the same value
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") != "parameter" && varElement.attribute("variability") != "constant")
        {
            /*if(!varElement.hasAttribute("causality"))
            {
                //inoutVarValueRefs << varElement.attribute("valueReference");
                inoutVarValueRefs << QString::number(i);
                inoutVarPortNames << varElement.attribute("name");
            }
            else */if(varElement.attribute("causality") == "input")
            {
                //inVarValueRefs << varElement.attribute("valueReference");
                inVarValueRefs << QString::number(i);
                inVarPortNames << varElement.attribute("name");
            }
            else if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "output")
            {
                //outVarValueRefs << varElement.attribute("valueReference");
                outVarValueRefs << QString::number(i);
                outVarPortNames << varElement.attribute("name");
            }
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
        ++i;
    }

    i=0;

    QStringList tlmPortTypes;
    QList<QStringList> tlmPortVarNames;
    QList<QStringList> tlmPortValueRefs;
    readTLMSpecsFromFile(fmuPath+"/"+fmuName+"_TLM.xml", tlmPortTypes, tlmPortVarNames, tlmPortValueRefs,
                         inVarValueRefs, inVarPortNames, outVarValueRefs, outVarPortNames, cqsType);
    if(cqsType=="")
    {
        printErrorMessage("CQS-type undefined. Mixing C and Q ports in the same component is not allowed.");
        return;
    }

    ////////////////////////////////
    // DEFINE PORT SPECIFICATIONS //
    ////////////////////////////////


    QList<FMIPortSpecification> portSpecs;
    for(int i=0; i<inVarPortNames.size(); ++i)
    {
        QString varName = toVarName("mpIn_"+inVarPortNames[i]);
        QString portName = toVarName(inVarPortNames[i]+"In");
        QString mpndName = toVarName("mpND_in"+inVarPortNames[i]);
        portSpecs << FMIPortSpecification(varName, portName, mpndName, inVarValueRefs[i], "ReadPort", "NodeSignal", "NodeSignal::Value", "input");
    }
    for(int i=0; i<outVarPortNames.size(); ++i)
    {
        portSpecs << FMIPortSpecification(toVarName("mpOut"+outVarPortNames[i]), toVarName(outVarPortNames[i]+"Out"),
                                          toVarName("mpND_out"+outVarPortNames[i]), outVarValueRefs[i],
                                          "WritePort", "NodeSignal", "NodeSignal::Value", "output");
    }
    for(int i=0; i<inoutVarPortNames.size(); ++i)
    {
        portSpecs << FMIPortSpecification(toVarName("mpIn"+inoutVarPortNames[i]), toVarName(inoutVarPortNames[i]+"In"),
                                          toVarName("mpND_in"+inoutVarPortNames[i]), inoutVarValueRefs[i],
                                          "ReadPort", "NodeSignal", "NodeSignal::Value", "");
        portSpecs << FMIPortSpecification(toVarName("mpOut"+inoutVarPortNames[i]), toVarName(inoutVarPortNames[i]+"Out"),
                                          toVarName("mpND_out"+inoutVarPortNames[i]), inoutVarValueRefs[i],
                                          "WritePort", "NodeSignal", "NodeSignal::Value", "");
    }
    for(int i=0; i<tlmPortVarNames.size(); ++i)
    {
        QString numStr = QString::number(i);
        QString varName = "mpP"+numStr;
        QString portName = "P"+numStr;
        QString portType = "PowerPort";
        QString nodeType;
        QString cqType;
        QStringList mpndNames, dataTypes, causalities;

        QStringList nodeTypes;
        GeneratorNodeInfo::getNodeTypes(nodeTypes);
        Q_FOREACH(const QString &type, nodeTypes)
        {
            if(tlmPortTypes[i] == GeneratorNodeInfo(type).niceName+"q")
            {
                nodeType = type;
                cqType = "q";
            }
            else if(tlmPortTypes[i] == GeneratorNodeInfo(type).niceName+"c")
            {
                nodeType = type;
                cqType = "c";
            }
        }

        GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);
        if(cqType == "q")
        {
            int j=0;
            for(; j<info.qVariables.size(); ++j)
            {
                mpndNames << "mpND_"+info.qVariables[j]+QString::number(i);
                dataTypes << nodeType+"::"+info.variableLabels[j];
                causalities << "output";
            }
            for(int k=0; k<info.cVariables.size(); ++k)
            {
                mpndNames << "mpND_"+info.cVariables[k]+QString::number(i);
                dataTypes << nodeType+"::"+info.variableLabels[j+k];
                causalities << "input";
            }
        }
        else if(cqType == "c")
        {
            int j=0;
            for(; j<info.qVariables.size(); ++j)
            {
                mpndNames << "mpND_"+info.qVariables[j]+QString::number(i);
                dataTypes << nodeType+"::"+info.variableLabels[j];
                causalities << "input";
            }
            for(int k=0; k<info.cVariables.size(); ++k)
            {
                mpndNames << "mpND_"+info.cVariables[k]+QString::number(i);
                dataTypes << nodeType+"::"+info.variableLabels[j+k];
                causalities << "output";
            }
        }

        for(int j=0; j<mpndNames.size(); ++j)
        {
            portSpecs << FMIPortSpecification(varName, portName, mpndNames[j], tlmPortValueRefs[i][j], portType, nodeType, dataTypes[j], causalities[j]);
        }
    }

    //Print port info to user
    for(int p=0; p<portSpecs.size(); ++p)
        printMessage(QString("\nPort:")+
                     "  \n  Variable        = \""+portSpecs[p].varName +
                     "\"\n  Name            = \""+portSpecs[p].portName +
                     "\"\n  Node pointer    = \""+portSpecs[p].mpndName+
                     "\"\n  Value reference = \""+portSpecs[p].valueRef+
                     "\"\n  Port type       = \""+portSpecs[p].valueRef+
                     "\"\n  Node type       = \""+portSpecs[p].nodeType+
                     "\"\n  Data type       = \""+portSpecs[p].dataType+
                     "\"\n  Causality       = \""+portSpecs[p].causality+"\"\n");


    /////////////////////////////////////
    // DEFINE PARAMETER SPECIFICATIONS //
    /////////////////////////////////////


    QList<FMIParameterSpecification> parSpecs;
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    int j=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            QString varName = toVarName("par"+QString::number(i));
            QString initValue = varElement.firstChildElement("Real").attribute("start", "0");
            QString parName = toVarName(varElement.attribute("name"));
            QString description = varElement.attribute("description");
            //QString valueRef = varElement.attribute("valueReference");
            QString valueRef = QString::number(j);
            parSpecs << FMIParameterSpecification(varName, parName, description, initValue, valueRef);
            ++i;

        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
        ++j;
    }

    //Print parameter info to user
    for(int p=0; p<parSpecs.size(); ++p)
        printMessage(QString("\nParameter:")+
                     "  \n  Variable        = \""+parSpecs[p].varName +
                     "\"\n  Name            = \""+parSpecs[p].parName +
                     "\"\n  Description     = \""+parSpecs[p].description+
                     "\"\n  Initial value   = \""+parSpecs[p].initValue+
                     "\"\n  Value reference = \""+parSpecs[p].valueRef+"\"\n");


    ///////////////////////////////////
    // GENERATE FILES FROM TEMPLATES //
    ///////////////////////////////////


    printMessage("Creating fmuLib.cc...");

    //Create fmuLib.cc
    QFile fmuLibFile;
    fmuLibFile.setFileName(fmuDir.path() + "/fmuLib.cc");
    if(!fmuLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open fmuLib.cc for writing.");
        removeDir(fmuDir.path());
        return;
    }

    printMessage("Writing fmuLib.cc...");

    QFile fmuLibTemplateFile(":templates/fmuLibTemplate.cc");
    assert(fmuLibTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuLibCode;
    QTextStream t(&fmuLibTemplateFile);
    fmuLibCode = t.readAll();
    fmuLibTemplateFile.close();
    assert(!fmuLibCode.isEmpty());

    fmuLibCode.replace("<<<0>>>", fmuName);
    fmuLibCode.replace("<<<1>>>", mCoreIncludePath);

    QTextStream fmuLibStream(&fmuLibFile);
    fmuLibStream << fmuLibCode;
    fmuLibFile.close();


    printMessage("Creating " + fmuName + ".hpp...");

    //Create <fmuname>.hpp
    QDir().mkdir(fmuDir.path() + "/component_code");
    QFile fmuComponentHppFile;
    fmuComponentHppFile.setFileName(fmuDir.path() + "/component_code/" + fmuName + ".hpp");
    if(!fmuComponentHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".hpp for writing.");
        removeDir(fmuDir.path());
        return;
    }

    printMessage("Writing " + fmuName + ".hpp...");

    QFile fmuComponentTemplateFile;
    if(isModelExchangeFMU)
    {
        fmuComponentTemplateFile.setFileName(":templates/fmuComponentTemplate.hpp");
    }
    else
    {
        fmuComponentTemplateFile.setFileName(":templates/fmuCoSimComponentTemplate.hpp");
    }
    assert(fmuComponentTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuComponentCode;
    QTextStream t2(&fmuComponentTemplateFile);
    fmuComponentCode = t2.readAll();
    fmuComponentTemplateFile.close();
    if(fmuComponentCode.isEmpty())
    {
        printErrorMessage("Unable to generate code for "+fmuName+".hpp.");
        return;
    }

    //Declare ports
    QString fmuComponentReplace2;
    QString portLine = extractTaggedSection(fmuComponentCode, "portdecl");
    QStringList addedPorts;
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(!addedPorts.contains(portSpecs[p].varName) && portSpecs[p].nodeType != "NodeSignal")
        {
            fmuComponentReplace2.append(replaceTag(portLine, "varname", portSpecs[p].varName));
            addedPorts << portSpecs[p].varName;
        }
    }

    //Declare node data pointers
    QString fmuComponentReplace3;
    QString mpndLine = extractTaggedSection(fmuComponentCode, "dataptrdecl");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        fmuComponentReplace3.append(replaceTag(mpndLine, "mpndname", portSpecs[p].mpndName));
    }

    //Declare parameters
    QString fmuComponentReplace4;
    QString parLine = extractTaggedSection(fmuComponentCode, "pardecl");
    for(int p=0; p<parSpecs.size(); ++p)
    {
        fmuComponentReplace4.append(replaceTag(parLine, "varname", parSpecs[p].varName));
    }

    //Add ports
    QString fmuComponentReplace7;
    QString addPortLine = extractTaggedSection(fmuComponentCode, "addports");
    addedPorts.clear();
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(!addedPorts.contains(portSpecs[p].varName) && portSpecs[p].portType == "PowerPort")
        {
            fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "varname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                    QStringList() << portSpecs[p].varName << portSpecs[p].portName << portSpecs[p].portType << portSpecs[p].nodeType << "Port::NotRequired"));
            addedPorts << portSpecs[p].varName;
        }
    }

    //Add variables
    QString fmuComponentReplaceAddVars;
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(portSpecs[p].portType == "ReadPort")
        {
            fmuComponentReplaceAddVars.append("            addInputVariable(\""+portSpecs[p].portName+"\",\"\",\"\",0,&"+portSpecs[p].mpndName+");\n");
        }
        if(portSpecs[p].portType == "WritePort")
        {
            fmuComponentReplaceAddVars.append("            addOutputVariable(\""+portSpecs[p].portName+"\",\"\",\"\",0,&"+portSpecs[p].mpndName+");\n");
        }
    }


    //Initialize and register parameters
    QString fmuComponentReplace8;
    QString regParLine = extractTaggedSection(fmuComponentCode, "addconstants");
    for(int p=0; p<parSpecs.size(); ++p)
    {
        fmuComponentReplace8.append(replaceTags(regParLine, QStringList() << "varname" << "initvalue" << "parname" << "description",
                                                QStringList() << parSpecs[p].varName << parSpecs[p].initValue << parSpecs[p].parName << parSpecs[p].description));
    }

    //Initialize node data pointers
    QString fmuComponentReplace9;
    QString getNodePtrLine = extractTaggedSection(fmuComponentCode, "initdataptrs");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(portSpecs[p].portType == "PowerPort")
        {
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "mpndname" << "varname" << "datatype", QStringList() << portSpecs[p].mpndName << portSpecs[p].varName << portSpecs[p].dataType));
        }
    }


    //Set parameters
    QString fmuComponentReplace10;
    QString writeParLines = extractTaggedSection(fmuComponentCode, "setpars");
    for(int p=0; p<parSpecs.size(); ++p)
    {
        fmuComponentReplace10.append(replaceTags(writeParLines, QStringList() << "valueref" << "varname", QStringList() << parSpecs[p].valueRef << parSpecs[p].varName));
    }

    //Write signal input values
    QString fmuComponentReplace14;
    QString writeSignalVarLines = extractTaggedSection(fmuComponentCode, "readsignalinputs");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(portSpecs[p].causality != "output" && portSpecs[p].nodeType == "NodeSignal")
            fmuComponentReplace14.append(replaceTags(writeSignalVarLines, QStringList() << "valueref" << "mpndname", QStringList() << portSpecs[p].valueRef << portSpecs[p].mpndName));
    }


    //Write input values
    QString fmuComponentReplace11;
    QString writeVarLines = extractTaggedSection(fmuComponentCode, "readinputs");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(portSpecs[p].causality != "output" && portSpecs[p].nodeType != "NodeSignal")
            fmuComponentReplace11.append(replaceTags(writeVarLines, QStringList() << "varname" << "valueref" << "mpndname", QStringList() << portSpecs[p].varName << portSpecs[p].valueRef << portSpecs[p].mpndName));
    }

    //Write back output values
    QString fmuComponentReplace12;
    QString readVarLines = extractTaggedSection(fmuComponentCode, "writeoutputs");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(portSpecs[p].causality != "input")
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << portSpecs[p].valueRef << portSpecs[p].mpndName));
    }

    QString fmuComponentReplace13;
#ifdef WIN32
    fmuComponentReplace13 = "dll";
#elif linux
    fmuComponentReplace13 = "so";
#endif

    fmuComponentCode.replace("<<<cqstype>>>", cqsType);
    fmuComponentCode.replace("<<<modelname>>>", fmuName);
    fmuComponentCode.replace("<<<includepath>>>", mCoreIncludePath);
    replaceTaggedSection(fmuComponentCode, "portdecl", fmuComponentReplace2);
    replaceTaggedSection(fmuComponentCode, "dataptrdecl", fmuComponentReplace3);
    replaceTaggedSection(fmuComponentCode, "pardecl", fmuComponentReplace4);
    fmuComponentCode.replace("<<<fmudir>>>", fmuDir.path());
    replaceTaggedSection(fmuComponentCode, "addports", fmuComponentReplace7);
    fmuComponentCode.replace("<<<addvars>>>", fmuComponentReplaceAddVars);
    replaceTaggedSection(fmuComponentCode, "addconstants", fmuComponentReplace8);
    replaceTaggedSection(fmuComponentCode, "initdataptrs", fmuComponentReplace9);
    replaceTaggedSection(fmuComponentCode, "setpars", fmuComponentReplace10);
    replaceTaggedSection(fmuComponentCode, "readinputs", fmuComponentReplace11);
    replaceTaggedSection(fmuComponentCode, "writeoutputs", fmuComponentReplace12);
    replaceTaggedSection(fmuComponentCode,  "readsignalinputs", fmuComponentReplace14);
    fmuComponentCode.replace("<<<fileext>>>", fmuComponentReplace13);

    QTextStream fmuComponentHppStream(&fmuComponentHppFile);
    fmuComponentHppStream << fmuComponentCode;
    fmuComponentHppFile.close();


    QString iconName = "fmucomponent.svg";
    QImage *pIconImage = new QImage(fmuPath+"/model.png");
    if(!pIconImage->isNull())
    {
        iconName = "model.svg";
    }


    printMessage("Writing "+fmuName+".xml...");

    //Create <fmuname>.xml
    //! @todo Use dom elements for generating xml (this is just stupid)
    QFile fmuXmlFile;
    fmuXmlFile.setFileName(fmuDir.path() + "/" + fmuName + ".xml");
    if(!fmuXmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".xml for writing.");
        removeDir(fmuDir.path());
        return;
    }

    int nInputPorts = inVarValueRefs.size()+inoutVarValueRefs.size();
    int nOutputPorts = outVarValueRefs.size()+inoutVarValueRefs.size();
    double scale = qMax(qMax(nInputPorts, nOutputPorts)/2.0, 1.0);
    QString scaleStr = QString::number(scale);

    QTextStream fmuXmlStream(&fmuXmlFile);
    fmuXmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    fmuXmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
    fmuXmlStream << "    <modelobject typename=\""+fmuName+"\" displayname=\""+fmuName+"\" sourcecode=\"component_code"+fmuName+".hpp\" libpath=\"\" recompilable=\"true\">\n";
    fmuXmlStream << "        <icons>\n";
    fmuXmlStream << "            <icon type=\"user\" path=\"fmucomponent.svg\" iconrotation=\"ON\" scale=\""+scaleStr+"\"/>\n";
    fmuXmlStream << "        </icons>\n";
    fmuXmlStream << "        <ports>\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;

    double tlmPosStep=1.0/(tlmPortTypes.size()+1.0);      //These 2 variables are used for TLM port positioning
    double tlmPos=0;

    double inputPosStep=1.0/(nInputPorts+1.0);      //These 4 variables are used for input/output port positioning
    double outputPosStep=1.0/(nOutputPorts+1.0);
    double inputPos=0;
    double outputPos=0;

    QString numStr, numStr2;
    for(int i=0; i<tlmPortTypes.size(); ++i)
    {
        tlmPos += tlmPosStep;
        numStr = QString::number(i);
        numStr2.setNum(tlmPos);
        fmuXmlStream << "            <port name=\"P"+numStr+"\" x=\""+numStr2+"\" y=\"0.0\"  a=\"270\"/>\n";
    }
    for(int i=0; i<inoutVarValueRefs.size(); ++i)
    {
        inputPos += inputPosStep;
        numStr2.setNum(inputPos);
        fmuXmlStream << "            <port name=\""+toVarName(inoutVarPortNames[i])+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
        outputPos += outputPosStep;
        numStr2.setNum(outputPos);
        fmuXmlStream << "            <port name=\""+toVarName(inoutVarPortNames[i])+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
    }
    for(int i=0; i<inVarValueRefs.size(); ++i)
    {
        inputPos += inputPosStep;
        numStr2.setNum(inputPos);
        fmuXmlStream << "            <port name=\""+toVarName(inVarPortNames[i])+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
    }
    for(int i=0; i<outVarValueRefs.size(); ++i)
    {
        outputPos += outputPosStep;
        numStr2.setNum(outputPos);
        fmuXmlStream << "            <port name=\""+toVarName(outVarPortNames[i])+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
    }
    fmuXmlStream << "        </ports>\n";
    fmuXmlStream << "    </modelobject>\n";
    fmuXmlStream << "</hopsanobjectappearance>\n";
    fmuXmlFile.close();

    //Move FMI source files to compile directory

#ifdef WIN32
    QString fmiSrcPath = mBinPath + "../ThirdParty/fmi/";
#elif linux
    QString fmiSrcPath = mBinPath + "../ThirdParty/fmi/linux/";
#elif __APPLE__
    QString fmiSrcPath = mBinPath + "../ThirdParty/fmi/macx/";
#endif

    if(!copyFile(fmiSrcPath+"sim_support.c", fmuDir.path()+"/sim_support.c")) return;
    if(!copyFile(fmiSrcPath+"stack.cc", fmuDir.path()+"/stack.cc")) return;
    if(!copyFile(fmiSrcPath+"xml_parser.h", fmuDir.path()+"/xml_parser.h")) return;
    if(!copyFile(fmiSrcPath+"sim_support.h", fmuDir.path()+"/sim_support.h")) return;
    if(!copyFile(fmiSrcPath+"stack.h", fmuDir.path()+"/stack.h")) return;
    if(!copyFile(fmiSrcPath+"xml_parser.cc", fmuDir.path()+"/xml_parser.cc")) return;
    if(!copyFile(fmiSrcPath+"expat.h", fmuDir.path()+"/expat.h")) return;
    if(!copyFile(fmiSrcPath+"expat_external.h", fmuDir.path()+"/expat_external.h")) return;
    if(!copyFile(fmiSrcPath+"fmi_me.h", fmuDir.path()+"/fmi_me.h")) return;
    if(!copyFile(fmiSrcPath+"fmiModelFunctions.h", fmuDir.path()+"/fmiModelFunctions.h")) return;
    if(!copyFile(fmiSrcPath+"fmiModelTypes.h", fmuDir.path()+"/fmiModelTypes.h")) return;
    if(!copyFile(fmiSrcPath+"fmi_cs.h", fmuDir.path()+"/fmi_cs.h")) return;
    if(!copyFile(fmiSrcPath+"fmiFunctions.h", fmuDir.path()+"/fmiFunctions.h")) return;
    if(!copyFile(fmiSrcPath+"fmiPlatformTypes.h", fmuDir.path()+"/fmiPlatformTypes.h")) return;

#ifdef WIN32
    if(!copyFile(fmiSrcPath+"libexpat.a", fmuDir.path()+"/libexpat.a")) return;
    if(!copyFile(fmiSrcPath+"libexpat.dll", fmuDir.path()+"/libexpat.dll")) return;
    if(!copyFile(fmiSrcPath+"libexpatw.a", fmuDir.path()+"/libexpatw.a")) return;
    if(!copyFile(fmiSrcPath+"libexpatw.dll", fmuDir.path()+"/libexpatw.dll")) return;
#elif linux
    if(!copyFile(fmiSrcPath+"libexpatMT.lib", fmuDir.path()+"/libexpatMT.lib")) return;
#elif __APPLE__
    if(!copyFile(fmiSrcPath+"libexpatMT.a", fmuDir.path()+"/libexpatMT.a")) return;
#endif

    printMessage("Writing " + fmuName + "_lib.xml...");

    QFile xmlFile;
    xmlFile.setFileName(fmuDir.path()+"/"+fmuName+"_lib.xml");
    if(!xmlFile.exists())
    {
        if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            printErrorMessage("Failed to open " + fmuName + "_lib.xml  for writing.");
            return;
        }
        QTextStream xmlStream(&xmlFile);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        xmlStream << "<hopsancomponentlibrary xmlversion=\"0.1\" libversion=\"1\" name=\""+fmuName+"\">\n";
        xmlStream << "  <lib>fmuLib</lib>\n";
        xmlStream << "  <source>fmuLib.cc</source>\n";
        xmlStream << "  <source>stack.cc</source>\n";
        xmlStream << "  <source>xml_parser.cc</source>\n";
        xmlStream << "</hopsancomponentlibrary>\n";
        xmlFile.close();
    }

#ifdef WIN32
    if(!compileComponentLibrary(QFileInfo(xmlFile).absoluteFilePath(), this, "-L./ -llibexpat"))
#else
    if(!compileComponentLibrary(QFileInfo(xmlFile).absoluteFilePath(), this))
#endif
    {
        printErrorMessage("Failed to import fmu.");
        return;
    }
    else
    {
        cleanUp(fmuPath, QStringList() << "sim_support.h" << "sim_support.c" << "stack.h" << "xml_parser.h" << "xml_parser.cc" << "expat.h" <<
                "expat_external.h" << "fmi_me.h" << "fmiModelFunctions.h" << "fmiModelTypes.h" << "compile.bat" << "fmuLib.cc",
                QStringList() << "component_code" << "binaries");

        printMessage("Finished.\n");
    }
}


//! @brief Generates an FMU for specified component system
//! @param savePath Path where to export FMU
//! @param me Boolean for using model exchange
//! @param pSystme Pointer to system to export
void HopsanFMIGenerator::generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem, bool me)
{
    printMessage("Initializing FMU export...");

    if(pSystem == 0)
    {
        printErrorMessage("System pointer is null. Aborting.");
        return;
    }

    QDir saveDir;
    saveDir.setPath(savePath);

    //Copy HopsanCore files to export directory
    if(!this->copyIncludeFilesToDir(savePath))
        return;
    if(!this->copySourceFilesToDir(savePath))
        return;
    if(!this->copyDefaultComponentCodeToDir(savePath))
        return;

    //Write the FMU ID
    int random = rand() % 1000000000;
    QString randomString = QString::number(random);
    QString ID = QUuid::createUuid().toString();

    //Collect information about ports
    QStringList inputVariables;
    QStringList inputComponents;
    QStringList inputPorts;
    QList<int> inputDatatypes;
    QStringList outputVariables;
    QStringList outputComponents;
    QStringList outputPorts;
    QList<int> outputDatatypes;
    QList<QStringList> tlmPorts;

    std::vector<HString> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        getInterfaceInfo(pSystem->getSubComponent(names[i])->getTypeName().c_str(), names[i].c_str(),
                         inputVariables, inputComponents, inputPorts, inputDatatypes,
                         outputVariables, outputComponents, outputPorts, outputDatatypes, tlmPorts);
    }


    //Collect information about system parameters
    QStringList parameterNames;
    QStringList parameterValues;
    std::vector<HString> parameterNamesStd;
    pSystem->getParameterNames(parameterNamesStd);
    for(size_t p=0; p<parameterNamesStd.size(); ++p)
    {
        parameterNames.append(QString(parameterNamesStd[p].c_str()));
    }
    for(int p=0; p<parameterNames.size(); ++p)
    {
        HString value;
        pSystem->getParameterValue(parameterNamesStd[p], value);
        parameterValues.append(QString(value.c_str()));
    }


    //Create file objects for all files that shall be created
    QFile modelSourceFile;
    QString modelName = pSystem->getName().c_str();
    // modelName.chop(4);
    QString realModelName = modelName;          //Actual model name (used for hmf file)
    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open " + modelName + ".c for writing.");
        return;
    }

    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QFile tlmDescriptionFile;
    tlmDescriptionFile.setFileName(savePath + "/"+modelName+"_TLM.xml");
    if(!tlmDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+modelName+"_TLM.xml for writing.");
        return;
    }

    QFile fmuHeaderFile;
    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QFile fmuSourceFile;
    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

#ifdef WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(savePath + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
#endif

    QFile modelHppFile;
    modelHppFile.setFileName(savePath + "/model.hpp");
    if(!modelHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open model.hpp for writing.");
        return;
    }

    printMessage("Writing modelDescription.xml...");

    QFile xmlTemplatefile(":templates/fmuModelDescriptionTemplate.xml");
    if(!xmlTemplatefile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QString xmlCode;
    QTextStream t(&xmlTemplatefile);
    xmlCode = t.readAll();
    xmlTemplatefile.close();
    if(xmlCode.isEmpty())
    {
        printErrorMessage("Failed to generate XML code for modelDescription.xml.");
        return;
    }

    QString xmlReplace3;
    QString scalarVarLines = extractTaggedSection(xmlCode, "variables");
    int i, j;
    for(i=0; i<inputVariables.size(); ++i)
    {
        QString refString = QString::number(i);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << inputVariables.at(i) << refString << "input"));
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QString refString = QString::number(i+j);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << outputVariables.at(j) << refString << "output"));
    }

    QString xmlReplace4;
    QString paramLines = extractTaggedSection(xmlCode, "parameters");
    for(int k=0; k<parameterNames.size(); ++k)
    {
        QString refString = QString::number(i+j+k);
        xmlReplace4.append(replaceTags(paramLines, QStringList() << "varname" << "varref" << "variability" << "start", QStringList() << parameterNames[k] << refString << "parameter" << parameterValues[k]));
    }

    xmlCode.replace("<<<modelname>>>", modelName);
    xmlCode.replace("<<<guid>>>", ID);
    replaceTaggedSection(xmlCode, "variables", xmlReplace3);
    replaceTaggedSection(xmlCode, "parameters", xmlReplace4);

    if(me)
    {
        xmlCode.replace("<<<implementation>>>","");
    }
    else
    {
        xmlCode.replace("<<<implementation>>>","<Implementation>\n  <CoSimulation_StandAlone>\n    <Capabilities\n      canHandleVariableCommunicationStepSize=\"false\"\n      canHandleEvents=\"false\"/>\n  </CoSimulation_StandAlone>\n</Implementation>");
    }

    if(me)
    {
        xmlCode.replace("<<<nstates>>>",QString::number(inputVariables.size()+outputVariables.size()));
    }
    else
    {
        xmlCode.replace("<<<nstates>>>","0");
    }


    QTextStream modelDescriptionStream(&modelDescriptionFile);
    modelDescriptionStream << xmlCode;
    modelDescriptionFile.close();


    if(!tlmPorts.isEmpty())
    {
        printMessage("Writing "+modelName+"_TLM.xml...");

        QString tlmCode;
        tlmCode.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        tlmCode.append("<fmutlm>\n");
        for(int i=0; i<tlmPorts.size(); ++i)
        {
            tlmCode.append("  <tlmport type=\""+tlmPorts[i].first()+"\">\n");
            for(int j=1; j<tlmPorts[i].size(); j+=2)
            {
                tlmCode.append("    <"+tlmPorts[i][j]+">"+tlmPorts[i][j+1]+"</"+tlmPorts[i][j]+">\n");
            }
            tlmCode.append("  </tlmport>\n");
        }
        tlmCode.append("</fmutlm>\n");

        QTextStream tlmDescriptionStream(&tlmDescriptionFile);
        tlmDescriptionStream << tlmCode;
        tlmDescriptionFile.close();
    }


    printMessage("Writing " + modelName + ".c...");

    QString sourceTemplateFileName;
    if(me)
    {
        sourceTemplateFileName = ":templates/fmuModelSourceTemplate.c";
    }
    else
    {
        sourceTemplateFileName = ":templates/fmuCoSimSourceTemplate.c";
    }

    QFile sourceTemplateFile(sourceTemplateFileName);
    if(!sourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+modelName+".c for writing.");
        return;
    }
    QString modelSourceCode;
    QTextStream t2(&sourceTemplateFile);
    modelSourceCode = t2.readAll();
    sourceTemplateFile.close();
    if(modelSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for "+modelName+".c.");
        return;
    }

    QString sourceReplace4;
    QString varDefLine = extractTaggedSection(modelSourceCode, "4");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << inputVariables.at(i) << QString::number(i)));
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << outputVariables.at(j) << QString::number(i+j)));
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << parameterNames.at(k) << QString::number(i+j+k)));

    QString sourceReplace5;
    i=0;
    j=0;
    if(!inputVariables.isEmpty())
    {
        sourceReplace5.append(inputVariables.at(0)+"_");
        ++i;
    }
    else if(!outputVariables.isEmpty())
    {
        sourceReplace5.append(outputVariables.at(0)+"_");
        ++j;
    }
    for(; i<inputVariables.size(); ++i)
        sourceReplace5.append(", "+inputVariables.at(i)+"_");
    for(; j<outputVariables.size(); ++j)
        sourceReplace5.append(", "+outputVariables.at(j)+"_");

    QString sourceReplace6;
    QString startValueLine = extractTaggedSection(modelSourceCode, "6");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace6.append(replaceTag(startValueLine, "varname", inputVariables.at(i)));         //!< Fix start value handling
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace6.append(replaceTag(startValueLine, "varname", outputVariables.at(j)));        //!< Fix start value handling

    QString sourceReplace8;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace8.append("           case "+inputVariables.at(i)+"_: return getVariable(vr, "+QString::number(inputDatatypes.at(i))+");\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace8.append("           case "+outputVariables.at(j)+"_: return getVariable(vr, "+QString::number(outputDatatypes.at(j))+");\n");

    QString sourceReplace9;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace9.append("           case "+inputVariables.at(i)+"_: setVariable(vr, "+QString::number(inputDatatypes.at(i))+", value); break;\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace9.append("           case "+outputVariables.at(j)+"_: setVariable(vr, "+QString::number(outputDatatypes.at(j))+", value); break;\n");
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace9.append("           case "+parameterNames.at(k)+"_: setParameter(\""+parameterNames.at(k)+"\", value); break;\n");

    modelSourceCode.replace("<<<0>>>", modelName);
    modelSourceCode.replace("<<<1>>>", ID);
    modelSourceCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));
    modelSourceCode.replace("<<<3>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));  //!< @todo Does number of variables equal number of states?
    replaceTaggedSection(modelSourceCode, "4", sourceReplace4);
    modelSourceCode.replace("<<<5>>>", sourceReplace5);
    replaceTaggedSection(modelSourceCode, "6", sourceReplace6);
    modelSourceCode.replace("<<<7>>>", modelName);
    modelSourceCode.replace("<<<8>>>", sourceReplace8);
    modelSourceCode.replace("<<<9>>>", sourceReplace9);

    QTextStream modelSourceStream(&modelSourceFile);
    modelSourceStream << modelSourceCode;
    modelSourceFile.close();


    printMessage("Writing HopsanFMU.h...");

    QFile fmuHeaderTemplateFile(":templates/fmuHeaderTemplate.h");
    if(!fmuHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QString fmuHeaderCode;
    QTextStream t3(&fmuHeaderTemplateFile);
    fmuHeaderCode = t3.readAll();
    fmuHeaderTemplateFile.close();
    if(fmuHeaderCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.h.");
        return;
    }

    QTextStream fmuHeaderStream(&fmuHeaderFile);
    fmuHeaderStream << fmuHeaderCode;
    fmuHeaderFile.close();


    printMessage("Writing HopsanFMU.cpp...");

    QString fmuSourceTemplateFileName(":templates/fmuSourceTemplate.c");
    QFile fmuSourceTemplateFile(fmuSourceTemplateFileName);
    if(!fmuSourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

    QString fmuSourceCode;
    QTextStream t4(&fmuSourceTemplateFile);
    fmuSourceCode = t4.readAll();
    fmuSourceTemplateFile.close();
    if(fmuSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.cpp.");
        return;
    }

    fmuSourceCode = replaceTag(fmuSourceCode, "nports", QString::number(inputVariables.size() + outputVariables.size()));

    QString fmuSourceReplace1;
    QString portPointerSection = extractTaggedSection(fmuSourceCode, "assignportpointers");
    for(i=0; i<inputVariables.size(); ++i)
    {
        QStringList tags = QStringList() << "idx" << "comp" << "port";
        QStringList replacements = QStringList() << QString::number(i) << inputComponents.at(i) << inputPorts.at(i);
        fmuSourceReplace1.append(replaceTags(portPointerSection, tags, replacements));
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QStringList tags = QStringList() << "idx" << "comp" << "port";
        QStringList replacements = QStringList() << QString::number(i+j) << outputComponents.at(j) << outputPorts.at(j);
        fmuSourceReplace1.append(replaceTags(portPointerSection, tags, replacements));
    }

    replaceTaggedSection(fmuSourceCode, "assignportpointers", fmuSourceReplace1);
    QTextStream fmuSourceStream(&fmuSourceFile);
    fmuSourceStream << fmuSourceCode;
    fmuSourceFile.close();

    printMessage("Copying HopsanCore files...");

    printMessage("Copying FMI files...");

    QFile fmuModelFunctionsHFile(mBinPath + "/../ThirdParty/fmi/fmiModelFunctions.h");
    fmuModelFunctionsHFile.copy(savePath + "/fmiModelFunctions.h");
    QFile fmuFunctionsHFile(mBinPath + "/../ThirdParty/fmi/fmiFunctions.h");
    fmuFunctionsHFile.copy(savePath + "/fmiFunctions.h");
    QFile fmuPlatformTypesHFile(mBinPath + "/../ThirdParty/fmi/fmiPlatformTypes.h");
    fmuPlatformTypesHFile.copy(savePath + "/fmiPlatformTypes.h");
    QFile fmiModelTypesHFile(mBinPath + "/../ThirdParty/fmi/fmiModelTypes.h");
    fmiModelTypesHFile.copy(savePath + "/fmiModelTypes.h");
    QFile fmiTemplateCFile(mBinPath + "/../ThirdParty/fmi/fmuTemplate.c");
    fmiTemplateCFile.copy(savePath + "/fmuTemplate.c");
    QFile fmiTemplateHFile(mBinPath + "/../ThirdParty/fmi/fmuTemplate.h");
    fmiTemplateHFile.copy(savePath + "/fmuTemplate.h");

    if(!assertFilesExist(savePath, QStringList() << "fmiModelFunctions.h" << "fmiFunctions.h" << "fmiModelTypes.h" << "fmuTemplate.c" << "fmuTemplate.h"))
        return;

    printMessage("Generating model file...");

    QStringList modelLines;
    QFile modelFile(savePath + "/" + realModelName + ".hmf");
    if (!modelFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Could not open "+realModelName+".hmf for reading.");
        return;
    }
    while (!modelFile.atEnd())
    {
        QString line = modelFile.readLine();
        line.chop(1);
        line.replace("\"", "\\\"");
        modelLines.append(line);
    }
    modelLines.last().append("\\n");
    modelFile.close();

    QTextStream modelHppStream(&modelHppFile);
    modelHppStream << "#include <vector>\n\n";
    modelHppStream << "std::string getModelString()\n{\n";
    modelHppStream << "    std::string model = ";
    Q_FOREACH(const QString line, modelLines)
    {
        modelHppStream << "\""+line+"\"\n";
    }
    modelHppStream << "    ;\n\n";
    modelHppStream << "    return model;\n}\n";
    modelHppFile.close();

    printMessage("Replacing namespace...");

    QString nameSpace = "HopsanFMU"+randomString;
    QStringList before = QStringList() << "using namespace hopsan;" << "namespace hopsan " << "\nhopsan::" << "::hopsan::" << " hopsan::" << "*hopsan::" << "namespace hopsan{";
    QStringList after = QStringList() << "using namespace "+nameSpace+";" << "namespace "+nameSpace+" " << "\n"+nameSpace+"::" << "::"+nameSpace+"::" << " "+nameSpace+"::" << "*"+nameSpace+"::" << "namespace "+nameSpace+"{";
    Q_FOREACH(const QString &file, getHopsanCoreSourceFiles())
    {
        if(!replaceInFile(savePath+"/HopsanCore/"+file, before, after))
            return;
    }
    Q_FOREACH(const QString &file, getHopsanCoreIncludeFiles())
    {
        if(!replaceInFile(savePath+"/HopsanCore/"+file, before, after))
            return;
    }
    QStringList componentFiles;
    findAllFilesInFolderAndSubFolders(savePath+"/componentLibraries/defaultLibrary", "hpp", componentFiles);
    findAllFilesInFolderAndSubFolders(savePath+"/componentLibraries/defaultLibrary", "h", componentFiles);
    Q_FOREACH(const QString &file, componentFiles)
    {
        //qDebug() << "Replacing component file: " << file;
        if(!replaceInFile(file, before, after))
            return;
    }
    if(!replaceInFile(savePath+"/HopsanFMU.cpp", before, after))
        return;
    if(!replaceInFile(savePath+"/HopsanFMU.h", before, after))
        return;

#ifdef WIN32
    printMessage("Compiling "+modelName+".dll...");
    //Write the compilation script file
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "gcc.exe -c -w -shared -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c\n";
    clBatchStream << "g++ -w -shared -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB -o "+modelName+".dll "+modelName+".o HopsanFMU.cpp HopsanCore/src/Component.cc HopsanCore/src/ComponentSystem.cc HopsanCore/src/HopsanEssentials.cc HopsanCore/src/HopsanTypes.cc HopsanCore/src/Node.cc HopsanCore/src/Nodes.cc HopsanCore/src/Parameters.cc HopsanCore/src/Port.cc HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc HopsanCore/src/ComponentUtilities/CSVParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc HopsanCore/src/ComponentUtilities/PLOParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/HopsanPowerUser.cc HopsanCore/src/ComponentUtilities/Integrator.cc HopsanCore/src/ComponentUtilities/IntegratorLimited.cc HopsanCore/src/ComponentUtilities/ludcmp.cc HopsanCore/src/ComponentUtilities/matrix.cc HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp HopsanCore/src/CoreUtilities/GeneratorHandler.cpp HopsanCore/src/CoreUtilities/HmfLoader.cc HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc HopsanCore/src/CoreUtilities/LoadExternal.cc HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp HopsanCore/src/CoreUtilities/StringUtilities.cpp componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc HopsanCore/Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp -IHopsanCore/include -IcomponentLibraries/defaultLibrary -IHopsanCore/Dependencies/rapidxml-1.13 -IHopsanCore/Dependencies/libcsv_parser++-1.0.0/include/csv_parser\n";
    clBatchFile.close();

    callProcess("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & compile.bat");

    if(!assertFilesExist(savePath, QStringList() << modelName+".dll"))
        return;

#elif linux
    printMessage("Compiling "+modelName+".so...");

    QString gccCommand1 = "cd "+savePath+" && gcc -c -w -shared -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c\n";
    QString gccCommand2 = "cd "+savePath+" && g++ -w -shared -fPIC -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB -o "+modelName+".so "+modelName+".o HopsanFMU.cpp HopsanCore/src/Component.cc HopsanCore/src/ComponentSystem.cc HopsanCore/src/HopsanEssentials.cc HopsanCore/src/Node.cc HopsanCore/src/Nodes.cc HopsanCore/src/Parameters.cc HopsanCore/src/Port.cc HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc HopsanCore/src/ComponentUtilities/CSVParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc HopsanCore/src/ComponentUtilities/PLOParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/Integrator.cc HopsanCore/src/ComponentUtilities/IntegratorLimited.cc HopsanCore/src/ComponentUtilities/ludcmp.cc HopsanCore/src/ComponentUtilities/matrix.cc HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp HopsanCore/src/CoreUtilities/GeneratorHandler.cpp HopsanCore/src/CoreUtilities/HmfLoader.cc HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc HopsanCore/src/CoreUtilities/LoadExternal.cc HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc HopsanCore/Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp -IHopsanCore/include -IcomponentLibraries/defaultLibrary -IHopsanCore/Dependencies/rapidxml-1.13 -IHopsanCore/Dependencies/libcsv_parser++-1.0.0/include/csv_parser\n";

    //qDebug() << "Command 1: " << gccCommand1;
    //qDebug() << "Command 2: " << gccCommand2;

    callProcess("gcc", QStringList() << "-c" << "-w" << "-shared" << "-fPIC" << "-Wl,--rpath,'$ORIGIN/.'" << modelName+".c", savePath);
    callProcess("g++", QStringList() << "-w" << "-shared" << "-fPIC" << "-DDOCOREDLLEXPORT" << "-DBUILTINDEFAULTCOMPONENTLIB" << "-o"+modelName+".so" << modelName+".o" << "HopsanFMU.cpp" << "HopsanCore/src/Component.cc" << "HopsanCore/src/ComponentSystem.cc" << "HopsanCore/src/HopsanEssentials.cc" << "HopsanCore/src/Node.cc" << "HopsanCore/src/Nodes.cc" << "HopsanCore/src/Parameters.cc" << "HopsanCore/src/Port.cc" << "HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc" << "HopsanCore/src/ComponentUtilities/CSVParser.cc" << "HopsanCore/src/ComponentUtilities/PLOParser.cc"<< "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc" << "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc" << "HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp" << "HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc" << "HopsanCore/src/ComponentUtilities/Integrator.cc" << "HopsanCore/src/ComponentUtilities/IntegratorLimited.cc" << "HopsanCore/src/ComponentUtilities/ludcmp.cc" << "HopsanCore/src/ComponentUtilities/matrix.cc" << "HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc" << "HopsanCore/src/ComponentUtilities/TurbulentFlowFunction.cc" << "HopsanCore/src/ComponentUtilities/ValveHysteresis.cc" << "HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc" << "HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp" << "HopsanCore/src/CoreUtilities/GeneratorHandler.cpp" << "HopsanCore/src/CoreUtilities/HmfLoader.cc" << "HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc" << "HopsanCore/src/CoreUtilities/LoadExternal.cc" << "HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp" << "componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc" << "HopsanCore/Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp" << "-IHopsanCore/include" << "-IcomponentLibraries/defaultLibrary" << "-IHopsanCore/Dependencies/rapidxml-1.13" << "-IHopsanCore/Dependencies/libcsv_parser++-1.0.0/include/csv_parser", savePath);

    if(!assertFilesExist(savePath, QStringList() << modelName+".so"))
        return;
#endif

    printMessage("Sorting files...");

#ifdef WIN32
    saveDir.mkpath("fmu/binaries/win32");
    saveDir.mkpath("fmu/resources");
    QFile modelDllFile(savePath + "/" + modelName + ".dll");
    modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
    QFile modelLibFile(savePath + "/" + modelName + ".lib");
    modelLibFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".lib");
#elif linux && __i386__
    saveDir.mkpath("fmu/binaries/linux32");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux32/" + modelName + ".so");
#elif linux && __x86_64__
    saveDir.mkpath("fmu/binaries/linux64");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux64/" + modelName + ".so");
#endif
    // QFile modelFile(savePath + "/" + realModelName + ".hmf");
    modelFile.copy(savePath + "/fmu/resources/" + realModelName + ".hmf");
    modelDescriptionFile.copy(savePath + "/fmu/modelDescription.xml");
    tlmDescriptionFile.copy(savePath+"/fmu/"+modelName+"_TLM.xml");

    printMessage("Compressing files...");

#ifdef WIN32
    QString program = mBinPath + "../ThirdParty/7z/7z";
    QStringList arguments = QStringList() << "a" << "-tzip" << "../"+modelName+".fmu" << savePath+"/fmu/modelDescription.xml" << savePath+"/fmu/"+modelName+"_TLM.xml" << "-r" << savePath + "/fmu/binaries";
    callProcess(program, arguments, savePath+"/fmu");
#elif linux && __i386__
    QStringList arguments = QStringList() << "-r" << "../"+modelName+".fmu" << "modelDescription.xml" << modelName+"_TLM.xml" << "binaries/linux32/"+modelName+".so";
    callProcess("zip", arguments, savePath+"/fmu");
#elif linux && __x86_64__
    QStringList arguments = QStringList() << "-r" << "../"+modelName+".fmu" << "modelDescription.xml" << modelName+"_TLM.xml" << "binaries/linux64/"+modelName+".so";
    callProcess("zip", arguments, savePath+"/fmu");
#endif

    if(!assertFilesExist(savePath, QStringList() << modelName+".fmu"))
        return;

    //Clean up temporary files
    cleanUp(savePath, QStringList() << "compile.bat" << modelName+".c" << modelName+".dll" << modelName+".so" << modelName+".o" << modelName+".hmf" <<
            "fmiModelFunctions.h" << "fmiModelTypes.h" << "fmuTemplate.c" << "fmuTemplate.h" << "HopsanFMU.cpp" << "HopsanFMU.h" << "model.hpp" <<
            "modelDescription.xml", QStringList() << "componentLibraries" << "fmu" << "HopsanCore");

    printMessage("Finished.");
}


bool HopsanFMIGenerator::readTLMSpecsFromFile(const QString &fileName, QStringList &tlmPortTypes, QList<QStringList> &tlmPortVarNames,
                                              QList<QStringList> &tlmPortValueRefs, QStringList &inVarValueRefs, QStringList &inVarPortNames,
                                              QStringList &outVarValueRefs, QStringList &outVarPortNames, QString &cqsType)
{
    QFile tlmSpecFile;
    tlmSpecFile.setFileName(fileName);
    QDomDocument tlmDomDocument;
    QDomElement tlmRoot;
    if(tlmSpecFile.exists())
    {
        printMessage("Reading TLM specifications from "+tlmSpecFile.fileName()+"...");
        tlmRoot = loadXMLDomDocument(tlmSpecFile, tlmDomDocument, "fmutlm");
        tlmSpecFile.close();
    }
    else
    {
        printMessage("No TLM specification file found.");
        return true;
    }

    if(tlmRoot == QDomElement())
    {
        printErrorMessage("Unable to parse TLM specification file.");
        return true;        // Don't abort import, it could still work without the TLM stuff
    }
    else
    {
        QString type;
        QStringList qVars;
        QStringList cVars;

        QDomElement portElement = tlmRoot.firstChildElement("tlmport");
        while(!portElement.isNull())
        {
            type = portElement.attribute("type");

            QStringList nodeTypes;
            GeneratorNodeInfo::getNodeTypes(nodeTypes);
            Q_FOREACH(const QString &nodeType, nodeTypes)
            {
                GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);
                if(type == info.niceName+"q" || type == info.niceName+"c")
                {
                    Q_FOREACH(const QString &var, info.qVariables)
                    {
                        QDomElement element = portElement.firstChildElement(var);
                        if(element.isNull())
                        {
                            printErrorMessage("Node type does not match variable names.");
                            return false;
                        }
                        qVars.append(element.text());
                    }
                    Q_FOREACH(const QString &var, info.cVariables)
                    {
                        QDomElement element = portElement.firstChildElement(var);
                        if(element.isNull())
                        {
                            printErrorMessage("Node type does not match variable names.");
                            return false;
                        }
                        cVars.append(element.text());
                    }
                }
            }

            Q_FOREACH(const QString &nodeType, nodeTypes)
            {
                GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);
                if(type == info.niceName+"q")
                {
                    QStringList varNames;
                    tlmPortValueRefs.append(QStringList());
                    Q_FOREACH(const QString &var, qVars)
                    {
                        if(!outVarPortNames.contains(var))
                        {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(var)]);
                        outVarValueRefs.removeAt(outVarPortNames.indexOf(var));
                        outVarPortNames.removeAll(var);
                    }
                    Q_FOREACH(const QString &var, cVars)
                    {
                        if(!inVarPortNames.contains(var))
                        {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(var)]);
                        inVarValueRefs.removeAt(inVarPortNames.indexOf(var));
                        inVarPortNames.removeAll(var);
                    }

                    printMessage("Adding "+info.niceName+" port of Q-type");

                    if(cqsType == "Signal" || cqsType == "Q")
                        cqsType = "Q";
                    else
                        cqsType = "";

                    tlmPortTypes.append(type);
                    tlmPortVarNames.append(varNames);
                }
                else if(type == info.niceName+"c")
                {
                    QStringList varNames;
                    tlmPortValueRefs.append(QStringList());
                    Q_FOREACH(const QString &var, qVars)
                    {
                        if(!inVarPortNames.contains(var))
                        {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(var)]);
                        inVarValueRefs.removeAt(inVarPortNames.indexOf(var));
                        inVarPortNames.removeAll(var);
                    }
                    Q_FOREACH(const QString &var, cVars)
                    {
                        if(!outVarPortNames.contains(var))
                        {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(var)]);
                        outVarValueRefs.removeAt(outVarPortNames.indexOf(var));
                        outVarPortNames.removeAll(var);
                    }

                    printMessage("Adding "+info.niceName+" port of C-type");

                    if(cqsType == "Signal" || cqsType == "C")
                        cqsType = "C";
                    else
                        cqsType = "";

                    tlmPortTypes.append(type);
                    tlmPortVarNames.append(varNames);
                }
            }
            cVars.clear();
            qVars.clear();
            portElement = portElement.nextSiblingElement("tlmport");
        }
    }

    return true;
}


void HopsanFMIGenerator::getInterfaceInfo(QString typeName, QString compName,
                                          QStringList &inVars, QStringList &inComps, QStringList &inPorts, QList<int> &inDatatypes,
                                          QStringList &outVars, QStringList &outComps, QStringList &outPorts, QList<int> &outDatatypes,
                                          QList<QStringList> &tlmPorts)
{
    if(typeName == "SignalInputInterface")
    {
        inVars.append(compName.remove(' ').remove("-"));
        inComps.append(compName);
        inPorts.append("out");
        inDatatypes.append(0);
    }
    else if(typeName == "SignalOutputInterface")
    {
        outVars.append(compName.remove(' ').remove("-"));
        outComps.append(compName);
        outPorts.append("in");
        outDatatypes.append(0);
    }

    QString nodeType, cqType;
    QString portName = "P1";
    getNodeAndCqTypeFromInterfaceComponent(typeName, nodeType, cqType);

    if(cqType == "c")
    {
        QString name=compName;
        name.remove(' ').remove("-");

        GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);

        tlmPorts.append(QStringList() << info.niceName+"q");
        Q_FOREACH(const QString &var, info.qVariables)
        {
            outVars.append(name+"_"+var+"__");
            outComps.append(compName);
            outPorts.append(portName);
            outDatatypes.append(info.varIdx[info.qVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
        Q_FOREACH(const QString &var, info.cVariables)
        {
            inVars.append(name+"_"+var+"__");
            inComps.append(compName);
            inPorts.append("P1");
            inDatatypes.append(info.varIdx[info.qVariables.size()+info.cVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
    }
    else if(cqType == "q")
    {
        QString name=compName.remove(' ').remove("-");

        GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);

        tlmPorts.append(QStringList() << info.niceName+"c");
        Q_FOREACH(const QString &var, info.qVariables)
        {
            inVars.append(name+"_"+var+"__");
            inComps.append(compName);
            inPorts.append("P1");
            inDatatypes.append(info.varIdx[info.qVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
        Q_FOREACH(const QString &var, info.cVariables)
        {
            outVars.append(name+"_"+var+"__");
            outComps.append(compName);
            outPorts.append("P1");
            outDatatypes.append(info.varIdx[info.qVariables.size()+info.cVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
    }
}

