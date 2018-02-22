#include <QtGui>
#include <QTimer>
#include <QToolTip>
#include <QScrollBar>

#include "codeeditor.h"
#include "gl_lang_compiler.h"
#include "gl_lang_completer.h"
#include "gl_lang_runner.h"
#include "project.h"
#include "highlight.h"

using namespace Demo;

CodeEditor::CodeEditor(const QString& name, Scope* globals, Project* owner, const QString& text):
    QPlainTextEdit(text),
    mCompileDelay(new QTimer(this)),
    mCompileErrorPos(-1),
    mRunErrorPos(-1),
    mCompiler(new GL::Compiler(name, globals, this)),
    mCompleter(new GL::Completer(globals, this))
{
    setObjectName(name);

    connect(this, SIGNAL(compiled()), owner, SLOT(scriptCompiled()));
    connect(this->document(), SIGNAL(modificationChanged(bool)), owner, SLOT(scriptModification_changed(bool)));
    connect(this, SIGNAL(statusChanged()), owner, SLOT(scriptStatus_changed()));

    mLineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    mCompileDelay->setInterval(700);
    toggleAutoCompile(owner->autoCompileEnabled());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    mHighlight = new Highlight(mCompiler, document());

    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void CodeEditor::toggleAutoCompile(bool on) {
    if (on) {
        connect(this, SIGNAL(textChanged()), mCompileDelay, SLOT(start()));
        connect(mCompileDelay, SIGNAL(timeout()), this, SLOT(compile()));
    } else {
        mCompileDelay->stop();
        disconnect(this, SIGNAL(textChanged()), mCompileDelay, SLOT(start()));
        disconnect(mCompileDelay, SIGNAL(timeout()), this, SLOT(compile()));
    }
}

void CodeEditor::rename(const QString& name) {
    setObjectName(name);
    mCompiler->setObjectName(name);
}

const QString& CodeEditor::fileName() {
    return mPath;
}

void CodeEditor::setFileName(const QString& path) {
    mPath = path;
}

GL::Compiler* CodeEditor::compiler() const {
    return mCompiler;
}

void CodeEditor::compile() {
    mCompileDelay->stop();
    int prevPos = mCompileErrorPos;
    QString prevMsg = mCompileError;
    try {
        mCompiler->compile(toPlainText());
        mCompileErrorPos = -1;
        emit compiled();
    } catch (GL::CompileError& e) {
        mCompileError = e.msg();
        mCompileErrorPos = e.pos();
    }
    if (prevPos != mCompileErrorPos || prevMsg != mCompileError) {
        highlightCurrentLine();
        emit statusChanged();
    }
}

void CodeEditor::run() {
    if (!mCompiler->ready()) return;
    int prevPos = mRunErrorPos;
    QString prevMsg = mRunError;
    try {
        mCompiler->run();
        mRunErrorPos = -1;
    } catch (RunError& e) {
        mRunError = e.msg();
        mRunErrorPos = e.pos();
    }

    if (prevPos != mRunErrorPos || prevMsg != mRunError) {
        highlightCurrentLine();
        emit statusChanged();
    }
}


void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::blue).lighter(175);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    int errpos = mRunErrorPos;
    if (mCompileErrorPos >= 0) errpos = mCompileErrorPos;

    if (errpos >= 0) {

        QTextEdit::ExtraSelection err;
        err.cursor = QTextCursor(document());
        setErrPos(err.cursor, errpos);

        QColor under = QColor(Qt::red).darker(175);
        QString errtip = (mCompileErrorPos >= 0) ? mCompileError : mRunError;
        err.format.setUnderlineColor(under);
        err.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        err.format.setToolTip(errtip);
        extraSelections.append(err);
    }


    setExtraSelections(extraSelections);
}

void CodeEditor::setErrPos(QTextCursor& cursor, int errpos) const {
    cursor.movePosition(QTextCursor::End);
    int endpos = cursor.position();

    if (errpos > endpos) errpos = endpos;

    cursor.setPosition(errpos, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    while (cursor.anchor() == cursor.position() && errpos > 0) {
        errpos -= 1;
        cursor.setPosition(errpos, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    }

    if (cursor.anchor() == cursor.position()) {
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }

}

bool CodeEditor::event(QEvent* ev) {
    if (ev->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(ev);
        QTextCursor cursor = cursorForPosition(helpEvent->pos());
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        for (auto& s: extraSelections()) {
            int pos = s.cursor.position();
            if (pos <= cursor.selectionEnd() && pos >= cursor.selectionStart()) {
                QToolTip::showText(helpEvent->globalPos(), s.format.toolTip());
            }
        }
        return true;
    }

    return QPlainTextEdit::event(ev);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 3;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 4 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}



void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}



void CodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy) {
        mLineNumberArea->scroll(0, dy);
    } else {
        mLineNumberArea->update(0, rect.y(), mLineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}



void CodeEditor::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent* e) {
    if (!mCompleter->popupVisible()) {
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        e->ignore();
        return; // let the completer do default behavior
    default:
        break;
    }

    const bool supportedMod = e->modifiers() & Qt::ShiftModifier;
    if (supportedMod && e->text().isEmpty()) return;

    bool unsupportedMod = (e->modifiers() != Qt::NoModifier) && !supportedMod;

    if (unsupportedMod || e->text().isEmpty()) {
        mCompleter->hidePopup();
        return;
    }

    QPlainTextEdit::keyPressEvent(e);

    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    mCompleter->updatePopup(tc.selectedText());
}

void CodeEditor::complete() {
    if (mCompleter->popupVisible()) return;
    QStringList completions;
    try {
        mCompleter->complete(toPlainText(), textCursor().position());
    } catch (GL::CompleterException& e) {
        completions = e.completions();
    }

    if (completions.isEmpty()) return;

    if (completions.length() == 1) {
        insertCompletion(completions.first());
        return;
    }

    mCompleter->popupCompletions(completions);
}

void CodeEditor::insertCompletion(const QString& item) {
    QTextCursor tc = textCursor();
    int pos = tc.position();
    tc.select(QTextCursor::WordUnderCursor);
    // qCDebug(OGL) << "insertCompletion #1" << tc.selectedText();
    if (!item.startsWith(tc.selectedText())) {
        tc.setPosition(pos - 1);
        tc.select(QTextCursor::WordUnderCursor);
        // qCDebug(OGL) << "insertCompletion #2" << tc.selectedText();
        if (!item.startsWith(tc.selectedText())) {
            // qCDebug(OGL) << "insertCompletion: no can do";
            return;
        }
    }
    tc.insertText(item);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(mLineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, mLineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }


    if (mRunErrorPos < 0) return;



    QTextCursor cursor = QTextCursor(document());
    setErrPos(cursor, mRunErrorPos);
    block = cursor.block();
    if (!block.isValid() || !block.isVisible()) return;

    top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    if (top > event->rect().bottom()) return;

    QIcon err = QIcon::fromTheme("error");
    err.paint(&painter, 0, top, mLineNumberArea->width(), fontMetrics().height(), Qt::AlignLeft, QIcon::Normal, QIcon::On);
}
