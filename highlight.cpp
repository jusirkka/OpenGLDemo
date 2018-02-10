#include <QTextDocument>
#include <QTextCharFormat>
#include <QTimer>

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
#include "variable.h"
#include "typedef.h"

using Demo::GL::Compiler;
using Demo::Symbol;
using Demo::Function;
using Demo::Constant;
using Demo::GL::LocationType;
using Demo::GL::ValueType;
using Demo::GL::Compiler;
using Demo::Variable;
using Demo::Typedef;

Highlight::Highlight(Compiler* c, QTextDocument* parent):
    QSyntaxHighlighter(parent),
    mCompiler(c),
    mCommentExp("//[^\n]*")
{

    gl_lang_lex_init(&mScanner);

    mComment.setForeground(Qt::gray);
    mReserved.setForeground(QColor("#808000"));
    mNumeric.setForeground(QColor("#000080"));
    // a hack
    QFont font("DejaVu Sans Mono", 14, QFont::Bold);
    mFunction.setFont(font);
    mFunction.setFontWeight(QFont::Bold);
    mConstant.setForeground(QColor("#800080"));
    mVariable.setFontItalic(true);
    mType.setForeground(QColor("#800080"));
    mText.setForeground(QColor("#008000"));

    mFormats[SHARED] = mReserved;
    mFormats[EXECUTE] = mReserved;
    mFormats[FROM] = mReserved;
    mFormats[IMPORT] = mReserved;
    mFormats[WHILE] = mReserved;
    mFormats[ENDWHILE] = mReserved;
    mFormats[IF] = mReserved;
    mFormats[ELSE] = mReserved;
    mFormats[ENDIF] = mReserved;
    mFormats[ELSIF] = mReserved;
    mFormats[ARRAY] = mReserved;
    mFormats[RECORD] = mReserved;
    mFormats[OF] = mReserved;
    mFormats[TYPE] = mReserved;
    mFormats[VAR] = mReserved;

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
                    auto sym = mCompiler->symbol(token_text);
                    auto fun = dynamic_cast<const Function*>(sym);
                    if (fun) {
                        setFormat(loc.pos + pshift, token_len, mFunction);
                    } else {
                        auto con = dynamic_cast<const Constant*>(sym);
                        if (con) {
                            setFormat(loc.pos + pshift, token_len, mConstant);
                        } else {
                            auto var = dynamic_cast<const Variable*>(sym);
                            if (var) {
                                setFormat(loc.pos + pshift, token_len, mVariable);
                            } else {
                                auto typ = dynamic_cast<const Typedef*>(sym);
                                if (typ) {
                                    setFormat(loc.pos + pshift, token_len, mType);
                                }
                            }
                        }
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
