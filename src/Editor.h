#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>

class Editor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);
};

#endif // EDITOR_H
