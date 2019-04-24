#ifndef TEXTPARSER_H
#define TEXTPARSER_H

#include <QStringList>
#include <QRegExp>

class TextParser
{

public:

    enum Status
    {
        Done, Err_EmptyTemplate, Err_FaultyText
    };

    static Status parseText(QStringList &res,
                            const QString &text,
                            const QString &templ,
                            QString sep = "#TEKSTI\\w*#");

    static QStringList testTemplate(const QString &templ,
                             QString sep = "#TEKSTI\\w*#");

private:
    TextParser(const QString &templ, QString sep);

    Status parse(const QString &text);
    QStringList getres(){return _result;}
    QStringList getCaps(){return _caps;}

private:
    QString _templ;
    QRegExp _capText;

    QStringList _result;
    QStringList _caps;

};

#endif // TEXTPARSER_H
