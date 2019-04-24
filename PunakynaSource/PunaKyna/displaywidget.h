#ifndef DISPLAYWIDGETCREATOR_H
#define DISPLAYWIDGETCREATOR_H

#include <QScrollArea>
#include <QProcess>


class Settings;

class QString;
class MathCheckHandler;
class QPdfDocument;
class QTextEdit;

class DisplayWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit DisplayWidget(Settings *set, QWidget *parent = nullptr);

    QString createText(const QString &fileName);

    QString getText();

public slots:
    void setDisplayFile(const QString &file);

signals:
    void widgetReady();

private slots:
    void on_MCHandlerReady(QString text);
    void on_Proc_err(QProcess::ProcessError err);

private:
    void handleMathCheck(const QString &file);
    void handleXML(const QString &file);
    void handlePdf(const QString &file);
    bool tryHandelPicture(const QString &file);

private:
    Settings *_settings;
    bool _busy;

    QString _file;

    MathCheckHandler *_mcHandle;
    QTextEdit *_htmlWindow;
    QPdfDocument *_pdfDoc;

    QString _popplerLoc;
};

#endif // DISPLAYWIDGETCREATOR_H
