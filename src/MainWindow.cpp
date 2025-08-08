#include "MainWindow.h"
#include "SimpleSymbolIndexer.h"
#include "CodeAnalyzer.h"
#include <QTabWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QTextEdit>
#include <QProcess>
#include <QLineEdit>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QDir>
#include <QTextBlock>

#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qDebug() << "MainWindow constructor started.";
    setWindowTitle("inCode IDE");
    setGeometry(100, 100, 1200, 800);

    createWidgets();
    setupLayout();
    createMenus();
    setupConnections();
    qDebug() << "MainWindow constructor finished.";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destructor started.";

    // Terminate terminal process if running
    if (terminalProcess && terminalProcess->state() == QProcess::Running) {
        terminalProcess->terminate();
        terminalProcess->waitForFinished();
    }

    // Stop and wait for the indexing thread to finish
    if (indexingThread && indexingThread->isRunning()) {
        qDebug() << "Quitting indexing thread...";
        indexingThread->quit();
        indexingThread->wait(); // Wait for the thread to finish
        qDebug() << "Indexing thread finished.";
    }

    // Delete objects that were moved to the thread or managed by it
    // QObject::deleteLater connections handle deletion, but explicit deletion here
    // ensures they are cleaned up if the thread was already quit.
    delete codeAnalyzer;   // Delete the code analyzer

    qDebug() << "MainWindow destructor finished.";
}

void MainWindow::createWidgets()
{
    qDebug() << "createWidgets started.";
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);

    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::currentPath());

    treeView = new QTreeView(this);
    treeView->setModel(fileModel);
    treeView->setRootIndex(fileModel->index(QDir::currentPath()));
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    treeView->setColumnHidden(3, true);
    treeView->setHeaderHidden(true);

    terminalOutput = new QPlainTextEdit(this);
    terminalOutput->setReadOnly(true); // Make it read-only for historical output
    terminalOutput->setPlainText("$ "); // Initial prompt

    terminalInput = new QLineEdit(this);

    terminalProcess = new QProcess(this);
    terminalProcess->setProcessChannelMode(QProcess::MergedChannels);
    terminalProcess->start("/bin/bash");
    if (!terminalProcess->waitForStarted(1000)) {
        qDebug() << "Terminal process failed to start!";
    } else {
        qDebug() << "Terminal process started successfully.";
    }

    symbolProvider = new SimpleSymbolIndexer(); // Instantiate the simple indexer
    indexingThread = new QThread(this);
    static_cast<SimpleSymbolIndexer*>(symbolProvider)->moveToThread(indexingThread);

    // Connect signals to start work in the thread
    connect(static_cast<SimpleSymbolIndexer*>(symbolProvider), &SimpleSymbolIndexer::startIndexing, static_cast<SimpleSymbolIndexer*>(symbolProvider), &SimpleSymbolIndexer::doIndexDirectory);
    connect(indexingThread, &QThread::finished, static_cast<SimpleSymbolIndexer*>(symbolProvider), &QObject::deleteLater);
    connect(indexingThread, &QThread::finished, indexingThread, &QObject::deleteLater);

    // Connect progress signals
    connect(static_cast<SimpleSymbolIndexer*>(symbolProvider), &SimpleSymbolIndexer::indexingProgress, this, &MainWindow::onIndexingProgress);
    connect(static_cast<SimpleSymbolIndexer*>(symbolProvider), &SimpleSymbolIndexer::indexingFinished, this, &MainWindow::onIndexingFinished);

    indexingThread->start(); // Start the thread
    codeAnalyzer = new CodeAnalyzer(this);         // Instantiate the code analyzer

    qDebug() << "Terminal setup complete.";
    qDebug() << "createWidgets finished.";
}

void MainWindow::setupLayout()
{
    qDebug() << "setupLayout started.";

    setCentralWidget(tabWidget);

    // Explorer dock
    QDockWidget *explorerDock = new QDockWidget(tr("Explorer"), this);
    explorerDock->setWidget(treeView);
    addDockWidget(Qt::LeftDockWidgetArea, explorerDock);

    // Terminal dock
    QWidget *terminalWidget = new QWidget(this);
    QVBoxLayout *terminalLayout = new QVBoxLayout(terminalWidget);
    terminalLayout->setContentsMargins(0, 0, 0, 0);
    terminalLayout->addWidget(terminalOutput);
    terminalLayout->addWidget(terminalInput);

    QDockWidget *terminalDock = new QDockWidget(tr("Terminal"), this);
    terminalDock->setWidget(terminalWidget);
    addDockWidget(Qt::BottomDockWidgetArea, terminalDock);

    // Status bar for indexing progress
    indexingStatusLabel = new QLabel("Ready");
    statusBar()->addWidget(indexingStatusLabel);

    indexingProgressBar = new QProgressBar();
    indexingProgressBar->setRange(0, 100);
    indexingProgressBar->setValue(0);
    indexingProgressBar->setTextVisible(false);
    indexingProgressBar->setFixedWidth(200);
    statusBar()->addWidget(indexingProgressBar);

    qDebug() << "setupLayout finished.";
}

void MainWindow::createMenus()
{
    qDebug() << "createMenus started.";
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *newAction = new QAction("&New File", this);
    newAction->setShortcuts(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction("&Open File...", this);
    openAction->setShortcuts(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, static_cast<void (MainWindow::*)()>(&MainWindow::openFile));
    fileMenu->addAction(openAction);

    QAction *openFolderAction = new QAction("Open &Folder...", this);
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::openFolder);
    fileMenu->addAction(openFolderAction);

    QAction *saveAction = new QAction("&Save", this);
    saveAction->setShortcuts(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(saveAction);

    QMenu *analyzeMenu = menuBar()->addMenu("&Analyze");
    QAction *analyzeCodeAction = new QAction("Analyze Code Repetitions", this);
    connect(analyzeCodeAction, &QAction::triggered, this, &MainWindow::analyzeCode);
    analyzeMenu->addAction(analyzeCodeAction);

    qDebug() << "createMenus finished.";
}

void MainWindow::setupConnections()
{
    qDebug() << "setupConnections started.";
    connect(treeView, &QTreeView::doubleClicked, this, &MainWindow::onFileTreeDoubleClicked);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    connect(terminalProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readTerminalOutput);
    connect(terminalInput, &QLineEdit::returnPressed, this, &MainWindow::handleTerminalCommand);
    connect(codeAnalyzer, &CodeAnalyzer::analysisFinished, this, &MainWindow::onAnalysisFinished);
    qDebug() << "setupConnections finished.";
}

void MainWindow::newFile()
{
    CodeEditor *editor = new CodeEditor(symbolProvider);
    int index = tabWidget->addTab(editor, "Untitled");
    tabWidget->setCurrentIndex(index);
    connect(editor, &CodeEditor::goToDefinitionRequested, this, &MainWindow::goToDefinition);
}

void MainWindow::openFile(const QString &filePath)
{
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + filePath);
        return;
    }

    CodeEditor *editor = new CodeEditor(symbolProvider);
    editor->setPlainText(file.readAll());
    file.close();

    int index = tabWidget->addTab(editor, QFileInfo(filePath).fileName());
    tabWidget->setCurrentIndex(index);
    connect(editor, &CodeEditor::goToDefinitionRequested, this, &MainWindow::goToDefinition);
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open File");
    openFile(filePath);
}

void MainWindow::openFolder()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Open Folder");
    if (!dirPath.isEmpty()) {
        fileModel->setRootPath(dirPath);
        treeView->setRootIndex(fileModel->index(dirPath));

        indexingStatusLabel->setText("Indexing...");
        indexingProgressBar->setValue(0);
        indexingProgressBar->show();

        // Emit signal to start indexing in the worker thread
        emit static_cast<SimpleSymbolIndexer*>(symbolProvider)->startIndexing(dirPath);
    }
}

void MainWindow::saveFile()
{
    if (tabWidget->count() == 0) return;

    QTextEdit *currentEditor = qobject_cast<QTextEdit*>(tabWidget->currentWidget());
    if (!currentEditor) return;

    // Simplified save logic. Needs improvement to track file paths.
    QString filePath = QFileDialog::getSaveFileName(this, "Save File");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not save file.");
        return;
    }

    QTextStream out(&file);
    out << currentEditor->toPlainText();
    file.close();

    tabWidget->setTabText(tabWidget->currentIndex(), QFileInfo(filePath).fileName());
}

void MainWindow::onFileTreeDoubleClicked(const QModelIndex &index)
{
    if (fileModel->isDir(index)) return;

    QString filePath = fileModel->filePath(index);
    openFile(filePath);
}

void MainWindow::onTabCloseRequested(int index)
{
    QWidget *widget = tabWidget->widget(index);
    tabWidget->removeTab(index);
    delete widget;
}

void MainWindow::handleTerminalCommand()
{
    QString command = terminalInput->text();
    terminalInput->clear();
    terminalProcess->write((command + "\n").toUtf8());
    terminalOutput->appendPlainText("$ " + command);
}

void MainWindow::readTerminalOutput()
{
    terminalOutput->appendPlainText(terminalProcess->readAllStandardOutput());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (terminalInput->isVisible() && terminalInput->isEnabled()) {
        terminalInput->setFocus();
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::goToDefinition(const QString &symbolName)
{
    qDebug() << "Attempting to go to definition for:" << symbolName;
    SymbolLocation location = symbolProvider->findSymbolLocation(symbolName);
    if (!location.filePath.isEmpty() && location.lineNumber != -1) {
        qDebug() << "Found symbol at:" << location.filePath << ":" << location.lineNumber;
        // TODO: Open file and navigate to line. This requires a custom editor widget.
        openFile(location.filePath); // For now, just open the file
        // In a real scenario, we'd also scroll to the line number.
    } else {
        qDebug() << "Symbol not found:" << symbolName;
        QMessageBox::information(this, "Go to Definition", "Symbol '" + symbolName + "' not found.");
    }
}

void MainWindow::analyzeCode()
{
    QString currentDirPath = fileModel->rootPath();
    if (currentDirPath.isEmpty()) {
        QMessageBox::warning(this, "Analysis", "Please open a folder first to analyze code.");
        return;
    }

    QList<QString> pathsToAnalyze;
    pathsToAnalyze << QDir(currentDirPath).filePath("app");

    // Filter out paths that don't exist
    QList<QString> existingPaths;
    for (const QString &path : pathsToAnalyze) {
        if (QDir(path).exists()) {
            existingPaths.append(path);
        } else {
            qDebug() << "Skipping non-existent path:" << path;
        }
    }

    if (existingPaths.isEmpty()) {
        QMessageBox::information(this, "Analysis", "No 'app' or 'resources' folders found in the current project to analyze.");
        return;
    }

    codeAnalyzer->analyzePaths(existingPaths);
}

void MainWindow::onAnalysisFinished(const QList<CodeRepetition> &repetitions)
{
    QString resultText;
    if (repetitions.isEmpty()) {
        resultText = "No significant code repetitions found in 'app' and 'resources' folders.";
    } else {
        resultText = QString("Found %1 code repetitions:\n\n").arg(repetitions.size());
        for (const CodeRepetition &rep : repetitions) {
            resultText += QString("File: %1, Lines: %2-%3\n").arg(rep.filePath).arg(rep.startLine).arg(rep.endLine);
            resultText += "Snippet:\n" + rep.snippet + "\n\n";
        }
    }
    QMessageBox::information(this, "Code Analysis Results", resultText);
}

void MainWindow::onIndexingProgress(int progress)
{
    indexingProgressBar->setValue(progress);
    indexingStatusLabel->setText(QString("Indexing: %1%").arg(progress));
}

void MainWindow::onIndexingFinished()
{
    indexingProgressBar->setValue(100);
    indexingStatusLabel->setText("Ready");
    indexingProgressBar->hide();

    for (int i = 0; i < tabWidget->count(); ++i) {
        if (CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(i))) {
            editor->setSymbolProvider(symbolProvider);
        }
    }
}