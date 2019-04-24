#include "textparser.h"

TextParser::Status TextParser::parseText(QStringList &res,
                                         const QString &text,
                                         const QString &templ, QString sep)
{
    TextParser pr(templ, sep);
    Status st = pr.parse(text);
    res = pr.getres();
    return st;
}

QStringList TextParser::testTemplate(const QString &templ, QString sep)
{
    QRegExp rx("("+sep+")");
    int indx = -1;
    QStringList ret;

    while ((indx = rx.indexIn(templ,indx+1)) >= 0)
        ret << rx.cap(1);

    return ret;
}

TextParser::TextParser(const QString &templ, QString sep)
    : _templ(templ)
    , _capText(QRegExp("("+sep+")"))
    , _result(QStringList())
    , _caps(QStringList())
{
}

TextParser::Status TextParser::parse(const QString &text)
{
    int tempInd = 0, lngth = 0;
    int indText = 0, indTempl = 0;

    indTempl = _capText.indexIn(_templ,indTempl);
    lngth = indTempl;

    if (indTempl < 0)
        return Status::Err_EmptyTemplate;

    QStringRef sr(&_templ,tempInd,lngth);
    indText = text.indexOf(sr)+lngth;

    while (indTempl >= 0)
    {

        indTempl += _capText.cap(0).size();
        tempInd = indTempl;
        _caps << _capText.cap(1);
        indTempl = _capText.indexIn(_templ,indTempl);
        if (indTempl<0)
            lngth = -1;
        else
            lngth = indTempl - tempInd;

        sr = QStringRef(&_templ,tempInd,lngth);

        tempInd = indText;
        indText = text.indexOf(sr, indText);
        if (indText < 0)
        {
            _result << QString();
            return Status::Err_FaultyText;
            // Muut virhetarkistukset
        }

        lngth = indText - tempInd;
        _result << text.mid(tempInd, lngth);
        indText += lngth;
    }
    return Status::Done;
}
