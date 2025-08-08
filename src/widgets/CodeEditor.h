#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QCompleter>

#include "../ISymbolProvider.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QKeyEvent;
class QFocusEvent;

class LineNumberArea; // Forward declaration

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(ISymbolProvider *provider = nullptr, QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setSymbolProvider(ISymbolProvider *provider);

signals:
    void goToDefinitionRequested(const QString &symbolName);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(QRectF rect);

private slots:
    void insertCompletion(const QString &completion);

private:
    QString textUnderCursor() const;
    void updateCompleter();

    QWidget *lineNumberArea;
    class PHPSyntaxHighlighter *highlighter;
    ISymbolProvider *symbolProvider;
    QCompleter *completer;
};

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
