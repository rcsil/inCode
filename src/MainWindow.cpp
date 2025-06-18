#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QSplitter>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QModelIndex>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), editor(new Editor(this)) {
    setupLayout();
    createMenu();
    setWindowTitle("inCode");
    resize(1000, 600);
}

void MainWindow::setupLayout() {
    fileTree = new QTreeView(this);
    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::homePath());
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    fileTree->setModel(fileModel);
    fileTree->hideColumn(1);
    fileTree->hideColumn(2);
    fileTree->hideColumn(3);

    connect(fileTree, &QTreeView::clicked, this, &MainWindow::fileClicked);

    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(fileTree);
    splitter->addWidget(editor);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);
}

void MainWindow::createMenu() {
    QMenu *fileMenu = menuBar()->addMenu("&Arquivo");

    QAction *newAct = fileMenu->addAction("&Novo");
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    QAction *openAct = fileMenu->addAction("&Abrir...");
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    QAction *saveAct = fileMenu->addAction("&Salvar");
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    fileMenu->addSeparator();

    QAction *openProj = fileMenu->addAction("Abrir &Projeto");
    connect(openProj, &QAction::triggered, this, &MainWindow::openProject);
}

void MainWindow::newFile() {
    editor->clear();
    currentFilePath.clear();
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir arquivo");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            editor->setPlainText(in.readAll());
            currentFilePath = fileName;
        }
    }
}

void MainWindow::saveFile() {
    if (currentFilePath.isEmpty()) {
        currentFilePath = QFileDialog::getSaveFileName(this, "Salvar arquivo");
    }
    if (!currentFilePath.isEmpty()) {
        QFile file(currentFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << editor->toPlainText();
        }
    }
}

void MainWindow::openProject() {
    QString dir = QFileDialog::getExistingDirectory(this, "Selecionar pasta");
    if (!dir.isEmpty()) {
        fileModel->setRootPath(dir);
        fileTree->setRootIndex(fileModel->index(dir));
    }
}

void MainWindow::fileClicked(const QModelIndex &index) {
    QString path = fileModel->filePath(index);
    QFileInfo info(path);

    if (info.isFile()) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in (&file);
            editor->setPlainText(in.readAll());
            currentFilePath = path;
        }
    }
}

void setDarkTheme(QApplication &app) {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Highlight, QColor(60, 60, 60));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);

    app.setPalette(darkPalette);
    app.setStyle("Fusion");
}

