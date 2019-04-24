#include "autocommenter.h"

#include <QTextEdit>
#include <QLabel>
#include <QRegularExpression>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>

#include "settings.h"

/*!
 * \class AutoCommenter
 * \brief The AutoCommenter class handles the automatic evaluation of texts
 *
 * The main class implementation is for creating the keywords that are used
 * for the evaluation, hence it inherits, and is, a QDialog.
 *
 * The class is used with two static functions, createKeywords() to create the
 * keywords, and checkText() to evaluate text with the keywords.
 */


/*!
 * \brief AutoCommenter::checkText Static function for matching given strings
 * \param textTables Text(s) to check
 * \param keyWords Words to check the text for
 * \param fileId optional stand-in for FILE macro
 * \return \c true if text contains keywords, \c false if it doesn't, or
 *
 * Checks the given \a text for given \a keyWord string. \a KeyWord are split by
 * '§' if their order is irrelevant, and by \c newline if it is.
 *
 * \a keyWords may contain empty lines.
 */
bool AutoCommenter::checkText(const CSVParser &textTable,
                              const QString &keyWords,
                              const QStringList &qMeta,
                              const QStringList &aMeta,
                              bool forced)
{
    if (keyWords.isEmpty())
    {
        if (forced)
            return true;
        else return false;
    }
    QStringList fields = textTable.getRow(0);
    int i = 0, j = 0;
    QString text, words, capword;

    if (fields.count() == 1 && fields.at(0).isEmpty())
        return checkTextPiece(textTable.getData(1,0), keyWords, qMeta, aMeta);

    else if (fields.count() > 1)
    {
        QRegExp rx("(" + fields.at(0) + ")");

        if ((i = rx.indexIn(keyWords)) >= 0)
        {
            capword = rx.cap(1);
            j += capword.count();

            while ((i = rx.indexIn(keyWords)) >= 0)
            {
                if (fields.contains(capword))
                {
                    text = textTable.getData(1,fields.indexOf(capword));
                    words = keyWords.mid(j,i-j);
                    checkTextPiece(text, words,  qMeta, aMeta);
                }

                capword = rx.cap(1);
                j += capword.count();
            }

            if (fields.contains(capword))
            {
                text = textTable.getData(1,fields.indexOf(capword));
                words = keyWords.mid(j,i-j);
                checkTextPiece(text, words,  qMeta, aMeta);
            }

        }
        else
        {
            text = textTable.getRow(1).join("\"\\n\"");
            return checkTextPiece(text, keyWords,  qMeta, aMeta);
        }
    }
    //error
    return false;
}

/*!
 * \brief AutoCommenter::checkPieceText Static function for matching given
 *  strings
 * \param text Text to check
 * \param keyWords Words to check the text for
 * \param fileId optional stand-in for FILE macro
 * \return \c true if text contains keywords, \c false if it doesn't, or if the
 *  \a keyWord is an empty string
 *
 * Checks the given \a text for given \a keyWord string. \a KeyWord are split by
 * '§' if their order is irrelevant, and by \c newline if it is.
 *
 * \a keyWords may contain empty lines.
 */
bool AutoCommenter::checkTextPiece(const QString &text,
                              const QString &keyWords,
                              const QStringList &qMeta,
                              const QStringList &aMeta)
{
    if (keyWords.isEmpty())
        return true;

    QStringList words = keyWords.split("\"\\n\"");

    QStringList splitWord;
    QStringList negativeLookBehind;
    int lookBehindStart = 0;
    bool lookingBehind = false;
    QString oneWord;
    // pos tells where to start looking for each word, and newPos tells where
    // the last word found ended, so for the next line of words pos := newPos
    int pos = 0;
    int newPos = 0;

    // each line is checked in order
    for (int i = 0; i < words.count(); ++i)
    {
        // splitWord is split by the '§' character, and perl comment
        splitWord = words.at(i).split("(?<!\\)§");

        if (negativeLookBehind.count())
            lookingBehind = true;

        // each word is checked separately
        for (int j = 0; j < splitWord.count(); ++j)
        {
            oneWord = splitWord.at(j);
            oneWord.replace("\\FILE", aMeta.at(BrowserMetaData::Meta_IdName));
            oneWord.replace("\\QDISP",
                            qMeta.at(BrowserMetaData::Meta_DisplayName));
            oneWord.replace("\\QFOLD",
                            qMeta.at(BrowserMetaData::Meta_DisplayName));
            oneWord.replace("\\§", "§");

            // if negativelookbehind command is encountered, the keywords
            // are stored for later evaluation, and nothing is to be checked
            // for now
            /* This is neccessary for knowing the extend of the text to check
             * for missing words. Checking for existing words is easy, and can
             * be stopped immediately after the word is found, but in case of
             * nonexisting word, the whole text must be parsed, as we expect
             * the word to not to be found.
             */
            if (oneWord.startsWith("(?#!)"))
            {
                lookingBehind = false;
                negativeLookBehind << oneWord;
                if (!lookBehindStart)
                    lookBehindStart = pos;
                continue;
            }

            QRegularExpression rx(oneWord,
                                  QRegularExpression::CaseInsensitiveOption);

            QRegularExpressionMatch rxm = rx.match(text, pos);
            if (!rxm.hasMatch())
                return false;

            if (rxm.capturedEnd() > newPos)
                newPos = rxm.capturedEnd();

        }
        if (lookingBehind)
        {
            QStringRef strref(&text, lookBehindStart, newPos-lookBehindStart);
            foreach (QString word, negativeLookBehind)
            {
                QRegularExpression rx(word,
                                     QRegularExpression::CaseInsensitiveOption);

                QRegularExpressionMatch rxm = rx.match(strref);
                if (rxm.hasMatch())
                    return false;
            }
        }
        pos = newPos;
    }
    return true;
}

/*!
 * \brief AutoCommenter::createKeywords Opens a dialog for creating keywords
 * \param old Previous keywords
 * \param accepted Reference boolean, set to true if user presses 'ok'
 * \param parent \c parent QWidget for QDialog
 * \return possibly edited keywords
 *
 * Keywords given in \a old are parsed to be more user readable, and users
 * written keywords are parsed for storing.
 */
QString AutoCommenter::createKeywords(const QString &old, bool &accepted,
                                      QWidget *parent)
{
    AutoCommenter dial(parent);
    dial.setOldKeywords(old);
    if (dial.exec() == QDialog::Accepted)
        accepted = true;
    else accepted = false;

    return dial.getKeywords();
}

/*!
 * \brief AutoCommenter::AutoCommenter Constructs the AutoCommenter dialog
 * \param parent \c parent QWidget for QDialog
 * \internal
 *
 * Simple dialog with text input, help text, and "ok" and "cancel" buttons.
 * Also included is a button for loading external file as text for text input.
 */
AutoCommenter::AutoCommenter(QWidget *parent)
    : QDialog(parent)
    , _text(new QTextEdit(this))
{
    QGridLayout *lay = new QGridLayout(this);

    lay->addWidget(_text,0,0,1,4);
    lay->setColumnStretch(0,1);

    QLabel *label = new QLabel(help_text, this);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    label->setWordWrap(true);
    lay->addWidget(label ,1,0,1,4);

    QPushButton *button = new QPushButton(tr("Avaa tiedostosta"),this);
    connect(button, &QPushButton::clicked,
            this, &AutoCommenter::on_addFile);
    lay->addWidget(button, 2,1);

    button = new QPushButton(tr("OK"),this);
    connect(button, &QPushButton::clicked,
            this, &QDialog::accept);
    lay->addWidget(button, 3,2);
    button->setDefault(true);

    button = new QPushButton(tr("Peruuta"),this);
    connect(button, &QPushButton::clicked,
            this, &QDialog::reject);
    lay->addWidget(button, 3,3);

}

/*!
 * \brief AutoCommenter::on_addFile Parses given file for text
 *
 * Opens a filedialog for selecting a file to be parsed. The text gained is
 * automatically inserted into the text input.
 *
 * With this you can write your keywords beforehand and use them in multiple
 * comments, or open a file with expected answer and parse it manually to create
 * keywords.
 */
void AutoCommenter::on_addFile()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Valitse automaattitarkastus tiedosto"), QString(),
                tr("teksti (*.txt, *.xml, *.html, *htm"));
    QFile file(fileName);
    if (!file.open(QFile::Text | QFile::ReadOnly))
        return;
    QTextStream str(&file);
    QString txt = _text->toPlainText();
    while (!str.atEnd())
    {
        txt += '\n';
        txt += str.readLine();
    }
    _text->setPlainText(txt);
}

/*!
 * \brief AutoCommenter::setOldKeywords Parses old keywords
 * \param text Old keywords
 * \internal
 *
 * Replaces characters that needed to be edited for filestorage,
 * and inserts the text into text input.
 */
void AutoCommenter::setOldKeywords(QString text)
{
    text.replace("\"\\n\"", "\n");
    _text->setPlainText(text);
}

/*!
 * \brief AutoCommenter::getKeywords Parses user inputted keywords
 * \return Parsed text
 * \internal
 *
 * Replaces characters that have special meaning in saveable text format
 */
QString AutoCommenter::getKeywords()
{
    QString txt = _text->toPlainText();
    txt.replace("\n", "\"\\n\"");
    return txt;
}
