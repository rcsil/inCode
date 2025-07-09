#include "PHPSyntaxHighlighter.h"
#include <QRegularExpression>

PHPSyntaxHighlighter::PHPSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keywords
    keywordFormat.setForeground(QColor("#C678DD"));
    QStringList keywordPatterns;
    keywordPatterns << "\b(abstract|and|array|as|break|callable|case|catch|class|clone|const|continue|declare|default|die|do|echo|else|elseif|empty|enddeclare|endfor|endforeach|endif|endswitch|endwhile|eval|exit|extends|final|for|foreach|function|global|goto|if|implements|include|include_once|instanceof|insteadof|interface|isset|list|namespace|new|or|print|private|protected|public|require|require_once|return|static|switch|throw|trait|try|unset|use|var|while|xor|yield)\b";
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Classes
    classFormat.setForeground(QColor("#E5C07B"));
    rule.pattern = QRegularExpression("\b[A-Z][A-Za-z0-9_]+\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // Single-line comments
    singleLineCommentFormat.setForeground(QColor("#5C6370"));
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Multi-line comments
    multiLineCommentFormat.setForeground(QColor("#5C6370"));

    // Quotation
    quotationFormat.setForeground(QColor("#98C379"));
    rule.pattern = QRegularExpression("(\"|\[quote]).*?\\1");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Functions
    functionFormat.setForeground(QColor("#61AFEF"));
    rule.pattern = QRegularExpression("\b[A-Za-z0-9_]+(?=\\()\b");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}

void PHPSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf("/*");

    while (startIndex >= 0) {
        int endIndex = text.indexOf("*/", startIndex + 2);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + 2;
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf("/*", startIndex + commentLength);
    }
}