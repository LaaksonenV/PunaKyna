#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

class Settings;
class QListWidget;
class QComboBox;
class QCheckBox;
class QLineEdit;

class SettingsWindow : public QDialog
{
public:
    SettingsWindow(Settings *set, QWidget *parent = nullptr);


private slots:
    void on_ok();
    void on_cancel();

    void on_addExam();
    void on_removeExam();
    void on_addQuest();
    void on_removeQuest();


private:
    void saveSettings();


    Settings *_settings;

    QListWidget *_examfolders;
    QComboBox *_defaultexam;

    QListWidget *_questfolders;
    QComboBox *_defaultquest;

    QCheckBox *_allowc;
    QLineEdit *_replc;
    QCheckBox *_printpq;
    QCheckBox *_printpc;

    QLineEdit *_noret;
    QLineEdit *_wrret;

};

#endif // SETTINGSWINDOW_H
