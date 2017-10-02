#include <QTextDocument>
#include <QTextCharFormat>

#include "highlight.h"
#include "gl_lang_compiler.h"
#include "gl_lang_parser.h"
#ifndef YYSTYPE
#define YYSTYPE GL_LANG_STYPE
#endif
#ifndef YYLTYPE
#define YYLTYPE GL_LANG_LTYPE
#endif
#include "gl_lang_scanner.h"

#include "constant.h"

using Demo::GL::Compiler;
using Demo::Symbol;
using Demo::Function;
using Demo::Constant;
using Demo::GL::LocationType;
using Demo::GL::ValueType;
using Demo::GL::Compiler;

Highlight::Highlight(Compiler* c, QTextDocument* parent):
    QSyntaxHighlighter(parent),
    mCompiler(c),
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
    mFormats[FROM] = mReserved;
    mFormats[IMPORT] = mReserved;
    mFormats[INT] = mNumeric;
    mFormats[FLOAT] = mNumeric;

}

void Highlight::highlightBlock(const QString &text) {

    QString parsed = text;
    int pshift = 0;
    if (previousBlockState() == 1) {
        parsed = R"(")" + text;
        pshift = -1;
    }
    setCurrentBlockState(0);

    gl_lang_lex_init(&mScanner);
    YY_BUFFER_STATE buf = gl_lang__scan_string(parsed.toUtf8().data(), mScanner);

    int text_start = 0;
    int text_length = 0;

    LocationType loc;
    gl_lang_set_lloc(&loc, mScanner);
    ValueType val;
    gl_lang_set_lval(&val, mScanner);
    int token = gl_lang_lex(&val, &loc, mScanner);
    while (token > 0) {
        int token_len = gl_lang_get_leng(mScanner);
        QString token_text(gl_lang_get_text(mScanner));
        if (currentBlockState() == 1) {
            text_length += token_len;
            if (token == ENDSTRING) {
                setCurrentBlockState(0);
                setFormat(text_start, text_length, mText);
            }
        } else {
            if (token == BEGINSTRING) {
                setCurrentBlockState(1);
                text_start = loc.pos  + pshift;
                text_length = token_len;
                if (text_start < 0) {
                    text_start = 0;
                    text_length = 0;
                }
            } else {
                if (mFormats.contains(token)) {
                    setFormat(loc.pos + pshift, token_len, mFormats[token]);
                } else if (token == ID && mCompiler->hasSymbol(token_text)) {
                    const Symbol* sym = mCompiler->symbol(token_text);
                    const Function* fun = dynamic_cast<const Function*>(sym);
                    if (fun) {
                        setFormat(loc.pos + pshift, token_len, mFunction);
                    } else {
                        const Constant* con = dynamic_cast<const Constant*>(sym);
                        if (con) {
                            setFormat(loc.pos + pshift, token_len, mConstant);
                        }
                        // TODO: variables
                    }
                }
            }
        }
        token = gl_lang_lex(&val, &loc, mScanner);
    }

    gl_lang__delete_buffer(buf, mScanner);


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

Highlight::~Highlight() {
    gl_lang_lex_destroy(mScanner);
}
