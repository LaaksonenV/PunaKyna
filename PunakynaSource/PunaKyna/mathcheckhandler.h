#ifndef MATHCHECKHANDLER_H
#define MATHCHECKHANDLER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QProcess>


//#define DECOMMENTER "MCTextAreas"
#define DECOMMENTER "MCDecomment"

class QProcess;

class Settings;

class MathCheckHandler : public QObject
{
    Q_OBJECT
public:
    enum ProcessState
    {
        State_Null, State_Ready, State_Decommenting_Started, State_Decommenting,
        State_Decommentin_Finished, State_Checking_Started, State_Checking, State_Finished
    };

    MathCheckHandler(Settings *set, QObject *parent = nullptr);

    QString getCSS();

    bool isAvailable();

    bool textInput(const QString &text);

    QString waitForOutput (const QString &text);

public slots:
    void processStarted();
    void processFinished(int);

signals:
    void textOutput(QString text);

private: // functions
    void decommentMathCheck();
    void callMathCheck();

private slots:
    void on_Proc_err(QProcess::ProcessError err);

private:
    Settings *_settings;

    int _programState;

    QString _decommenterLoc;
    QString _mathCheckLoc;

    QProcess *_process;

    QByteArray _inputOutput;
};

#endif // MATHCHECKHANDLER_H
