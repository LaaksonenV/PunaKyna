#ifndef AUTOCOMMENTER_H
#define AUTOCOMMENTER_H

#include <QDialog>
#include <QString>

#include "csvparser.h"

class QTextEdit;

class AutoCommenter : public QDialog
{
    Q_OBJECT
public:

    static bool checkText (const CSVParser &textTable, const QString &keyWords,
                           const QStringList &qMeta, const QStringList &aMeta, bool forced);

    static bool checkTextPiece(const QString &text, const QString &keyWords,
                               const QStringList &qMeta,
                               const QStringList &aMeta);

    static QString createKeywords(const QString &old,
                                  bool &accepted,
                                  QWidget *parent = nullptr);

private slots:

    void on_addFile();

private:
    const QString help_text =
            "Avainsanat käyttäytyvät kuin Perl -kielen säännölliset"
            " lausekkeet. Tarkempaa tietoa näistä löytyy mm. sivuilta <br>\n"
            "<a href=\"http://perldoc.perl.org/perlretut.html\">"
            "http://perldoc.perl.org/perlretut.html</a> (tutorial),<br>\n"
            "<a href=\"http://perldoc.perl.org/perlrequick.html\">"
            "http://perldoc.perl.org/perlrequick.html</a> (kertaus),<br>\n"
            "<a href=\"http://perldoc.perl.org/perlre.html#Regular-Expressions\">"
            "http://perldoc.perl.org/perlre.html#Regular-Expressions</a>.<br>\n"
            "KUNHAN KERKEÄN KIRJOITAN TÄHÄN KOOSTEEN USEIMMIN"
            " KÄYTETYISTÄ MERKEISTÄ. <br>\n"
            "Kunhan tekstissä ei ole merkkejä {}[]()^$.|*+?\\ niin sanoja"
            " etsitään sanoina. Jos tarvitsee etsiä edellä mainittuja merkkejä"
            " tekstistä, niitä pitää edeltää pakomerkki \\ (esim. "
            " sanaa \"(q*w)\" etsitään kirjoittamalla \"\\(q\\*w\\)\"). <br>\n"
            "Kirjainten koolla ei ole väliä. Huomaa että tutkittava teksti"
            "on raakaa html -tekstiä tai vastaavaa.<br>\n"
            "Koko tekstin hakuun on lisäksi seuraavia komentoja:<br>\n"
            "Samalla rivillä olevat sanat etsitään järjestyksestä riippumatta "
            " jos ne ovat erotettu § merkillä."
            "Jos tekstistä pitää etsiä"
            " merkkiä '§', kirjoita se pakomerkillä '\\§'."
            "\"\\FILE\" korvautuu tarkistaessa tarkistettavan vastauksen tiedoston"
            " nimellä, ilman tunnistetta.\n"
            "Merkeillä \"(?#!)\" annetaan negatiivinen rivi: avainsanat "
            "täsmäätä, jos tätä riviä ei löydy ennen seuraavalla rivillä olevia "
            "sanoja. \"(?#!)\" täytyy olla rivin/sanan alussa, ja seuraavalla rivillä "
            "pitää olla tekstiä joka ei ala negaatio-merkinnällä jotta "
            "tarkistus toimii.";

    explicit AutoCommenter(QWidget *parent = nullptr);

    void setOldKeywords(QString text);

    QString getKeywords ();

    QTextEdit *_text;
};

#endif // AUTOCOMMENTER_H
