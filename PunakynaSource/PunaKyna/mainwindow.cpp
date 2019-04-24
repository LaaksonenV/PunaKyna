#include "mainwindow.h"

#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QScreen>
#include <QKeyEvent>
#include <QString>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>

#include "gradingwindow.h"
#include "browserwindow.h"
#include "browsermodel.h"
#include "settings.h"
#include "smalldialogs.h"
#include "displaywidget.h"
#include "build_def.h"
#include "settingswindow.h"
#include "textparser.h"
#include "csvparser.h"

MainWindow::MainWindow(Settings *set, QWidget *parent)
    : QMainWindow(parent)
    , _settings(set)
    , _testtree(new BrowserWindow(_settings, this))
    , _testmodel(new BrowserModel(_settings))
    , _display(new DisplayWidget(_settings, this))
    , _grading(new GradingWindow(_settings, this))
    , _fileMenu(new QMenu(tr("Tiedosto"), this))
    , _ExamMenu(new QMenu(tr("Tentti"), this))
    , _LockMenu(new QMenu(tr("Lukitse"), this))
    , _AutoMenu(new QMenu(tr("Automaattitarkastus"), this))
    , _GradeAction(new QAction(tr("Vie arviointi tiedostoon"), this))
    , _NumbAction(new QAction(tr("Muuta tehtävien numerointia"), this))
{
    _settings->writeDebug("Ohjelma alkaa");

    if (_settings->getValue(Settings::Section::Section_User, "lastuser") == "" ||
        QMessageBox::question(this, tr("Käyttäjä:"), _settings->getValue(Settings::Section::Section_User, "lastuser"),
                              tr("Kyllä"), tr("Ei")) == 1)
    {
        QStringList saved = _settings->getMultiValue(Settings::Section::Section_User, "savedusers");
        QString user("");
        bool ok = true;
        user = QInputDialog::getItem(this, tr("Käyttäjän valinta"),
                                     tr("Valitse käyttäjä tai kirjoita uusi"),
                                     saved, 0, true, &ok);
        if (!ok)
            user = "";
        else if (user != "" && !saved.contains(user))
            _settings->addMultiValue(Settings::Section::Section_User, "savedusers", user);

        _settings->setValue(Settings::Section::Section_User, "lastuser", user);
    }

    connect(_testtree, &BrowserWindow::entrySelected,
            this, &MainWindow::on_browserSelect);
    connect(_testtree, &BrowserWindow::badEntry,
            this, &MainWindow::on_browserBadEntry);

    // Vai dockwidget ollenkaan?
    QDockWidget *docker = new QDockWidget(tr("Vastaukset"),this);
    //TODO widgetin voi sulkea, efektiivisesti pienentää
    docker->setFeatures(QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable);

    docker->setWidget(_testtree);
    addDockWidget(Qt::LeftDockWidgetArea,docker);


    connect(_grading, &GradingWindow::stateChanged,
            this, &MainWindow::on_stateChange);
    connect(_grading, &GradingWindow::nextAnswer,
            _testtree, &BrowserWindow::selectNext);
    connect(_grading, &GradingWindow::prevAnswer,
            _testtree, &BrowserWindow::selectPrev);

    docker = new QDockWidget(tr("Kommentit"),this);
    docker->setFeatures(QDockWidget::DockWidgetMovable |
                        QDockWidget::DockWidgetFloatable);

    docker->setWidget(_grading);
    addDockWidget(Qt::RightDockWidgetArea,docker);
    docker->setMinimumWidth(400);

    setCentralWidget(_display);


    menuBar()->addMenu(_fileMenu);

    QAction *act = _fileMenu->addAction(tr("Avaa uusi tentti"));
    connect(act, &QAction::triggered,
            this, &MainWindow::on_selectExam);

    QStringList recents = _settings->getMultiValue(
                Settings::Section::Section_Folders, "recent_exams");

    if (recents.count())
    {
        QMenu *menu = _fileMenu->addMenu(tr("Avaa vanha tentti"));
        QStringList text;
        text << "" << "" << "";
        for (int i = 0; i < recents.count(); ++i)
        {
            text[0] = recents.at(i).section(SetSep,0,-3);
            text[1] = recents.at(i).section(SetSep,-2,-2);
            text[2] = recents.at(i).section(SetSep,-1,-1);
            act = menu->addAction(text.at(0));
            connect(act, &QAction::triggered, this,
                    [=](){selectExamDir(text.at(0), text.at(1),
                                        text.at(2));});
        }
    }

    menuBar()->addMenu(_ExamMenu);

    _ExamMenu->addAction(_GradeAction);
    connect(_GradeAction, &QAction::triggered, this,
            &MainWindow::on_exportToGrading);
    _GradeAction->setDisabled(true);

    _ExamMenu->addSeparator();
    _ExamMenu->addAction(_NumbAction);
    connect(_NumbAction, &QAction::triggered,
            this, &MainWindow::on_checkQuestionNumbering);
    _NumbAction->setDisabled(true);
    _ExamMenu->addSeparator();

    act = _ExamMenu->addAction(tr("Tallenna muutokset"));
    connect(act, &QAction::triggered, this, &MainWindow::on_save);

    _ExamMenu->setDisabled(true);

    menuBar()->addMenu(_LockMenu);
    act = _LockMenu->addAction(tr("Tentti"));
    connect(act, &QAction::triggered,
            this, &MainWindow::on_lockCommented);

    _LockMenu->setDisabled(true);

    menuBar()->addMenu(_AutoMenu);
    act = _AutoMenu->addAction(tr("Koko tentti"));
    connect(act, &QAction::triggered,
            this, &MainWindow::on_auto_all);

    act = _AutoMenu->addAction(tr("Valittu kysymys"));
    connect(act, &QAction::triggered,
            this, &MainWindow::on_auto_some);

    act = _AutoMenu->addAction(tr("Valittu vastaus"));
    connect(act, &QAction::triggered,
            this, &MainWindow::on_auto_one);

    _AutoMenu->addSeparator();

    act = _AutoMenu->addAction(tr("Muokkaa tarkastajaa"));
    connect(act, &QAction::triggered,
            _grading, &GradingWindow::editCurrentAuto);

    _AutoMenu->setDisabled(true);

#ifdef Q_OS_MAC
    QMenu *prefMenu = new QMenu(tr("&Preferences"), this);
    act = prefMenu->addAction(tr("preferences"));
    menuBar()->addMenu(prefMenu);
#else
    act = menuBar()->addAction(tr("Asetukset"));
#endif // Q_OS_MAC
    connect(act, &QAction::triggered,
            this, &MainWindow::on_settings);


#ifdef Q_OS_MAC
    QMenu *helpMenu = new QMenu(tr("&Help"), this);
    act = helpMenu->addAction(tr("about"));
    menuBar()->addMenu(helpMenu);
#else
    act = menuBar()->addAction(tr("Tietoa"));
#endif // Q_OS_MAC
    connect(act, &QAction::triggered,
            this, &MainWindow::on_about);

    QScreen *scr = QGuiApplication::primaryScreen();
    QSize windowSize = scr->availableSize();
    resize(windowSize);

    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    if (_testmodel)
        delete _testmodel;
}

bool MainWindow::selectExamDir(const QString &folder, QString type,
                               QString fileext)
{
    QString fileName;
    QStringList formats = fileext.split('.', QString::SkipEmptyParts);

    fileName = _testmodel->setExamDir(folder, type, formats);
    
 /*   else if (type == 1)
        fileName = _testmodel->setExamDir(folder,BrowserModel::Type_Moodle,
                                          formats);*/
    if (fileName.isEmpty())
        return false;

    bool found = true;
    fileName = _grading->checkExamFile(fileName, &found);
    QMap<QString, int> questions = _testmodel->getQuestionNames();
    if (!found)
    {
        if (questions.isEmpty() ||
            !_grading->generateExamFile(fileName, questions))
            return false;
    }
    else if (!_grading->setExamFile(fileName, questions))
        return false;

    _testtree->setBrowserModel(_testmodel);

    showMenus(0);

    QString value =(folder + SetSep + type + SetSep + fileext);
    QStringList recents = _settings->getMultiValue(
                Settings::Section::Section_Folders, "recent_exams");
    if (recents.contains(value))
        return true;

    while (recents.count() >= 10)
        recents.removeLast();
    recents.prepend(value);
    _settings->setMultiValue(Settings::Section::Section_Folders,
                             "recent_exams", recents);

    return true;



}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->accept();
    else
        e->ignore();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urllst = e->mimeData()->urls();
    QStringList lst;
    foreach (QUrl u, urllst)
    {
        lst << u.toString().remove("file://");
    }
    if (lst.count() == 1)
    {
        if (hasOpenExam())
        {
            if (lst.at(0).endsWith("grading.csv"))
                exportToGrading(lst.at(0));
        }
        else
            prepareSelectedExam(lst.at(0));
    }
}

bool MainWindow::hasOpenExam()
{
    if (_testmodel)
        return true;
    return false;
}

void MainWindow::showMenus(int type)
{
    _fileMenu->setDisabled(true);
    _ExamMenu->setEnabled(true);
    _LockMenu->setEnabled(true);
    _AutoMenu->setEnabled(true);
    _GradeAction->setEnabled(true);
    
    if (type == 0)
        _NumbAction->setEnabled(true);


}

bool MainWindow::prepareSelectedExam(QString folder)
{
    QStringList types(_testmodel->getExamTypes());
    SmallDialogs::ExamType typedial(types, this);
    if (typedial.exec() == QDialog::Rejected)
        return false;
    if (selectExamDir(folder, typedial.getDirType(),
                       (typedial.getFileExtension()).join('.')))
        _settings->setValue(Settings::Section::Section_Folders, "lastexam",
                            folder);
    return true;
}

bool MainWindow::search_pdf2text()
{
    QDir dir(_settings->appPath());

    QString popplerLoc = _settings->getValue(Settings::Section::Section_Folders,
                                      "pdftxt");
    QString popplerfile;
#ifdef Q_OS_MACOS
        popplerfile = "Poppler.sh";
#else
        popplerfile = "pdftotext.exe";
#endif

    if (popplerLoc == "")
    {
        dir.cd("Pdf2Txt");

        popplerLoc = dir.absoluteFilePath(popplerfile);



    }

    if (!dir.exists(popplerLoc))
    {
        QFileDialog dial(nullptr, tr("Valitse Pdf2Txt -tiedosto"),
                         dir.absolutePath());
        dial.setFileMode(QFileDialog::ExistingFile);

        QString text = "Valitse Pdf2Txt-tiedosto ("
                        + popplerfile +
                        ")";
        dial.setLabelText(QFileDialog::Accept, text);
        if (dial.exec())
            popplerLoc = dial.selectedFiles().at(0);
        else
            popplerLoc.clear();

        if (popplerLoc.isEmpty())
            return false;
    }

    _settings->setValue(Settings::Section_Folders, "pdftxt",
                        popplerLoc);

    return true;
}

QString MainWindow::createText(const QString &fileName)
{
    QString text;

    QFile fl;
    QDir dir(fileName);
    dir.cdUp();
    if(!dir.cd("ASTEXT"))
    {
        dir.mkdir("ASTEXT");
        dir.cd("ASTEXT");
    }
    if (fileName.endsWith(".html") || fileName.endsWith(".htm"))
        fl.setFileName(dir.absoluteFilePath("as.html"));
    else
        fl.setFileName(dir.absoluteFilePath("as.txt"));

    if(fl.open(QFile::Text | QFile::ReadOnly))
    {
        QTextStream str(&fl);
        str.setCodec("UTF-8");
        text = str.readAll();
        fl.close();
    }
    else
    {
        text = _display->createText(fileName);

        if(fl.open(QFile::Text | QFile::WriteOnly))
        {
            QTextStream str(&fl);
            str.setCodec("UTF-8");
            str << text;
            fl.close();
        }
    }
    return text;
}

CSVParser *MainWindow::fitTemplate(Indexer &ind)
{
    if (ind.answ < 0 || ind.quest < 0)
        return nullptr;

    Indexer temp;
    temp.quest = ind.quest;

    QString templ = _testmodel->getText(temp, BrowserModel::File);
    if (templ.isEmpty())
        //Message
        return nullptr;

    templ = _display->createText(templ);
    templ = templ.simplified();

    QString sep = "#TEKSTI\\w*#";

    QStringList caps = TextParser::testTemplate(templ, sep);
    if (caps.count() <= 0)
        //Message
        return nullptr;
    caps.prepend(sep);

    QString filen = _testmodel->getText(ind, BrowserModel::File);
    if (filen.isEmpty())
        //Message
        return nullptr;

    QString file = createText(filen);
    file = file.simplified();

    QStringList texts;
    TextParser::Status st = TextParser::parseText(texts, file, templ, sep);

    //Error checks
    if (st == TextParser::Done)
    {
        texts.prepend("");
        QDir dir(filen);
        dir.cdUp();
        if(!dir.cd("ASTEXT"))
        {
            dir.mkdir("ASTEXT");
            dir.cd("ASTEXT");
        }
        filen = dir.absoluteFilePath("as.csv");

        CSVParser *csv = new CSVParser(filen, true, true, false);
        csv->addRow(caps);
        csv->addRow(texts);
        csv->printCSV(false);
        return csv;
    }
    return nullptr;
}

void MainWindow::on_browserSelect(Indexer &ind)
{
    _settings->writeDebug("Tiedosto valittu");

    _display->setDisplayFile(_testmodel->getText(ind, BrowserModel::File));

    Indexer temp;
    QString quest = QString();
    if (ind.quest >= 0)
    {
        temp.quest = ind.quest;
        quest = _testmodel->getText(temp, BrowserModel::Identifier);
    }
    if (ind.answ < 0)
        _grading->changeGradeable(quest, QString());
    else
    {
        temp.answ = ind.answ;
        _grading->changeGradeable(quest,
                           _testmodel->getText(temp, BrowserModel::Identifier));
    }

}

void MainWindow::on_browserBadEntry(Indexer &ind)
{
    Indexer temp;
    QString quest = QString();
    if (ind.quest >= 0)
    {
        temp.quest = ind.quest;
        quest = _testmodel->getText(temp, BrowserModel::Identifier);
    }
    if (ind.answ >= 0)
    {
        temp.answ = ind.answ;
        _grading->badEntry(quest,
                           _testmodel->getText(temp, BrowserModel::Identifier),
                       _testmodel->getText(temp, BrowserModel::State).toInt());
    }
}

void MainWindow::on_stateChange(const QString &question, const QString &answer,
                                int state)
{
    if (!_testmodel)
        return;
    Indexer ind = _testmodel->getIndexOf(question, answer);
    if ((!question.isEmpty() && ind.quest < 0) ||
        (!answer.isEmpty() && ind.answ < 0))
        return;

    _testmodel->setText(ind, BrowserModel::State,
                        GradingState::stateToChar(state));
}

void MainWindow::on_lockCommented()
{
    //NOT IMPLEMENTED
}

void MainWindow::on_selectExam()
{
    QString fileName = _settings->getValue(Settings::Section::Section_Folders,
                                           "lastexam");

    QFileDialog dial(this, tr("Valitse tarkastettavan tentin kansio"),
                     fileName);
    dial.setFileMode(QFileDialog::Directory);
    dial.setLabelText(QFileDialog::Accept, tr("Valitse tenttikansio"));

    if (dial.exec())
        fileName = dial.selectedFiles().at(0);
    else fileName = QString();


    if (!fileName.isEmpty())
        prepareSelectedExam(fileName);
}

void MainWindow::on_exportToGrading()
{
    _settings->writeDebug("Export gradingiin");

    QString fileName;// = _testmodel->getGradings();

    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this,
                                            tr("Valitse gradings.csv tiedosto"),
                                                QDir::homePath(), "(*.csv)");
    if (fileName.isEmpty())
        return;

    exportToGrading(fileName);
}

void MainWindow::exportToGrading(QString fileName)
{
    _grading->exportToGrading(fileName);
}

void MainWindow::on_save()
{
    _settings->writeDebug("Tallennus");

    _grading->saveAllWork();
}

void MainWindow::on_checkQuestionNumbering()
{
    _settings->writeDebug("Kysymysten numerointi");

    _grading->checkQuestionNumbers();
}

void MainWindow::on_auto_one()
{
    if (!search_pdf2text())
        return;

    QString text = _display->getText();
    if (!text.isEmpty())
        _grading->autoCheckCurrent(text);
}

void MainWindow::on_auto_some()
{
    if (!search_pdf2text())
        return;

    Indexer ind = _testtree->currentIndex();
    ind.xtr = -1; ind.answ = -1;
    QStringList quest = _testmodel->getMeta(ind);
    QStringList ans;
    QString text;
    CSVParser *tmpl = nullptr;
    for (int i = 0; i < _testmodel->indexCount(ind); ++i)
    {
        ind.answ = i;
        ans = _testmodel->getMeta(ind);

        if ((tmpl = fitTemplate(ind)) == nullptr)
        {
            text = createText(_testmodel->getText(ind, BrowserModel::File));
            if (text.isEmpty())
            {
                ind.answ = -1;
                continue;
            }
            tmpl = new CSVParser();
            tmpl->addColumn(QStringList("") << text);
        }

        _grading->autoCheck(*tmpl, quest, ans);

        delete tmpl;
        ind.answ = -1;
    }
}

void MainWindow::on_auto_all()
{
    if (!search_pdf2text())
        return;

    Indexer ind;
    QStringList ans;
    QStringList quest;
    QString text;
    CSVParser *tmpl = nullptr;

    for (int k = 0; k < _testmodel->indexCount(ind); ++k)
    {
        ind.quest = k;
        quest = _testmodel->getMeta(ind);
        for (int i = 0; i < _testmodel->indexCount(ind); ++i)
        {
            ind.answ = i;

            ans = _testmodel->getMeta(ind);

            if ((tmpl = fitTemplate(ind)) == nullptr)
            {
                text = createText(_testmodel->getText(ind, BrowserModel::File));
                if (text.isEmpty())
                {
                    ind.answ = -1;
                    continue;
                }
                tmpl = new CSVParser();
                tmpl->addColumn(QStringList("") << text);
            }

            _grading->autoCheck(*tmpl, quest, ans);

            delete tmpl;
            ind.answ = -1;
        }
        ind.quest = -1;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Enter: case Qt::Key_Return:
        if (e->modifiers() == Qt::ShiftModifier)
            _testtree->selectPrev();
        else
            _testtree->selectNext();
        return;
    default:
        if (e->text() != "" && _grading->keyPress(e->text()))
            return;
        QWidget::keyPressEvent(e);
    }
}

void MainWindow::on_settings()
{
    SettingsWindow s(_settings, this);
    s.exec();
}

void MainWindow::on_about()
{

    QMessageBox::information(this, tr("Tietoa"),
                             ABOUT);
}
