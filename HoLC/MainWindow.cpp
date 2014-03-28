#include <QDockWidget>
#include <QColor>
#include <QAction>
#include <QToolBar>
#include <QGridLayout>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>

#include "MainWindow.h"
#include "Widgets/ProjectFilesWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/EditorWidget.h"
#include "Widgets/OptionsWidget.h"
#include "Handlers/MessageHandler.h"
#include "Handlers/FileHandler.h"
#include "Handlers/OptionsHandler.h"
#include "Dialogs/NewProjectDialog.h"
#include "Dialogs/CreateComponentWizard.h"

MainWindow::MainWindow(QWidget *pParent)
    : QMainWindow(pParent)
{
    // Set the size and position of the main window
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    this->resize(sw*0.8, sh*0.8);   //Resize window to 80% of screen height and width

    //Create dock widgets
    QDockWidget *pFilesDockWidget = new QDockWidget("Project Files", this);
    QDockWidget *pMessageDockWidget = new QDockWidget("Messages", this);

    //Add dock widgets
    this->addDockWidget(Qt::LeftDockWidgetArea, pFilesDockWidget);
    this->addDockWidget(Qt::BottomDockWidgetArea, pMessageDockWidget);

    mpOptionsHandler = new OptionsHandler();

    //Create widgets
    mpEditorWidget = new EditorWidget(this);
    mpProjectFilesWidget = new ProjectFilesWidget(this);
    mpMessageWidget = new MessageWidget(this);
    mpOptionsWidget = new OptionsWidget(mpOptionsHandler, this);

    //Assign widgets to areas
    QWidget *pCentralWidget = new QWidget(this);
    QGridLayout *pCentralLayout = new QGridLayout(pCentralWidget);
    pCentralLayout->addWidget(mpEditorWidget,0,0);
    pCentralLayout->addWidget(mpOptionsWidget,0,0);
    mpOptionsWidget->setHidden(true);
    setCentralWidget(pCentralWidget);
    pFilesDockWidget->setWidget(mpProjectFilesWidget);
    pMessageDockWidget->setWidget(mpMessageWidget);

    //Create toolbars
    QAction *pNewAction = new QAction("New Library", this);
    pNewAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-New.png"));
    QAction *pOpenAction = new QAction("Open Library", this);
    pOpenAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Open.png"));
    QAction *pSaveAction = new QAction("Save Current File", this);
    pSaveAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Save.png"));
    QAction *pAddComponentAction = new QAction("Add New Component", this);
    pAddComponentAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Add.png"));
    QAction *pAddComponentFromFileAction = new QAction("Add Component From Existing File", this);
    pAddComponentFromFileAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-AddFromFile.png"));
    QAction *pOptionsAction = new QAction("Options", this);
    pOptionsAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Options.png"));
    pOptionsAction->setCheckable(true);
    QAction *pCompileAction = new QAction("Compile Library", this);
    pCompileAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Compile.png"));

    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pNewAction);
    pToolBar->addAction(pOpenAction);
    pToolBar->addAction(pSaveAction);
    pToolBar->addAction(pAddComponentAction);
    pToolBar->addAction(pAddComponentFromFileAction);
    pToolBar->addAction(pOptionsAction);
    pToolBar->addAction(pCompileAction);
    this->addToolBar(pToolBar);

    //Create handlers
    mpMessageHandler = new MessageHandler(mpMessageWidget);
    mpFileHandler = new FileHandler(mpProjectFilesWidget, mpEditorWidget, mpMessageHandler, mpOptionsHandler);

    //Create dialogs
    mpNewProjectDialog = new NewProjectDialog(mpFileHandler, this);
    mpNewProjectDialog->hide();
    mpCreateComponentWizard = new CreateComponentWizard(mpFileHandler, mpMessageHandler, this);
    mpCreateComponentWizard->hide();

    //Setup connetions
    connect(mpEditorWidget,                 SIGNAL(textChanged()),      mpFileHandler,              SLOT(updateText()));
    connect(pOpenAction,                    SIGNAL(triggered()),        mpFileHandler,              SLOT(loadFromXml()));
    connect(pSaveAction,                    SIGNAL(triggered()),        mpFileHandler,              SLOT(saveToXml()));
    connect(mpEditorWidget,                 SIGNAL(textChanged()),      mpProjectFilesWidget,       SLOT(addAsterisk()));
    connect(pOptionsAction,                 SIGNAL(toggled(bool)),      mpOptionsWidget,            SLOT(setVisible(bool)));
    connect(pOptionsAction,                 SIGNAL(toggled(bool)),      mpEditorWidget,             SLOT(setHidden(bool)));
    connect(pCompileAction,                 SIGNAL(triggered()),        mpFileHandler,              SLOT(compileLibrary()));
    connect(mpFileHandler,                  SIGNAL(fileOpened(bool)),   pOptionsAction,             SLOT(setChecked(bool)));
    connect(pNewAction,                     SIGNAL(triggered()),        mpNewProjectDialog,         SLOT(show()));
    connect(pAddComponentAction,            SIGNAL(triggered()),        mpCreateComponentWizard,    SLOT(open()));
    connect(pAddComponentFromFileAction,    SIGNAL(triggered()),        mpFileHandler,              SLOT(addComponent()));

    //Debug
    QString msg = "Test message!";
    mpMessageHandler->addInfoMessage(msg);
    mpMessageHandler->addWarningMessage(msg);
    mpMessageHandler->addErrorMessage(msg);
}

MainWindow::~MainWindow()
{
}
