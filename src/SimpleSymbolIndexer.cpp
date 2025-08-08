#include "SimpleSymbolIndexer.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDirIterator>
#include <QDebug>
#include <QDir>

SimpleSymbolIndexer::SimpleSymbolIndexer(QObject *parent)
    : QObject(parent)
{
}

SimpleSymbolIndexer::~SimpleSymbolIndexer()
{
}

SymbolLocation SimpleSymbolIndexer::findSymbolLocation(const QString &symbolName) const
{
    if (symbolMap.contains(symbolName)) {
        return symbolMap.value(symbolName);
    }
    return SymbolLocation{"", -1}; // Not found
}

QStringList SimpleSymbolIndexer::allSymbols() const
{
    return symbolMap.keys();
}

void SimpleSymbolIndexer::indexDirectory(const QString &directoryPath)
{
    // This method is called from the main thread to initiate indexing in the worker thread
    emit startIndexing(directoryPath);
}

void SimpleSymbolIndexer::doIndexDirectory(const QString &directoryPath)
{
    qDebug() << "Indexing started for directory:" << directoryPath;
    symbolMap.clear(); // Clear existing symbols

    QDirIterator it(directoryPath, QStringList() << "*.php", QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QList<QString> filesToProcess;
    while (it.hasNext()) {
        filesToProcess.append(it.next());
    }

    int totalFiles = filesToProcess.size();
    for (int i = 0; i < totalFiles; ++i) {
        indexFile(filesToProcess.at(i));
        emit indexingProgress((i * 100) / totalFiles);
    }

    qDebug() << "Indexing finished. Total symbols:" << symbolMap.size();
    emit indexingFinished();
}

void SimpleSymbolIndexer::indexFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for indexing:" << filePath;
        return;
    }

    QTextStream in(&file);
    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        // Simple regex for class and function names
        QRegularExpression classRegex("\\bclass\\s+(\\w+)\\b");
        QRegularExpressionMatch classMatch = classRegex.match(line);
        if (classMatch.hasMatch()) {
            QString className = classMatch.captured(1);
            symbolMap.insert(className, SymbolLocation{filePath, lineNumber});
            //qDebug() << "Found class:" << className << "in" << filePath << "at line" << lineNumber;
        }

        QRegularExpression functionRegex("\\bfunction\\s+(\\w+)\\s*\\(\\)");
        QRegularExpressionMatch functionMatch = functionRegex.match(line);
        if (functionMatch.hasMatch()) {
            QString functionName = functionMatch.captured(1);
            symbolMap.insert(functionName, SymbolLocation{filePath, lineNumber});
            //qDebug() << "Found function:" << functionName << "in" << filePath << "at line" << lineNumber;
        }
    }
    file.close();
}