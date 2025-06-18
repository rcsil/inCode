#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QFileSystemModel>
#include "Editor.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void openProject();
    void fileClicked(const QModelIndex &index);

private:
    Editor *editor;
    QTreeView *fileTree;
    QFileSystemModel *fileModel;
    QString currentFilePath;

    void createMenu();
    void setupLayout();
};

#endif // MAINWINDOW_H