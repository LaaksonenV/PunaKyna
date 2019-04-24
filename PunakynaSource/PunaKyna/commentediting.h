#ifndef COMMENTEDITING_H
#define COMMENTEDITING_H

class QStringList;
class QLabel;

#include <QDialog>

namespace Editor {

const int window_def_length = 200;
const int comment_height = 50;
const int text_height = 25;

class Comment : public QWidget
{
    Q_OBJECT
public:
    Comment(QString name, int id, QWidget *parent = nullptr);

    QSize sizeHint() const;

    void moveByStep(int i);

    void remove(bool rem);
    bool toBeRemoved() const;
    void select(bool sel);

    int getId() const;

    void mousePressEvent(QMouseEvent *);

protected:
    void paintEvent(QPaintEvent *);

signals:
    void selected(Comment *,bool);

private:
    QString _name;
    int _id;

    bool _selected;
    bool _tbr;
};

class CommentWindow : public QWidget
{
    Q_OBJECT
public:
    CommentWindow(QWidget *parent = nullptr);

    QSize sizeHint() const;

    void addComment(QString name);

    QVector<int> getRemoved();
    QVector<int> getNewOrder();

    void moveComment(bool up);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e);

private slots:
    void on_comment_select(Comment *c, bool sel);

private:
    QList<Comment*> _comments;
    Comment *_selected;
};

}

class CommentEditing : public QDialog
{
    Q_OBJECT
public:    
    CommentEditing(const QStringList &comments, QWidget *parent = nullptr);

    QVector<int> getRemoved();
    QVector<int> getNewOrder();

private:
    Editor::CommentWindow *wind;

};
#endif // COMMENTEDITING_H
