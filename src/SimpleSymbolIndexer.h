#ifndef INCODE_SIMPLESYMBOLINDEXER_H
#define INCODE_SIMPLESYMBOLINDEXER_H

#include "ISymbolProvider.h"
#include <QMap>
#include <QString>
#include <QObject>

class SimpleSymbolIndexer : public QObject, public ISymbolProvider
{
    Q_OBJECT

public:
    explicit SimpleSymbolIndexer(QObject *parent = nullptr);
    ~SimpleSymbolIndexer() override;

    SymbolLocation findSymbolLocation(const QString &symbolName) const override;
    void indexDirectory(const QString &directoryPath) override; // Called from main thread
    QStringList allSymbols() const override;

public slots:
    void doIndexDirectory(const QString &directoryPath); // This will run in the thread

signals:
    void startIndexing(const QString &directoryPath); // New signal to trigger work in worker thread
    void indexingProgress(int progress);
    void indexingFinished();

private:
    void indexFile(const QString &filePath);

    QMap<QString, SymbolLocation> symbolMap;
};

#endif // INCODE_SIMPLESYMBOLINDEXER_H
