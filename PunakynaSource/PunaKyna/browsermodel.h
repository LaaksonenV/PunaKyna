#ifndef BROWSERMODEL_H
#define BROWSERMODEL_H

#include <QObject>

class QString;
class QStringList;
#include <QVector>
#include <QMap>
class QDir;
#include <QRegularExpression>

class Settings;
class CSVParser;

struct Indexer
{
    int quest = -1;
    int answ = -1;
    int xtr = -1;

    void clear(){quest = -1;answ = -1;xtr = -1;}
};

class BrowserModel : public QObject
{
    Q_OBJECT

public:

    enum DataType
    {
        DisplayName, State, Identifier, Url, File, NullDS
    };

    enum DirectoryTypes
    {
        Type_Null, Type_EXAM, Type_Moodle
    };

    explicit BrowserModel(Settings *set, QObject *parent= nullptr);
    virtual ~BrowserModel();

    QString setExamDir(const QString &folder, DirectoryTypes type,
                       QStringList formats);
    QString setExamDir(const QString &folder, QString type,
                       QStringList formats);

    QMap<QString, int> getQuestionNames();
    QString getGradings();

    QString getText(Indexer of, DataType type) const;
    QStringList getMeta(Indexer of) const;

    bool setText(Indexer of, DataType type, const QString &to);
    Indexer getIndexOf(const QString &question, QString answer = QString());

    int indexCount(Indexer of);
    
    QStringList getExamTypes();

signals:
    void modelReady();
    void dataChanged(Indexer of, DataType slot);

private:
    struct Directory
    {
        QString dispName;
        QString idname;
        int state;
        QString url;

        QVector<Directory*> unders;

        QString file;

        Directory()
            : dispName(QString())
            , idname(QString())
            , state(0)
            , url(QString())
            , unders(QVector<Directory*>())
            , file(QString())
        {}
        Directory(const Directory *copy)
            : dispName(copy->dispName)
            , idname(copy->idname)
            , state(copy->state)
            , url(copy->url)
            , unders(QVector<Directory*>())
            , file(copy->file)
        {}
        void destroy();
    };

    void clear();
    bool ensureDir(QDir &dir);
    QMap<QString, int> searchQuestionNumbers();
    QString searchEXAMName();
    bool populateDirectories(QDir &dir, DirectoryTypes type);

    bool populateDirectories(QDir &dir, QString type);
    bool populateAll(QDir &dir, int row,
                     QMap<QString, Directory *> &Qs,
                     QStringList &ids, int metaRow);
    Directory *createBranch(int metarow, QStringList &ids, int depth);
    QString lookUpMeta(int row, int col, QStringList &ids);

    bool populateExam(QDir &dir);
    bool populateMoodle(QDir &dir);
    bool checkFormat(const QString &entry);

    bool changeState(Indexer of, int newState);

    Directory *getDir(Indexer of) const;

private:
    Settings *_settings;

    QString _infoName;
    Directory *_top;
    DirectoryTypes _dirType;
    QRegularExpression _formats;
    CSVParser *_dirForm;

};

#endif // BROWSERMODEL_H
