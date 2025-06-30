#ifndef INCODE_MAINWINDOW_H
#define INCODE_MAINWINDOW_H

#include <QMainWindow>
#include "ISymbolProvider.h"
#include "CodeAnalyzer.h"
#include "widgets/CodeEditor.h"
#include <QThread>
#include <QProgressBar>
#include <QLabel>

class QTabWidget;
class QTreeView;
class QFileSystemModel;
// class QTextEdit; // No longer needed for editor
class QProcess;
class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void openFile();
    void openFolder();
    void saveFile();
    void onFileTreeDoubleClicked(const QModelIndex &index);
    void onTabCloseRequested(int index);
    void handleTerminalCommand();
    void readTerminalOutput();
    void goToDefinition(const QString &symbolName);
    void analyzeCode();
    void onAnalysisFinished(const QList<CodeRepetition> &repetitions);
    void onIndexingProgress(int progress);
    void onIndexingFinished();

private:
    void openFile(const QString &filePath);
    void createMenus();
    void createWidgets();
    void setupLayout();
    void setupConnections();

    QTabWidget *tabWidget;
    QTreeView *treeView;
    QFileSystemModel *fileModel;
    QTextEdit *terminalOutput; // Still QTextEdit for terminal output
    QLineEdit *terminalInput;
    QProcess *terminalProcess;
    ISymbolProvider *symbolProvider;
    CodeAnalyzer *codeAnalyzer;
    QThread *indexingThread;
    QProgressBar *indexingProgressBar;
    QLabel *indexingStatusLabel;
};

#endif // INCODE_MAINWINDOW_H