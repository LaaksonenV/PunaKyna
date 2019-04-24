#ifndef ENUMSCONSTS_H
#define ENUMSCONSTS_H

#include <QCoreApplication>

class QString;
class QStringList;
#include <QMap>

namespace GradingState
{
enum GradingStates : int
{
    UnTouched, Viewed, Conflicting, AutoCommented, Commented, Empty, Wrong,
    Locked, Sent, NullState
};
inline QString stateToChar(int state)
{
    if (state == UnTouched)
        return "";
    else if (state == Viewed)
        return QCoreApplication::tr("Katsottu");
    else if (state == Conflicting)
        return QCoreApplication::tr("TARKISTETTAVA");
    else if (state == AutoCommented)
        return QCoreApplication::tr("KoneTarkastus");
    else if (state == Commented)
        return QCoreApplication::tr("Tarkastettu");
    else if (state == Empty)
        return QCoreApplication::tr("EiPalautusta");
    else if (state == Wrong)
        return QCoreApplication::tr("VääräFormaatti");
    else if (state == Locked)
        return QCoreApplication::tr("Valmis");
    else if (state == Sent)
        return QCoreApplication::tr("Arkistoitu");
    return QCoreApplication::tr("NULL");
}
inline GradingStates charToState(const QString &c)
{
    if (c == QCoreApplication::tr("Katsottu"))
        return Viewed;
    else if (c == QCoreApplication::tr("TARKISTETTAVA"))
        return Conflicting;
    else if (c == QCoreApplication::tr("KoneTarkastus"))
        return AutoCommented;
    else if (c == QCoreApplication::tr("Tarkastettu"))
        return Commented;
    else if (c == QCoreApplication::tr("EiPalautusta"))
        return Empty;
    else if (c == QCoreApplication::tr("VääräFormaatti"))
        return Wrong;
    else if (c == QCoreApplication::tr("Valmis"))
        return Locked;
    else if (c == QCoreApplication::tr("Arkistoitu"))
        return Sent;
    else if (c == "")
        return UnTouched;
    return NullState;
}
}

namespace BrowserMetaData
{
enum Meta
{
    Meta_DisplayName, Meta_IdName, Meta_NULLData
};
}

namespace ExamFile
{
enum CommentCell
{
    CommentCell_Ids, CommentCell_Free, CommentCell_Points, CommentCell_By,
    CommentCell_Status, CommentCell_NullCell
};
}

namespace QuestionFile
{
enum MetaRow
{
    Meta_Points, Meta_Divider, Meta_AutoControl, Meta_NoControl
};

enum CommentRow
{
    CommentRow_Id, CommentRow_Label, CommentRow_Comment,
    CommentRow_Points, CommentRow_By, CommentRow_Keys, CommentRow_Size
};
}

namespace Printing
{
const QString separator = ";;";
const QString smallseparator = ":";
}

inline QString bool2String(bool b)
{
    if (b)
        return "true";
    return "false";
}

inline bool string2Bool(const QString &s)
{
    if (s == "true")
        return true;
    return false;
}

class Settings
{
public:

    enum Section : int
    {
        Section_NoSection, Section_Window, Section_Folders, Section_User
        , Section_EXAM, Section_DEF
    };


    Settings(char *path);
    ~Settings();

    /*
     * Shows dialog for editing the settings on the fly
     */
    //void editSettings();

    void setValue(Section section, const QString &key, const QString &value);
    QString getValue(Section section, const QString &key);

    void addMultiValue(Section section, const QString &key,
                       const QString &value);
    void setMultiValue(Section section, const QString &key,
                       const QStringList &value);
    QStringList getMultiValue(Section section, const QString &key);

    void writeDebug(const QString &text);
    void writeError(const QString &text);
    void printOnError();

    QString appPath(){return _appPath;}

private:
    Settings(const Settings&){}
    Settings& operator=(const Settings&) = delete;

    QString sectionToString(Section section);
    Section stringToSection(const QString &section);

    void readSettings();
    void writeSettings();
    void writeDefaults();

private:
    QString _fileName;

    QMap<Section, QMap<QString, QString>*> _settings;
    QStringList _debug;
    bool _debugging;

    QString _appPath;


};
#endif // ENUMSCONSTS_H
