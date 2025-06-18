#include "Editor.h"

Editor::Editor(QWidget *parent) : QPlainTextEdit(parent) {
    setFont(QFont("Monospace", 11));
    setWordWrapMode(QTextOption::NoWrap);
}