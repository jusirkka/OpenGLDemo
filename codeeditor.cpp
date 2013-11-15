/****************************************************************************
 **
 ** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 **     of its contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include <QtGui>
#include <QTimer>

#include "codeeditor.h"
#include "parser.h"
#include "runner.h"
#include "project.h"
#include "highlight.h"

using Demo::Parser;
using Demo::ParseError;
using Demo::Runner;
using Demo::Project;


CodeEditor::CodeEditor(Project* owner)
    :QPlainTextEdit(),
      mParseDelay(new QTimer(this)),
      mRunner(0),
      mErrorPos(-1)
{
    connect(this, SIGNAL(runnerReady()), owner, SLOT(runnerReady()));
    connect(this->document(), SIGNAL(modificationChanged(bool)), owner, SLOT(groupModified(bool)));

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    mParseDelay->setInterval(1000);
    connect(this, SIGNAL(textChanged()), mParseDelay, SLOT(start()));
    connect(mParseDelay, SIGNAL(timeout()), this, SLOT(parse()));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    mHighlight = new Highlight(document());
}

void CodeEditor::parse() {
    mParseDelay->stop();
    try {
        Parser::ParseIt(toPlainText());
        delete mRunner;
        mRunner = Parser::CreateRunner();
        mErrorPos = -1;
        emit runnerReady();
    } catch (ParseError& e) {
        mError = e.msg();
        mErrorPos = e.pos();
    }
    highlightCurrentLine();
}


int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
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



void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}



void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
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
    if (mErrorPos >= 0) {
        QTextEdit::ExtraSelection err;
        err.cursor = QTextCursor(document());
        err.cursor.movePosition(QTextCursor::End);
        int endpos = err.cursor.position();
        if (mErrorPos <= endpos) {
            err.cursor.setPosition(mErrorPos, QTextCursor::MoveAnchor);
            err.cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            // err.cursor.select(QTextCursor::WordUnderCursor);
            // qDebug() << "high" << err.cursor.anchor();
            // qDebug() << "high" << err.cursor.position();

            QColor under = QColor(Qt::red).darker(175);

            err.format.setUnderlineColor(under);
            err.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            err.format.setToolTip(mError);
            extraSelections.append(err);
        }
    }

    setExtraSelections(extraSelections);
}

bool CodeEditor::event(QEvent* ev) {
    if (ev->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast <QHelpEvent*>(ev);
        QTextCursor cursor = cursorForPosition(helpEvent->pos());
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        foreach(QTextEdit::ExtraSelection s, extraSelections()) {
            int pos = s.cursor.position();
            if (pos <= cursor.selectionEnd() && pos >= cursor.selectionStart()) {
                QToolTip::showText(helpEvent->globalPos(), s.format.toolTip());
            }
        }
        return true;
    }

    return QPlainTextEdit::event(ev);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
