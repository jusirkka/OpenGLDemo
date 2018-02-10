#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>


class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QTimer;
class QCompleter;

class LineNumberArea;

class Highlight;

namespace Demo {


namespace GL {
class Compiler;
class Completer;
}

class Project;
class Scope;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:

    using EditorVector = QVector<Demo::CodeEditor *>;

public:
    CodeEditor(const QString& name, Scope* globals, Project* owner, const QString& text);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    bool event(QEvent *e) override;

    bool hasRunError() const {return mRunErrorPos != -1;}
    bool hasCompileError() const {return mCompileErrorPos != -1;}

    void toggleAutoCompile(bool on);
    void complete();
    void rename(const QString& name);

    const QString& fileName();
    void setFileName(const QString&);

    GL::Compiler* compiler() const;

public slots:

    void compile();
    void run();

protected:

    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:

    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void insertCompletion(const QString& item);

signals:

    void compiled();
    void statusChanged();


private:

    QWidget* mLineNumberArea;
    QTimer* mCompileDelay;
    QString mRunError;
    QString mCompileError;
    int mCompileErrorPos;
    int mRunErrorPos;
    Highlight* mHighlight;
    GL::Compiler* mCompiler;
    GL::Completer* mCompleter;
    QString mPath;
};


class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), mCodeEditor(editor) {}

    QSize sizeHint() const override {
        return QSize(mCodeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        mCodeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *mCodeEditor;
};

} // namespace Demo

#endif
