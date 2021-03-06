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

/* static const char *const tokens[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "CHAR", "ID", "SHARED",
  "EXECUTE", "FROM", "IMPORT", "WHILE", "ENDWHILE", "IF", "ELSE", "ELSIF",
  "ENDIF", "ARRAY", "OF", "UNK", "BEGINSTRING", "ENDSTRING", "SEP", "TYPE",
  "VAR", "RECORD", "'.'", "'<'", "'>'", "EQ", "NE", "LE", "GE", "'+'",
  "'-'", "OR", "BOR", "'*'", "'/'", "AND", "BAND", "NEG", "'!'", "','",
  "'='", "'('", "')'", "':'", "'@'", "'['", "']'", "$accept", "input",
  "elements", "element", "import_from", "symbols", "typedecl", "typedef",
  "members", "member", "memberseq", "membertype", "declaration",
  "variables", "typespec", "assignment", "declaration_and_assignment",
  "statement", "rhs", "cond_rhs_seq", "cond_rhs", "simple_rhs", "guard",
  "expression", "comp_op", "terms", "add_op", "factors", "mul_op",
  "factor", "unary_op", "refpath", "refpathitem", "member_op",
  "arrayitems", "arrayseq", "recorditems", "text", "chars", "parameters",
  "arglist"
}; */

void Highlight::highlightBlock(const QString &text) {

    setCurrentBlockState(previousBlockState());

    if (text.trimmed().isEmpty()) return;

    gl_lang_lex_init(&mScanner);

    int text_start = 0;
    int text_length = 0;

    LocationType loc;
    gl_lang_set_lloc(&loc, mScanner);
    ValueType val;
    gl_lang_set_lval(&val, mScanner);

    if (currentBlockState() == 1) {
//        qCDebug(OGL) << "INSTRING";
//        qCDebug(OGL) << text;
        gl_lang__scan_string("\"", mScanner);
        for (int t = gl_lang_lex(&val, &loc, mScanner); t > 0; t = gl_lang_lex(&val, &loc, mScanner)) {}
//    } else {
//        qCDebug(OGL) << "INITIAL";
//        qCDebug(OGL) << text;
    }

    gl_lang__scan_string(text.toUtf8().data(), mScanner);

    for (int token = gl_lang_lex(&val, &loc, mScanner);
         token > 0;
         token = gl_lang_lex(&val, &loc, mScanner)) {

        int token_len = gl_lang_get_leng(mScanner);
        QString token_text(gl_lang_get_text(mScanner));

        // if (token >= 255) {
        //     qCDebug(OGL) << tokens[token - 255] << token_text;
        // } else {
        //     qCDebug(OGL) << token_text;
        //}


        if (currentBlockState() == 1) {
            text_length += token_len;
            if (token == ENDSTRING) {
                setCurrentBlockState(0);
                setFormat(text_start, text_length, mText);
            }
            continue;
        }

        if (token == BEGINSTRING) {
            setCurrentBlockState(1);
            text_start = loc.pos;
            text_length = token_len;
            if (text_start < 0) {
                text_start = 0;
                text_length = 0;
            }
            continue;
        }

        if (mFormats.contains(token)) {
            setFormat(loc.pos, token_len, mFormats[token]);
            continue;
        }

        if (token == ID && mCompiler->hasSymbol(token_text)) {
            auto sym = mCompiler->symbol(token_text);
            if (dynamic_cast<const Function*>(sym)) {
                setFormat(loc.pos, token_len, mFunction);
                continue;
            }
            if (dynamic_cast<const Constant*>(sym)) {
                setFormat(loc.pos, token_len, mConstant);
                continue;
            }
            if (dynamic_cast<const Variable*>(sym)) {
                setFormat(loc.pos, token_len, mVariable);
            }
            if (dynamic_cast<const Typedef*>(sym)) {
                setFormat(loc.pos, token_len, mType);
                continue;
            }
        }
    }

    gl_lang_lex_destroy(mScanner);


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
}
