#include "settingswindow.h"
#include "settings.h"

#include <QGridLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QListWidget>
#include <QStringList>
#include <QComboBox>
#include <QLineEdit>
#include <QFileDialog>

SettingsWindow::SettingsWindow(Settings *set,QWidget *parent)
    : QDialog(parent)
    , _settings(set)
{
    QGridLayout *lay = new QGridLayout(this);

    QTabWidget *mainwidget = new QTabWidget(this);
    lay->addWidget(mainwidget,0,0,3,3);

    QPushButton *button = new QPushButton(tr("Peruuta"),this);
    connect(button, &QPushButton::clicked, this, &SettingsWindow::on_cancel);
    lay->addWidget(button, 3,1);

    button = new QPushButton(tr("Ok"),this);
    connect(button, &QPushButton::clicked, this, &SettingsWindow::on_ok);
    lay->addWidget(button, 3,2);


    QWidget *tab = new QWidget();
    lay = new QGridLayout(tab);

    QLabel *label = new QLabel(tr("Tenttilokien kansiot"),tab);
    lay->addWidget(label,0,0);
    _examfolders = new QListWidget(tab);
    QStringList folders = _settings->getMultiValue(Settings::Section_Folders,
                                                   "examfiles");
    _examfolders->addItems(folders);
    lay->addWidget(_examfolders,1,0,2,2);

    button = new QPushButton(tr("Lisää"), tab);
    connect(button, &QPushButton::clicked, this, &SettingsWindow::on_addExam);
    lay->addWidget(button, 3,0);

    button = new QPushButton(tr("Poista"), tab);
    connect(button, &QPushButton::clicked,
            this, &SettingsWindow::on_removeExam);
    lay->addWidget(button, 3,1);

    label = new QLabel(tr("Oletuskansio:"),tab);
    lay->addWidget(label,4,0);
    _defaultexam = new QComboBox(tab);
    _defaultexam->addItem("");
    _defaultexam->addItems(folders);
    lay->addWidget(_defaultexam,4,1);

    label = new QLabel(tr("Kysymystiedostojen kansiot"),tab);
    lay->addWidget(label,5,0);

    _questfolders = new QListWidget(tab);
    folders = _settings->getMultiValue(Settings::Section_Folders,
                                                   "questionfiles");
    _questfolders->addItems(folders);
    lay->addWidget(_questfolders,6,0,2,2);

    button = new QPushButton(tr("Lisää"), tab);
    connect(button, &QPushButton::clicked, this, &SettingsWindow::on_addQuest);
    lay->addWidget(button, 8,0);

    button = new QPushButton(tr("Poista"), tab);
    connect(button, &QPushButton::clicked,
            this, &SettingsWindow::on_removeQuest);
    lay->addWidget(button, 8,1);

    label = new QLabel(tr("Oletuskansio:"),tab);
    lay->addWidget(label,9,0);
    _defaultquest = new QComboBox(tab);
    _defaultquest->addItem("");
    _defaultquest->addItems(folders);
    lay->addWidget(_defaultquest,9,1);

    mainwidget->addTab(tab, tr("Kansiot"));

    tab = new QWidget();
    lay = new QGridLayout(tab);

    _allowc = new QCheckBox(tr("EXAM hyväksyy pilkut"),tab);
    if (string2Bool(_settings->getValue(Settings::Section_EXAM,
                                        "allow_commas")))
        _allowc->setChecked(true);
    lay->addWidget(_allowc,0,0,1,2);

    label = new QLabel(tr("Korvaa pilkut merkillä:"),tab);
    lay->addWidget(label,1,0);

    _replc = new QLineEdit(_settings->getValue(Settings::Section_EXAM,
                                               "replace_commas"), tab);
    lay->addWidget(_replc,1,1);

    _printpc = new QCheckBox(tr("Tulosta palautteeseen kommenttikohtaiset"
                                " pisteet"),tab);
    if (string2Bool(_settings->getValue(Settings::Section_EXAM,
                                        "print_cpts")))
        _printpc->setChecked(true);
    lay->addWidget(_printpc,2,0,1,2);

    _printpq = new QCheckBox(tr("Tulosta palautteeseen tehtäväkohtaiset"
                                " pisteet"),tab);
    if (string2Bool(_settings->getValue(Settings::Section_EXAM,
                                        "print_qpts")))
        _printpq->setChecked(true);
    lay->addWidget(_printpq,3,0,1,2);

    mainwidget->addTab(tab, tr("EXAM"));

    tab = new QWidget();
    lay = new QGridLayout(tab);

    label = new QLabel(tr("Tyhjän palautuksen kommentti:"),tab);
    lay->addWidget(label,0,0);

    _noret = new QLineEdit(_settings->getValue(Settings::Section_DEF,
                                               "no_returns"), tab);
    lay->addWidget(_noret,0,1);

    label = new QLabel(tr("Väärän palautuksen kommentti:"),tab);
    lay->addWidget(label,1,0);

    _wrret = new QLineEdit(_settings->getValue(Settings::Section_DEF,
                                               "wrong_format"), tab);
    lay->addWidget(_wrret,1,1);

    mainwidget->addTab(tab, tr("Kommentit"));
}

void SettingsWindow::on_ok()
{
    saveSettings();
    accept();
}

void SettingsWindow::on_cancel()
{
    reject();
}

void SettingsWindow::on_addExam()
{
    QFileDialog dial(this, tr("Valitse tenttilokien kansio"));
    dial.setFileMode(QFileDialog::Directory);
    dial.setLabelText(QFileDialog::Accept, tr("Valitse tenttilokien"
                                              " kansio"));
    if (dial.exec())
    {
        _examfolders->addItems(dial.selectedFiles());
        _defaultexam->addItems(dial.selectedFiles());
    }
}

void SettingsWindow::on_removeExam()
{
    foreach (QListWidgetItem *i, _examfolders->selectedItems())
    {
        _defaultexam->removeItem(_examfolders->row(i)+1);
        _examfolders->removeItemWidget(i);
        delete i;
    }
}

void SettingsWindow::on_addQuest()
{
    QFileDialog dial(this, tr("Valitse kysymystiedostojen kansio"));
    dial.setFileMode(QFileDialog::Directory);
    dial.setLabelText(QFileDialog::Accept, tr("Valitse kysymystiedostojen"
                                              " kansio"));
    if (dial.exec())
    {
        _questfolders->addItems(dial.selectedFiles());
        _defaultquest->addItems(dial.selectedFiles());
    }
}

void SettingsWindow::on_removeQuest()
{
    foreach (QListWidgetItem *i, _questfolders->selectedItems())
    {
        _defaultquest->removeItem(_questfolders->row(i)+1);
        _questfolders->removeItemWidget(i);
        delete i;
    }
}

void SettingsWindow::saveSettings()
{
    QStringList lst;
    for (int i = 0; i < _examfolders->count(); ++i)
        lst << _examfolders->item(i)->text();
    _settings->setMultiValue(Settings::Section_Folders, "examfiles", lst);

    lst.clear();
    for (int i = 0; i < _questfolders->count(); ++i)
        lst << _questfolders->item(i)->text();
    _settings->setMultiValue(Settings::Section_Folders, "questionfiles", lst);

    _settings->setValue(Settings::Section_Folders, "defaultexam",
                        _defaultexam->currentText());
    _settings->setValue(Settings::Section_Folders, "defaultquestion",
                        _defaultquest->currentText());

    _settings->setValue(Settings::Section_EXAM, "allow_commas",
                        bool2String(_allowc->isChecked()));
    _settings->setValue(Settings::Section_EXAM, "replace_commas",
                        _replc->text());
    _settings->setValue(Settings::Section_EXAM, "print_cpts",
                        bool2String(_printpc->isChecked()));
    _settings->setValue(Settings::Section_EXAM, "print_qpts",
                        bool2String(_printpq->isChecked()));

    _settings->setValue(Settings::Section_DEF, "no_returns",
                        _noret->text());
    _settings->setValue(Settings::Section_DEF, "wrong_format",
                        _wrret->text());

}
