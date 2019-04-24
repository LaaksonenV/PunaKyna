#include "settings.h"

#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QString>

Settings::Settings(char *path)
    : _fileName(QString(""))
    , _settings(QMap<Section, QMap<QString, QString>*>())
    , _debug(QStringList())
    , _debugging(false)
{
#ifdef Q_OS_MACOS
    _appPath = QString(path);
    _appPath = _appPath.section(QDir::separator(), 0, -5);
#else
    _appPath = QString::fromLatin1(path);
    _appPath = _appPath.section(QDir::separator(), 0, -2);
#endif

    QDir(_appPath).mkdir("settings");

    _fileName = _appPath + "/settings/settings.ini";

    readSettings();


    writeDefaults();
}

Settings::~Settings()
{
    writeSettings();
    if (_debugging)
        printOnError();
}


void Settings::setValue(Settings::Section section, const QString &key,
                        const QString &value)
{
    QMap<QString, QString> *sect;
    if (!_settings.contains(section))
    {
        sect = new QMap<QString, QString>();
        _settings.insert(section, sect);
    }
    else
        sect = _settings.value(section);
    sect->insert(key, value);
}

QString Settings::getValue(Settings::Section section,const QString &key)
{
    if (!_settings.contains(section))
        return "";
    QMap<QString, QString> *sect = _settings.value(section);
    return sect->value(key, "");
}

void Settings::addMultiValue(Settings::Section section, const QString &key,
                             const QString &value)
{
    QMap<QString, QString> *sect;
    QStringList original;
    if (!_settings.contains(section))
    {
        sect = new QMap<QString, QString>();
        _settings.insert(section, sect);
    }
    else
        sect = _settings.value(section);
    original = sect->value(key, "").split(Printing::separator, QString::SkipEmptyParts);
    if (original.contains(value))
        return;
    original << value;
    sect->insert(key, original.join(Printing::separator));
}

void Settings::setMultiValue(Settings::Section section, const QString &key,
                             const QStringList &value)
{
    QMap<QString, QString> *sect;
    if (!_settings.contains(section))
    {
        sect = new QMap<QString, QString>();
        _settings.insert(section, sect);
    }
    else
        sect = _settings.value(section);

    sect->insert(key, value.join(Printing::separator));
}

QStringList Settings::getMultiValue(Settings::Section section, const QString &key)
{
    if (!_settings.contains(section))
        return QStringList();
    QMap<QString, QString> *sect = _settings.value(section);
    return sect->value(key).split(Printing::separator, QString::SkipEmptyParts);
}

void Settings::writeDebug(const QString &text)
{
    _debug << text;
}
void Settings::writeError(const QString &text)
{
    QString err = "ERROR  ";
    err += text;
    _debug << text;
    _debugging = true;
}

void Settings::printOnError()
{
    QString fileName = QCoreApplication::applicationDirPath()
            # ifdef __APPLE__
                .section('/', 0,-4)
            # endif
                .append("/DebugLog#");

    fileName.append(QDateTime::currentDateTime().toString("HH;mm dd.MM.yy"));
    fileName.append(".txt");
    QFile file(fileName);
    if (!file.open(QFile::Text | QFile::WriteOnly | QFile::Truncate))
        return;

    QTextStream out (&file);

    foreach (QString s, _debug)
        out << s << endl;
    file.close();
}

QString Settings::sectionToString(Section section)
{
    switch (section)
    {
    case Section::Section_Folders:
        return "[Folders]";
    case Section::Section_Window:
        return "[Window]";
    case Section::Section_User:
        return "[Users]";
    case Section::Section_EXAM:
        return "[EXAM]";
    case Section::Section_DEF:
        return "[DEFAULTS]";
    case Section::Section_NoSection:
        return "[]";
    default:
        return "";
    }
}

Settings::Section Settings::stringToSection(const QString &section)
{
    if (section == "[Folders]")
        return Section::Section_Folders;
    if (section == "[Window]")
        return Section::Section_Window;
    if (section == "[Users]")
        return Section::Section_User;
    if (section == "[EXAM]")
        return Section::Section_EXAM;
    if (section == "[DEFAULTS]")
        return Section::Section_DEF;

    return Section::Section_NoSection;
}

void Settings::readSettings()
{
    QMap<QString, QString> *sect = nullptr;
    Section currentSection = Section::Section_NoSection;
    QFile sett(_fileName);
    if(sett.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream reading(&sett);
        QString line;
        while(!reading.atEnd())
        {
            line = reading.readLine();
            if (line.isEmpty() || line.startsWith(';'))
            {
                continue;
            }
            else if (stringToSection(line) != Section::Section_NoSection)
            {
                currentSection = stringToSection(line);
                if (!_settings.contains(Section::Section_NoSection))
                {
                    sect = new QMap<QString, QString>();
                    _settings.insert(currentSection, sect);
                }
                else
                    sect = _settings.value(currentSection);
            }
            else
            {
                if (sect == nullptr)
                {
                    currentSection = Section::Section_NoSection;
                    if (!_settings.contains(Section::Section_NoSection))
                    {
                        sect = new QMap<QString, QString>();
                        _settings.insert(currentSection, sect);
                    }
                    else
                        sect = _settings.value(currentSection);
                }
                sect->insert(line.section('=',0,0), line.section('=',1));
            }
        }
        sett.close();
    }
}

void Settings::writeSettings()
{
    QFile sett(_fileName);
    if (sett.open(QFile::WriteOnly | QFile::Text))
    {
        sett.flush();
        QTextStream writing(&sett);
        QMap<QString, QString> *value;
        foreach (Section sect, _settings.keys())
        {
            writing << sectionToString(sect) << '\n';
            value = _settings.value(sect);
            foreach (QString key, value->keys())
                writing << key << '=' << value->value(key) << '\n';
            endl(writing);
        }
        sett.close();
    }
}

void Settings::writeDefaults()
{
    if (getValue(Section_EXAM,"allow_commas").isEmpty())
        setValue(Section_EXAM,"allow_commas",bool2String(false));
    if (getValue(Section_EXAM,"replace_commas").isEmpty())
        setValue(Section_EXAM,"replace_commas","_");
    if (getValue(Section_DEF,"no_returns").isEmpty())
        setValue(Section_DEF,"no_returns",
                 QCoreApplication::tr("Tyhjä palautus."));
    if (getValue(Section_DEF,"wrong_format").isEmpty())
        setValue(Section_DEF,"wrong_format",
                 QCoreApplication::tr("Väärä palautuksen formaatti."));
    if (getValue(Section_EXAM, "print_cpts").isEmpty())
        setValue(Section_EXAM, "print_cpts",
                        bool2String(false));
    if (getValue(Section_EXAM, "print_qpts").isEmpty())
        setValue(Section_EXAM, "print_qpts",
                        bool2String(true));
    if (getValue(Section_DEF,"directory_type_csv").isEmpty())
        setValue(Section_DEF,"directory_type_csv",
                 QString(_appPath) + "/settings/DirTypes.csv");
    if (getValue(Section_DEF,"grading_csv").isEmpty())
        setValue(Section_DEF,"grading_csv",
                 QString(_appPath) + "/settings/Gradings.csv");
    if (getValue(Section_DEF,"extra_prints").isEmpty())
        setValue(Section_DEF,"extra_prints",
                 QString(_appPath) + "/settings/Prints.csv");

}
