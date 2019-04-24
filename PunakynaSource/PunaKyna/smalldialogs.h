#ifndef SMALLDIALOGS_H
#define SMALLDIALOGS_H

#include <QDialog>

template<typename T> class QVector;
#include <QMap>
class QGridLayout;
class QButtonGroup;
class QLabel;
class QSpinBox;
class QLineEdit;
class QCheckBox;
class QRadioButton;
class QComboBox;

namespace SmallDialogs
{

class QuestionNumber : public QDialog
{
    Q_OBJECT
public:

    static void getNumbers(QMap<QString, int> &questions, QVector<int> &points,
                                   QWidget *parent = nullptr);

    QuestionNumber(const QMap<QString, int> &questions, QVector<int> &points,
                   QWidget *parent = nullptr);

    int getNumber(QString of);
    QVector<int> getPoints();

public slots:
    void addNumber();
    void pointsChanged();
    void buttonToggled(int id, bool checked);

private:

    QMap<QString,int> _origin;

    QVector<QButtonGroup*> _questions;
    QVector<QSpinBox*> _points;
    QVector<ushort> _selected;
//    QVector<QLineEdit*> _modifiers;
    QLineEdit* _totalPoints;

    QStringList _aliasList;

    QGridLayout *_layout;

};

// // // // //

class ExamType : public QDialog
{
    Q_OBJECT
public:

    ExamType(QStringList types, QWidget *parent = nullptr);

    QString getDirType() const;
    QStringList getFileExtension() const;

private:
    void addToGroup(QCheckBox *b, QString text);
    void check(int i, bool check);
    
//    QButtonGroup *_dirType;
    QComboBox *_dirType;
    QRadioButton *_noFormat;
    QVector<bool> _selects;
    QVector<QString> _formats;
    int _checks;

};

// // // // //

class Grading : public QDialog
{
    Q_OBJECT
public:

    static int getGradings(QVector<int> &refer, QVector<int> &points, QWidget *parent = 0);

    //
    // refer contains originallly allocated pointlimits (at(0) = limit for grade 1 etc
    // points must have size of max points, each index containing the amount of students who have got that
    // points.
    //
    Grading(QVector<int> &refer, QVector<int> &points, QWidget *parent = 0);

public slots:
    void on_limitChange(int value, int by);

private:
    QVector<int> &_refer;
    const QVector<int> &_points;
    QVector<QLabel*> _grades;
    QVector<QLabel*> _percents;
    int _allpoints;

};

// // // // //

class AutoControl : public QDialog
{
    Q_OBJECT
public:

    static bool createControl(QString &ctrl, QStringList &labels,
                              QWidget *parent = nullptr);

    AutoControl(QStringList &labels, QWidget *parent = nullptr);

public slots:
    void on_Ok();

private:
    void setPrev(const QString &ctrl);
    bool checkControl();
    QString getControl();
    QStringList getNoControl();

    QLineEdit *control;
    int labelcount;
};

// // // // //

class GradeLog : public QDialog
{
    Q_OBJECT
public:

    GradeLog(QVector<int> datapoints, QWidget *parent = nullptr);

};

}
#endif // SMALLDIALOGS_H
