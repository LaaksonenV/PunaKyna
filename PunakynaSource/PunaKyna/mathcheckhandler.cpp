#include "mathcheckhandler.h"

#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QByteArray>
#include <QMessageBox>

#include "settings.h"

MathCheckHandler::MathCheckHandler(Settings *set, QObject *parent)
    : QObject(parent)
    , _settings(set)
    , _programState(State_Null)
    , _decommenterLoc("")
    , _mathCheckLoc("")
    , _process(new QProcess(this))
    , _inputOutput("")
{
    QDir dir(_settings->appPath());

    _mathCheckLoc = _settings->getValue(Settings::Section::Section_Folders, "mathcheck");

    if (_mathCheckLoc == "")
    {
        dir.cd("MathCheck");
        _mathCheckLoc = dir.absoluteFilePath("MathCheck");

    }

    if (!dir.exists(_mathCheckLoc))
    {
        QMessageBox::information(NULL, "", "MathCheck -ohjelmistoa ei löytynyt ennaltamääritetyistä sijainnista.\n"
                                 "Valitse MathCheck tiedoston sijainti.\n\n"
                                 "Tämä ikkuna ponnahti koska Macin natiivit dialogit ei kaikessa"
                                 "viisaudessaan näytä otsikkoriviä. Ilmoitus poistuu myöhemmässä versiossa.",
                                 QMessageBox::Ok);
        _mathCheckLoc = QFileDialog::getOpenFileName(NULL, tr("Valitse MathCheck -tiedosto"),
                      dir.absolutePath(),"MathCheck");
        if (_mathCheckLoc != "")
            _settings->setValue(Settings::Section_Folders, "mathcheck", _mathCheckLoc);
        else
            return;
    }
    else
    {
        _settings->setValue(Settings::Section_Folders, "mathcheck", _mathCheckLoc);
    }

    _decommenterLoc = _settings->getValue(Settings::Section::Section_Folders, "decommenter");

    if (_decommenterLoc == "")
    {
        dir.cd("MathCheck");
        _decommenterLoc = dir.absoluteFilePath(DECOMMENTER);
    }

    if (!dir.exists(_decommenterLoc))
        dir = _mathCheckLoc.section(QDir::separator(),0,-2);

    if (!dir.exists(_decommenterLoc))
    {
        QMessageBox::information(NULL, "", "MathCheck -ohjelmistoa ei löytynyt ennaltamääritetyistä sijainnista.\n"
                                           "Valitse MathCheck " DECOMMENTER " tiedoston sijainti.\n\n"
                                           "Tämä ikkuna ponnahti koska Macin natiivit dialogit ei kaikessa"
                                           "viisaudessaan näytä otsikkoriviä. Ilmoitus poistuu myöhemmässä versiossa.",
                                 QMessageBox::Ok);
        _decommenterLoc = QFileDialog::getOpenFileName(NULL, tr("Valitse MathCheck decommentointi -tiedosto"),
                                                    dir.absolutePath(),"MCDecomment");
        if(_decommenterLoc != "")
            _settings->setValue(Settings::Section_Folders, "decommenter", _decommenterLoc);
        else
            return;
    }
    else
    {
        _settings->setValue(Settings::Section_Folders, "decommenter", _decommenterLoc);
    }

    connect(_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
    connect(_process, SIGNAL(started()), this, SLOT(processStarted()));
    connect(_process, &QProcess::errorOccurred,
            this, &MathCheckHandler::on_Proc_err);

    _programState = State_Ready;
}

void MathCheckHandler::on_Proc_err(QProcess::ProcessError err)
{
    if (err == QProcess::FailedToStart)
        QMessageBox::critical(nullptr, tr("MathCheck ei käynnistynyt"),
                              tr("Varmista että MathCheck on missä sen pitäisi"
                                 " olla, ja että sinulla on käyttöoikeudet"
                                 " siihen."));
    else if (err == QProcess::Crashed)
        QMessageBox::critical(nullptr, tr("MathCheck kaatui"),
                              tr("Yritä tarkistusta uudelleen. Jos MathCheck"
                                 " kaatuu uudestaan, ilmoita PunaKynän"
                                 " ylläpitäjälle."));
    else if (err == QProcess::Timedout)
        QMessageBox::critical(nullptr, tr("MathCheck jäi jumiin"),
                              tr("Yritä tarkistusta uudelleen. Jos MathCheck"
                                 " jää jumiin uudestaan, ilmoita PunaKynän"
                                 " ylläpitäjälle."));
    else
        QMessageBox::critical(nullptr, tr("Tuntematon ongelma popplerissa"),
                              tr("Yritä tarkistusta uudelleen. Jos MathCheck"
                                 " lakkaa toimimasta uudestaan, ilmoita"
                                 " PunaKynän ylläpitäjälle."));
}

QString MathCheckHandler::getCSS()
{

    QString ret = _settings->getValue(Settings::Section::Section_Folders, "mathcheckCSS");

    if (ret != "")
        return ret;

    ret = _mathCheckLoc.section(QDir::separator(),0,-2);
    QDir dir(ret);

    if (dir.exists("mathcheck.css"))
    {
        ret += QDir::separator();
        ret += "mathcheck.css";
        _settings->setValue(Settings::Section::Section_Folders, "mathcheckCSS", ret);
        return ret;
    }
    return QString();
}

bool MathCheckHandler::isAvailable()
{
    if (_programState == State_Ready)
        return true;
    return false;
}

bool MathCheckHandler::textInput(const QString &text)
{
    if (_programState != State_Ready)
        return false;

    _settings->writeDebug("MathCheck alkaa");

    _inputOutput = text.toUtf8();

    decommentMathCheck();
    return true;
}

QString MathCheckHandler::waitForOutput(const QString &text)
{
    if (_process->state() == QProcess::Running)
        _process->close();
    _process->start(_decommenterLoc);
    int test = _process->write(text.toUtf8());
    QByteArray ret;
    if (test < 0)
    {
        _process->close();
        _settings->writeError("MathCheck write error");
        return "MathCheck write error";
    }
//    if(_process->state() == QProcess::Running)
        _process->closeWriteChannel();

    _process->waitForFinished();
    ret = _process->readAllStandardOutput();

    _process->start(_mathCheckLoc);
    test = _process->write(ret);
    if (test < 0)
    {
        _process->close();
        _settings->writeError("MathCheck write error");
        return "MathCheck write error";
    }
//    if(_process->state() == QProcess::Running)
        _process->closeWriteChannel();

    _process->waitForFinished();
    ret = _process->readAllStandardOutput();
    return QString::fromUtf8(ret);
}

void MathCheckHandler::processStarted()
{

    if (_programState == State_Decommenting_Started ||
        _programState == State_Checking_Started)
    {
        ++_programState;
        int test = _process->write(_inputOutput);
        if (test < 0)
        {
            _process->close();
            _settings->writeError("MathCheck write error");
            _programState = State_Ready;
        }
        if(_process->state() == QProcess::Running)
            _process->closeWriteChannel();
    }
}


void MathCheckHandler::processFinished(int)
{
    if (_programState == State_Decommenting)
    {
        _inputOutput = _process->readAllStandardOutput();
        _settings->writeDebug("MathCheck decomment valmis");
        _programState = State_Decommentin_Finished;
        callMathCheck();
    }

    if (_programState == State_Checking)
    {
        _inputOutput = _process->readAllStandardOutput();
        _settings->writeDebug("MathCheck valmis");
        _programState = State_Ready;
        emit textOutput(QString::fromUtf8(_inputOutput));
    }
}

void MathCheckHandler::decommentMathCheck()
{
    _settings->writeDebug("MathCheck decomment");

    if (_process->state() == QProcess::Running)
        _process->close();
    _programState = State_Decommenting_Started;
    _process->start(_decommenterLoc);
}

void MathCheckHandler::callMathCheck()
{
    _settings->writeDebug("MathCheck");

    if (_process->state() == QProcess::Running)
        _process->close();
    _programState = State_Checking_Started;
    _process->start(_mathCheckLoc);
}
