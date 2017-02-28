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

#include "gl_lang_types.h"
#include "gl_lang_scanner.h"

}

#include "gl_lang.h"

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

    gl_lang_lex_destroy();
    QString parsed = text;
    int pshift = 0;
    if (previousBlockState() == 1) {
        parsed = "\"" + text;
        pshift = -1;
    }
    setCurrentBlockState(0);
    gl_lang__scan_string(parsed.toUtf8().data());

    int text_start = 0;
    int text_length = 0;

    int token = gl_lang_lex();
    while (token > 0) {
        if (currentBlockState() == 1) {
            text_length += gl_lang_leng;
            if (token == ENDSTRING) {
                setCurrentBlockState(0);
                setFormat(text_start, text_length, mText);
            }
        } else {
            if (token == BEGINSTRING) {
                setCurrentBlockState(1);
                text_start = gl_lang_lloc.pos  + pshift;
                text_length = gl_lang_leng;
                if (text_start < 0) {
                    text_start = 0;
                    text_length = 0;
                }
            } else {
                if (mFormats.contains(token)) {
                    setFormat(gl_lang_lloc.pos + pshift, gl_lang_leng, mFormats[token]);
                } else if (token == ID && Parser::Symbols().contains(gl_lang_text)) {
                    Function* fun = dynamic_cast<Function*>(Parser::Symbols()[gl_lang_text]);
                    if (fun) {
                        setFormat(gl_lang_lloc.pos + pshift, gl_lang_leng, mFunction);
                    } else {
                        Constant* con = dynamic_cast<Constant*>(Parser::Symbols()[gl_lang_text]);
                        if (con) {
                            setFormat(gl_lang_lloc.pos + pshift, gl_lang_leng, mConstant);
                        }
                    }
                }
            }
        }
        token = gl_lang_lex();
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
