#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QString;

class BrowserWindow;
class BrowserModel;
struct Indexer;
class GradingWindow;
class DisplayWidget;
class Settings;
class CSVParser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Settings *set, QWidget *parent = nullptr);
    ~MainWindow();

    bool selectExamDir(const QString &folder, QString type,
                       QString fileext = QString());

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

    bool hasOpenExam();

private:
    void showMenus(int type);
    bool prepareSelectedExam(QString folder);

    bool search_pdf2text();

    QString createText(const QString &fileName);

    CSVParser* fitTemplate(Indexer &ind);

public slots:
    void on_browserSelect(Indexer &ind);
    void on_browserBadEntry(Indexer &ind);

    void on_stateChange(const QString &question, const QString &answer, int state);
    void on_lockCommented();

    void on_selectExam();
    void on_exportToGrading();
    void exportToGrading(QString fileName);
    void on_save();

    void on_checkQuestionNumbering();

    void on_auto_one();
    void on_auto_some();
    void on_auto_all();

    void on_settings();
    void on_about();

protected:
    void keyPressEvent(QKeyEvent *e);



private:
    const char SetSep = '&';

    Settings *_settings;

    BrowserWindow *_testtree;
    BrowserModel *_testmodel;
    DisplayWidget *_display;
    GradingWindow *_grading;

    QMenu *_fileMenu;
    QMenu *_ExamMenu;
    QMenu *_LockMenu;
    QMenu *_AutoMenu;

    QAction *_GradeAction;
    QAction *_NumbAction;
};

#endif // MAINWINDOW_H
