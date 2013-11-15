#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QMap>


class QTextDocument;

class Highlight : public QSyntaxHighlighter
{
    Q_OBJECT

public:

    Highlight(QTextDocument* parent = 0);

protected:

    void highlightBlock(const QString &text);

private:
    typedef QMap<int, QTextCharFormat> FormatMap;

    QRegExp mCommentExp;
    QTextCharFormat mComment;
    QTextCharFormat mReserved;
    QTextCharFormat mNumeric;
    QTextCharFormat mFunction;
    QTextCharFormat mConstant;
    QTextCharFormat mText;
    FormatMap mFormats;
};


#endif // HIGHLIGHT_H
