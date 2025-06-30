#ifndef ISYMBOLPROVIDER_H
#define ISYMBOLPROVIDER_H

#include <QString>
#include <QMap>

struct SymbolLocation {
    QString filePath;
    int lineNumber;
};

class ISymbolProvider {
public:
    virtual ~ISymbolProvider() = default;

    // Method to find the definition of a symbol
    virtual SymbolLocation findSymbolLocation(const QString &symbolName) const = 0;

    // Method to index a directory (e.g., when a folder is opened)
    virtual void indexDirectory(const QString &directoryPath) = 0;
};

#endif // ISYMBOLPROVIDER_H
