HUOM WINDOWS; Jostain syystä pdf-ikkuna zoomaa vastaukset joskus väärin. Tähän on tehty korjaava asetus yläpalkin "Näkymän korjaus" valikossa.

# PunaKynä - Matematiikan tehtävien/Sähköisten tenttien tarkastaja

PunaKynä on luotu helpottamaan sähköisissä tenttijärjestelmissä tehtyjä tenttejä, mikäli järjestelmät antavat tarkastajien ladata vastaukset lokaaliin kansioon. Samalla sen on tarkoitus toimia alustana tenttien automaattiselle tarkastamiselle. PunaKynä on osa diplomityötä sähköisten tenttien automaattitarkastamisesta <http://URN.fi/URN:NBN:fi:tty-201811012521>.

PunaKynän uusimman version voi ladata sivluta <https://github.com/LaaksonenV/PunaKyna/releases/tag/v1.8.0.1b>.

PunaKynä vaatii vielä testaamista, virheitä saattaa tulla vastaan, ja näistä on toivottavaa että ilmoitatte mahdollisimman pian, jotta korjaukset saadaan aloitettua.

## PunaKynän ja sen osien kääntäminen

Käyttöjärjestelmästä riippumatta Punakynä käyttää montaa ulkoista Qt-kirjastoa,
jotka täytyy asentaa erikseen ennen itse Punakynän kääntämistä paikallisesti.
Jos kaikki on mennyt hyvin repositoriota kloonatessa, ei käyttäjän kuitenkaan pitäisi joutua
asentamaan kuin seuraavat moduulit:

1. QtLabs PDF Module: <https://blog.qt.io/blog/2017/01/30/new-qtpdf-qtlabs-module/>
2. QtXlsx Writer: <https://github.com/VSRonin/QtXlsxWriter>

Qt-kirjastojen asennukseen vaaditaan tietysti ensin itse [Qt](https://www.qt.io/):n asentamista.


