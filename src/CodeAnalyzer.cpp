#include "CodeAnalyzer.h"
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QDebug>

CodeAnalyzer::CodeAnalyzer(QObject *parent) : QObject(parent)
{
}

void CodeAnalyzer::analyzePaths(const QList<QString> &paths)
{
    detectedRepetitions.clear();
    snippetHashes.clear();

    qDebug() << "Starting code analysis for paths:" << paths;

    for (const QString &path : paths) {
        QDirIterator it(path, QStringList() << "*.php", QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            analyzeFile(it.next());
        }
    }

    // Now, identify actual repetitions (snippets appearing more than once)
    for (auto it = snippetHashes.constBegin(); it != snippetHashes.constEnd(); ++it) {
        if (it.value().size() > 1) {
            // This snippet appears multiple times, so it's a repetition
            detectedRepetitions.append(it.value());
        }
    }

    qDebug() << "Code analysis finished. Found" << detectedRepetitions.size() << "repetitions.";
    emit analysisFinished(detectedRepetitions);
}

void CodeAnalyzer::analyzeFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for analysis:" << filePath;
        return;
    }

    QTextStream in(&file);
    QList<QString> processedLines;
    int currentLineNumber = 0;

    QRegularExpression useRegex("^\\s*use\\s+[^;]+;");
    QRegularExpression classRegex("^\\s*(?:abstract\\s+|final\\s+)?class\\s+\\w+");
    QRegularExpression namespaceRegex("^\\s*namespace\\s+[A-Za-z0-9_\\]+(?:\\s*;|\\s*\\{)?$");

    while (!in.atEnd()) {
        QString line = in.readLine();
        currentLineNumber++;

        // Ignore lines with use, class, or namespace declarations
        if (line.contains(useRegex) || line.contains(classRegex) || line.contains(namespaceRegex)) {
            continue;
        }
        processedLines.append(line);
    }
    file.close();

    // Iterate through processed lines to find snippets
    for (int i = 0; i < processedLines.size() - MIN_LINES_FOR_REPETITION + 1; ++i) {
        QString currentSnippet;
        for (int j = 0; j < MIN_LINES_FOR_REPETITION; ++j) {
            currentSnippet += processedLines.at(i + j) + "\n";
        }

        // Calculate hash of the snippet
        QByteArray hash = QCryptographicHash::hash(currentSnippet.toUtf8(), QCryptographicHash::Md5);
        QString snippetHash = QString(hash.toHex());

        // Store the snippet and its location
        CodeRepetition repetition;
        repetition.filePath = filePath;
        // Note: startLine and endLine here will be relative to processedLines, not original file lines.
        // For accurate line numbers, we'd need to map back to original line numbers.
        // For now, this is a simplification.
        repetition.startLine = i + 1; // Line numbers are 1-based
        repetition.endLine = i + MIN_LINES_FOR_REPETITION;
        repetition.snippet = currentSnippet.trimmed(); // Store trimmed snippet

        snippetHashes[snippetHash].append(repetition);
    }
}
