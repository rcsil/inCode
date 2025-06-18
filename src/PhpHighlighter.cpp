#include "PhpHighlighter.h"

PhpHighlighter::PhpHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    variableFormat.setForeground(QColor("#d19a66"));  // cor laranja clara
    variableFormat.setFontWeight(QFont::Bold);
}

void PhpHighlighter::highlightBlock(const QString &text) {
    QRegularExpression varPattern(R"(\$[a-zA-Z_][a-zA-Z0-9_]*)");
    QRegularExpressionMatchIterator i = varPattern.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        setFormat(match.capturedStart(), match.capturedLength(), variableFormat);
    }
}