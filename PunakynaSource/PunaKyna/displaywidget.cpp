#include "displaywidget.h"

#include <QTextEdit>
#include <QDesktopWidget>
#include <QScrollArea>
#include <QScreen>
#include <QKeyEvent>
#include <QTextStream>
#include <QTextCodec>
#include <iostream>
#include <stdio.h>
#include <QRegularExpression>
#include <QFile>
#include <QDir>
#include <QPdfDocument>
#include <QFileDialog>
#include <QPixmap>
#include <QLabel>
#include <QMessageBox>

#include "mathcheckhandler.h"
#include "pdfwindow.h"
#include "settings.h"

DisplayWidget::DisplayWidget(Settings *set, QWidget *parent)
    : QScrollArea(parent)
    , _settings(set)
    , _busy(true)
    , _file(QString())
    , _mcHandle(nullptr)
    , _htmlWindow(nullptr)
    , _pdfDoc(nullptr)
    , _popplerLoc()
    , _stopSignal(false)
{

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _busy = false;
}

QString DisplayWidget::createText(const QString &fileName)
{
    QString s;

    if (_stopSignal)
    {
        return QString();
    }

    if (fileName.endsWith(".pdf"))
    {

        if (_popplerLoc.isEmpty())
            _popplerLoc = _settings->getValue(
                        Settings::Section::Section_Folders, "pdftxt");

        if (_popplerLoc.isEmpty())
            return QString();

        QProcess prc;
        connect(&prc, &QProcess::errorOccurred,
                this, &DisplayWidget::on_Proc_err);

        QStringList args = QStringList(fileName);

#ifndef Q_OS_MACOS
//        args.prepend("-eol unix");
        args << "-";
#endif

        prc.start(_popplerLoc, args, QIODevice::ReadOnly);

        prc.waitForFinished();
        s = prc.readAll();
    }

    else if (fileName.endsWith(".html") || fileName.endsWith(".htm"))
    {
        if (!_mcHandle)
        {
            _mcHandle = new MathCheckHandler(_settings, this);
            connect(_mcHandle, &MathCheckHandler::textOutput,
                    this, &DisplayWidget::on_MCHandlerReady);
        }

        //uudelleen testaus?
        if (!_mcHandle->isAvailable())
        {
            return QString();
        }

        QFile html(fileName);
        if (!html.open(QFile::Text | QFile::ReadOnly))
            return QString();
        QTextStream input(&html);
        QStringList inputText;
        while (!input.atEnd())
            inputText << input.readLine();

        s = _mcHandle->waitForOutput(inputText.join('\n'));
    }

/*    else if (!fileName.endsWith(".xml") ||
             !fileName.endsWith(".mlx"))
            return QString();

    QFile file (fileName);
    if (fileName.endsWith(".mlx"))
    {
        fl.setFileName(dir.absoluteFilePath("as.txt"));

        QString unzipcmd;
        QDir dir(fileName);
        dir.cdUp();
        QStringList expEntries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (!expEntries.count() || !expEntries.contains("expanded"))
        {
            unzipcmd = "unzip ";
            unzipcmd += dir.absoluteFilePath(fileName);
            unzipcmd += " -d ";
            unzipcmd += dir.absolutePath();
            unzipcmd += QDir::separator();
            unzipcmd += "expanded";
            system(unzipcmd.toStdString().c_str());
        }
        dir.cd("expanded");
        dir.cd("matlab");
        file.setFileName(dir.filePath("document.xml"));
    }

    if (!file.open(QFile::Text | QFile::ReadOnly))
        return QString();
    QStringList ret;
    QTextStream str(&file);
    while (!str.atEnd())
        ret << str.readLine();
    return ret.join('\n');
    */

    return s;

}

void DisplayWidget::on_Proc_err(QProcess::ProcessError err)
{
    if (err == QProcess::FailedToStart)
        QMessageBox::critical(this, tr("Poppler ei käynnistynyt"),
                              tr("Varmista että poppler on missä sen pitäisi"
                                 " olla, ja että sinulla on käyttöoikeudet"
                                 " siihen."));
    else if (err == QProcess::Crashed)
        QMessageBox::critical(this, tr("Poppler kaatui"),
                              tr("Yritä tarkistusta uudelleen. Jos poppler"
                                 " kaatuu uudestaan, ilmoita ylläpitäjälle."));
    else if (err == QProcess::Timedout)
        QMessageBox::critical(this, tr("Poppler jäi jumiin"),
                              tr("Yritä tarkistusta uudelleen. Jos poppler"
                                 " jää jumiin uudestaan, ilmoita"
                                 " ylläpitäjälle."));
    else
        QMessageBox::critical(this, tr("Tuntematon ongelma popplerissa"),
                              tr("Yritä tarkistusta uudelleen. Jos poppler"
                                 " lakkaa toimimasta uudestaan, ilmoita"
                                 " ylläpitäjälle."));
    _stopSignal = true;
}

QString DisplayWidget::getText()
{
    return createText(_file);
}

void DisplayWidget::resetState()
{
    _stopSignal = false;
}

void DisplayWidget::setDisplayFile(const QString &file)
{
    if (_file == file)
    {
        emit widgetReady();
        return;
    }

    _file = file;

    // HTML ei automaattisesti mathcheck tarkistus?
    if (_file.endsWith(".html") || _file.endsWith(".htm"))
        handleMathCheck(_file);

    else if (_file.endsWith(".pdf"))
        handlePdf(_file);

    else if (_file.endsWith(".xml"))
        handleXML(_file);

    else if (_file.endsWith(".mlx"))
    {
        QString unzipcmd;
        QDir dir(_file);
        dir.cdUp();
        QStringList expEntries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (!expEntries.count() || !expEntries.contains("expanded"))
        {
            _settings->writeDebug("Unzippaus");

            unzipcmd = "unzip ";
            unzipcmd += dir.absoluteFilePath(_file);
            unzipcmd += " -d ";
            unzipcmd += dir.absolutePath();
            unzipcmd += QDir::separator();
            unzipcmd += "expanded";
            system(unzipcmd.toStdString().c_str());
        }
        dir.cd("expanded");
        dir.cd("matlab");
        handleXML(dir.filePath("document.xml"));
    }
    else if (!tryHandelPicture(_file))
    {
        QTextEdit *window = new QTextEdit(this);

        window->setPlainText(tr("Ei näytettävää tiedostoa"));
        window->setReadOnly(true);
        setWidget(window);
        window->resize(size());
        window->show();

        emit widgetReady();
        return;
    }

}

void DisplayWidget::on_MCHandlerReady(QString text)
{
    QRegularExpression rx;
    QString htmlcss = "<link rel = stylesheet type = \"text/css\" href = \"";
    htmlcss.replace(' ', "\\s*");
    rx.setPattern(htmlcss);
    QRegularExpressionMatch rxm = rx.match(text);
    int firstpos = rxm.capturedEnd();
    if (firstpos >= 0)
    {
        rx.setPattern("\"\\s*>");
        int lastpos = text.indexOf(rx,firstpos);
        if (lastpos > 0)
        {
            int len = lastpos-firstpos;
            text.replace(firstpos,len,_mcHandle->getCSS());
        }
    }
    QTextEdit *window = new QTextEdit(this);
    window->setHtml(text);
    window->setReadOnly(true);
    setWidget(window);
    window->resize(size());
    window->show();
    emit widgetReady();
}

void DisplayWidget::handleMathCheck(const QString &file)
{
    _settings->writeDebug("MCParsa");

    if (!_mcHandle)
    {
        _mcHandle = new MathCheckHandler(_settings, this);
        connect(_mcHandle, &MathCheckHandler::textOutput,
                this, &DisplayWidget::on_MCHandlerReady);
    }

    //uudelleen testaus?
    if (!_mcHandle->isAvailable())
    {
        return;
    }

    QFile html(file);
    if (!html.open(QFile::Text | QFile::ReadOnly))
        return;
    QTextStream input(&html);
    QStringList inputText;
    while (!input.atEnd())
        inputText << input.readLine();

    _mcHandle->textInput(inputText.join('\n'));
}

void DisplayWidget::handleXML(const QString &file)
{
    _settings->writeDebug("XML parsaus");

    QFile xml(file);
    if (!xml.open(QFile::Text | QFile::ReadOnly))
        return;
    QTextStream str(&xml);
    QString text;
    while (!str.atEnd())
    {
        text += str.readLine();
        text += "<br/>";
    }
    QTextEdit *window = new QTextEdit(this);

    window->setHtml(text);
    window->setReadOnly(true);
    setWidget(window);
    window->resize(size());
    window->show();

    emit widgetReady();
}

void DisplayWidget::handlePdf(const QString &file)
{
    if (!_pdfDoc)
        _pdfDoc = new QPdfDocument(this);
    _pdfDoc->load(file);

    PDFWindow *window = new PDFWindow(this);
    window->setDocument(_pdfDoc);
    setWidget(window);
    show();
    window->setZoomWindowWidth();
    emit widgetReady();
}

bool DisplayWidget::tryHandelPicture(const QString &file)
{
    QPixmap pic;
    if (!pic.load(file))
        return false;

    QLabel *window = new QLabel(this);

    window->setPixmap(pic);
    setWidget(window);
    window->resize(size());
    window->show();

    emit widgetReady();
    return true;
}
