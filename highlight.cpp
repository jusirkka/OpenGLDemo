#include <QTextDocument>
#include <QTextCharFormat>

#include "highlight.h"
#include "parser.h"
#include "constant.h"

using Demo::Parser;
using Demo::Symbol;
using Demo::Function;
using Demo::Constant;

extern "C"
{

#include "s_p_types.h"
#include "s_p.h"

}

#include "g_p.h"

Highlight::Highlight(QTextDocument* parent)
    :QSyntaxHighlighter(parent),
      mCommentExp("//[^\n]*")
{
    mComment.setForeground(Qt::gray);
    mReserved.setForeground(Qt::blue);
    mNumeric.setForeground(Qt::darkRed);
    mFunction.setFontWeight(QFont::Bold);
    mConstant.setForeground(Qt::darkMagenta);
    mText.setForeground(Qt::darkGreen);

    mFormats[VECTOR] = mReserved;
    mFormats[MATRIX] = mReserved;
    mFormats[TEXT] = mReserved;
    mFormats[NATURAL] = mReserved;
    mFormats[SHARED] = mReserved;
    mFormats[REAL] = mReserved;
    mFormats[EXECUTE] = mReserved;
    mFormats[INT] = mNumeric;
    mFormats[FLOAT] = mNumeric;

}

void Highlight::highlightBlock(const QString &text) {

    s_plex_destroy();
    QString parsed = text;
    int pshift = 0;
    if (previousBlockState() == 1) {
        parsed = "\"" + text;
        pshift = -1;
    }
    setCurrentBlockState(0);
    s_p_scan_string(parsed.toAscii().data());

    int text_start = 0;
    int text_length = 0;

    int token = g_plex();
    while (token > 0) {
        if (currentBlockState() == 1) {
            text_length += s_pleng;
            if (token == ENDSTRING) {
                setCurrentBlockState(0);
                setFormat(text_start, text_length, mText);
            }
        } else {
            if (token == BEGINSTRING) {
                setCurrentBlockState(1);
                text_start = g_plloc.pos  + pshift;
                text_length = s_pleng;
                if (text_start < 0) {
                    text_start = 0;
                    text_length = 0;
                }
            } else {
                if (mFormats.contains(token)) {
                    setFormat(g_plloc.pos + pshift, s_pleng, mFormats[token]);
                } else if (token == ID && Parser::Symbols().contains(s_ptext)) {
                    Function* fun = dynamic_cast<Function*>(Parser::Symbols()[s_ptext]);
                    if (fun) {
                        setFormat(g_plloc.pos + pshift, s_pleng, mFunction);
                    } else {
                        Constant* con = dynamic_cast<Constant*>(Parser::Symbols()[s_ptext]);
                        if (con) {
                            setFormat(g_plloc.pos + pshift, s_pleng, mConstant);
                        }
                    }
                }
            }
        }
        token = g_plex();
    }

    if (currentBlockState() == 1) {
        setFormat(text_start, text_length, mText);
    } else {
        int index = mCommentExp.indexIn(text);
        while (index >= 0) {
            int length = mCommentExp.matchedLength();
            setFormat(index, length, mComment);
            index = mCommentExp.indexIn(text, index + length);
        }
    }
}
