#include "CodeEditor.h"
#include "PHPSyntaxHighlighter.h"

#include <QPainter>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QMouseEvent>
#include <QTextCursor>
#include <QDebug>
#include <QFontDatabase>
#include <QStringListModel>
#include <QScrollBar>

CodeEditor::CodeEditor(ISymbolProvider *provider, QWidget *parent)
    : QPlainTextEdit(parent), symbolProvider(provider), completer(new QCompleter(this))
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    // Set a modern monospaced font
    QFont font;
    font.setFamily("Fira Code");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);

    highlighter = new PHPSyntaxHighlighter(document());

    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
    updateCompleter();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int width = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return width;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(QRectF rect)
{
    if (rect.contains(viewport()->rect())) {
        update(0, rect.y(), lineNumberArea->width(), rect.height());
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor("#2c313a");

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor("#21252b"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#5C6370"));
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        QTextCursor cursor = cursorForPosition(event->pos());
        cursor.select(QTextCursor::WordUnderCursor);
        QString word = cursor.selectedText();
        if (!word.isEmpty()) {
            emit goToDefinitionRequested(word);
        }
    }
    QPlainTextEdit::mousePressEvent(event);
}

void CodeEditor::mouseMoveEvent(QMouseEvent *event)
{
    // Optional: Change cursor to hand when Ctrl is pressed over a symbol
    if (event->modifiers() & Qt::ControlModifier) {
        QTextCursor cursor = cursorForPosition(event->pos());
        cursor.select(QTextCursor::WordUnderCursor);
        QString word = cursor.selectedText();
        if (!word.isEmpty()) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::IBeamCursor);
        }
    } else {
        setCursor(Qt::IBeamCursor);
    }
    QPlainTextEdit::mouseMoveEvent(event);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (completer && completer->popup()->isVisible()) {
        switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            event->ignore();
            return; // let completer handle these
        default:
            break;
        }
    }

    QPlainTextEdit::keyPressEvent(event);

    QString prefix = textUnderCursor();
    if (!prefix.isEmpty()) {
        completer->setCompletionPrefix(prefix);
        QRect rect = cursorRect();
        rect.setWidth(completer->popup()->sizeHintForColumn(0)
                       + completer->popup()->verticalScrollBar()->sizeHint().width());
        completer->complete(rect);
    } else {
        completer->popup()->hide();
    }
}

void CodeEditor::focusInEvent(QFocusEvent *event)
{
    if (completer)
        completer->setWidget(this);
    QPlainTextEdit::focusInEvent(event);
}

void CodeEditor::insertCompletion(const QString &completion)
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.insertText(completion);
    setTextCursor(cursor);
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}

void CodeEditor::setSymbolProvider(ISymbolProvider *provider)
{
    symbolProvider = provider;
    updateCompleter();
}

void CodeEditor::updateCompleter()
{
    if (!completer)
        return;
    QStringList words;
    if (symbolProvider)
        words = symbolProvider->allSymbols();
    completer->setModel(new QStringListModel(words, completer));
}

