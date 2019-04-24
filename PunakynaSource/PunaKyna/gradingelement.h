#ifndef GRADINGELEMENT_H
#define GRADINGELEMENT_H

#include <QWidget>

class QStringList;
class QLineEdit;
class QCheckBox;

class Settings;

class GradingElement : public QWidget
{
    Q_OBJECT
public:
    explicit GradingElement(Settings *set, unsigned id, QWidget *parent = 0);

    void setId(int i){_id = i;}
    void setContent(const QStringList &text);
    void setShortcut(QString c);
    bool checkElement(bool check);
    bool isElementChecked();
    void changeCheck();

    int getValue() const;
    QString getName() const;

protected:
    void paintEvent(QPaintEvent *);

signals:
    void elementChecked(unsigned id, bool check);
    void elementChanged(unsigned id, QString value,
                        int column);

public slots:
    int on_elementChecked(bool checked);

private:
    Settings *_settings;

    unsigned _id;
    QLineEdit *_short;
    QLineEdit *_name;
    QCheckBox *_check;
    QLineEdit *_comment;
    QLineEdit *_points;

};

#endif // GRADINGELEMENT_H
