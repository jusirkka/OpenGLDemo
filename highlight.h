#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QMap>

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
using yyscan_t = void *;
#endif


class QTextDocument;

namespace Demo {
namespace GL {
class Compiler;
}
}

class Highlight : public QSyntaxHighlighter
{
    Q_OBJECT

public:

    Highlight(Demo::GL::Compiler* compiler, QTextDocument* parent);
    ~Highlight() override;

protected:

    void highlightBlock(const QString &text) override;

private:

    using FormatMap = QMap<int, QTextCharFormat>;

    Demo::GL::Compiler* mCompiler;
    QRegExp mCommentExp;
    QTextCharFormat mComment;
    QTextCharFormat mReserved;
    QTextCharFormat mNumeric;
    QTextCharFormat mFunction;
    QTextCharFormat mConstant;
    QTextCharFormat mText;
    FormatMap mFormats;
    yyscan_t mScanner;
};


#endif // HIGHLIGHT_H
