#ifndef CODEANALYZER_H
#define CODEANALYZER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QRegularExpression>

// Structure to hold information about a code repetition
struct CodeRepetition {
    QString filePath;
    int startLine;
    int endLine;
    QString snippet; // The actual repeated code snippet
};

class CodeAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit CodeAnalyzer(QObject *parent = nullptr);

    // Method to start the analysis for a list of directories
    void analyzePaths(const QList<QString> &paths);

signals:
    // Signal emitted when analysis is complete, providing the repetitions found
    void analysisFinished(const QList<CodeRepetition> &repetitions);

private:
    // Helper to analyze a single file
    void analyzeFile(const QString &filePath);

    // Store detected repetitions
    QList<CodeRepetition> detectedRepetitions;

    // Map to store hashes of code snippets and their locations
    // Key: hash of the snippet
    // Value: List of locations where this snippet appears
    QMap<QString, QList<CodeRepetition>> snippetHashes;

    // Minimum number of lines for a snippet to be considered for repetition
    const int MIN_LINES_FOR_REPETITION = 5;
};

#endif // CODEANALYZER_H
