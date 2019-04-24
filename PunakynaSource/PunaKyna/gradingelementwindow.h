#ifndef GRADINGELEMENTWINDOW_H
#define GRADINGELEMENTWINDOW_H

#include <QWidget>

class QVBoxLayout;
class QScrollArea;
class QLabel;
class QLineEdit;
template<typename T> class QVector;
#include <QString>
#include <QMap>

#include "binaryparser.h"
class GradingElement;
class CSVParser;
class Settings;


class GradingElementWindow : public QWidget
{
    Q_OBJECT

public:
    GradingElementWindow(Settings *set = nullptr, QWidget *parent = nullptr);
    ~GradingElementWindow();

    bool addFile(const QString &ident, CSVParser *file);
    void copyfile(CSVParser &file, QString to = QString());
    bool contains(const QString &ident);
    bool changeQuestion(const QString &ident);

    bool checkComments(BinaryParser::bins binary);
    void setPersonalComment(const QString &text);
    void setPersonalPoints(const QString &points);
    bool lockComments(bool lock);

    void createControl();
    void autoCheck(const CSVParser &text, const QStringList &answer,
                   const QStringList &question, bool curr = false);


    void saveWork();

    BinaryParser::bins getCommented(bool autoc = false);
    bool isConflicting();
    QString getPersonalComment();
    QString getPersonalPoints();
    QString getCommentText(int comment, QString of = QString());
    int getCommentValue(int comment, QString of = QString());
    int getMaxPoints(QString of = QString());
    int getDivider(QString of = QString());

    bool keyPress(const QString &key);

    void editComments();
    void removeComment(unsigned comment);
    void moveComments(QVector<int> &newOrder);

signals:
    void pointsChanged(short);
    void commentRemoved(unsigned);
    void commentsMoved(QVector<int> &newOrder);

public slots:
    GradingElement* on_addElement();
    short on_elementCheck(unsigned i, bool check);
    void on_elementChange(unsigned i, QString value,
                          int column);
    int on_maxPointsChange(const QString &points);
    int on_DividerChange(const QString &points);
    void on_freePointChange(const QString &points);

private: //functions
    struct FileStore
    {
        explicit FileStore(CSVParser *fl = nullptr);
        ~FileStore();

        CSVParser *file;
        QWidget *widg;
        QVector<GradingElement *> comms;
    };

    struct Checker
    {
        explicit Checker();
        void newLine();
        void endLine();
        void checkLine();

        void AND();
        void OR();

        bool truth;

    private:
        QVector<bool> truthAt;
        QVector<bool> ANDing;
        QVector<bool> ORing;
    };

    void importComments(FileStore *from);
    void generateNewLayout();
    GradingElement *addElement(FileStore *store);

    void fillDataRow(QStringList &d);

    void contextMenuEvent(QContextMenuEvent *e);

private:
    Settings *_settings;

    QMap<QString, FileStore*> _fileFolder;
    QString _current;

    QScrollArea *_area;
    QLabel *_pointCounter;
    short _points;
    QLineEdit *_maxPoints;
    QLineEdit *_divider;
    QLineEdit *_freeComment;
    QLineEdit *_freePoints;

    BinaryParser::bins _commented;
    BinaryParser::bins _autoCommented;
    bool _autoConflicting;
};

#endif // GRADINGELEMENTWINDOW_H
