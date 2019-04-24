#ifndef UIGRADE_H
#define UIGRADE_H

/*
 * UIGrade pitää yllä valitulle tehtävälle annettuja mahdollisia
 * kommentteja ja pisteytyksiä, ja päivittää annetut arvostelut
 * listaukseen.
 *
 * Arvosteluihin GradingElement, jota voi muokata käytön aikana,
 * ja tallentaa tehtävien pisteet vasta kun käyttäjä on valmis.
 * Siihen mennessä vastauksen arvosteluun toimii elementin id
*/

#include <QWidget>

class QString;
class QStringList;
class QVBoxLayout;
class QPushButton;

class Settings;
class GradingElementWindow;
class GradingPrinter;
class CSVParser;

#include <QMap>
/*
 * Widget that holds the exams .csv and is in charge of everything
 * that happens to it and it is used for.
 * Also contains the widgets that hold the question .csvs,
 * and has the buttons for signaling browsing.
 */
class GradingWindow : public QWidget
{
    Q_OBJECT
public:

    explicit GradingWindow(Settings *set, QWidget *parent = nullptr);
    ~GradingWindow();

    QString checkExamFile(const QString &file, bool *found = nullptr);
    bool setExamFile(const QString &filePath,
                     QMap<QString, int> questions);
    bool generateExamFile(const QString &filePath, QMap<QString,
                          int> questions);
private:
    QString searchFilePath(const QString &file, QChar type,
                                   bool *found = nullptr);
    void parseStatuses();

    void removeComment(unsigned comment);
    void moveComments(QVector<int> &newOrder);

public:

    // NOT IMPLEMENTED
    void importExamFile(const QString &file);

    bool changeGradeable(const QString &question, const QString &answer);
    void badEntry(const QString &question, const QString &answer,
                  int type);

    void changeLocked(bool lock);
    void lockCurrent(bool lock);
    void exportToGrading(QString fileName);

    bool keyPress(const QString & key);

    void checkQuestionNumbers();

    void saveWork();
    void saveAllWork();

    void autoCheckCurrent(const CSVParser &text);
    void autoCheck(const CSVParser &text, const QStringList &question,
                   const QStringList &answer);
    void editCurrentAuto();

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
protected:
    void contextMenuEvent(QContextMenuEvent *e);    
protected slots:
    void on_copyComments();

private:
    void clearGradeable();
    void fillDataRow(QStringList &d);

signals:
    void nextAnswer();
    void prevAnswer();

    void stateChanged(const QString &question, const QString &answer, int state);

private:
    Settings *_settings;
    CSVParser *_examFile;

    GradingElementWindow *_element;
    GradingPrinter *_printer;

    QVBoxLayout *_lay;
    QPushButton *_lockButton;

    int _fileRow;
    int _fileColumn;
};

#endif // UIGRADE_H
