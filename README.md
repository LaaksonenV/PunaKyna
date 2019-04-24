HUOM 1.8! Uusimmassa versiossa settings.ini on siirretty omaan kansioonsa, johon kerääntyy muita vastaavia tiedostoja. Jos haluat säilyttää vanhat asetukset, siirrä vanha .ini tiedosto 'settings' kansioon, mutta HUOMaa, että aikaisempien tenttien asetukset eivät enää vastaa uuden version tarpeita, joten ne kannattaa poistaa settings.ini tiedostosta käsin

HUOM !!! Siirrän mac ja win versiot punakynästä omiin brancheihin, master-branch päivittyy sitten omalla ajallaan, mutta tuskin kukaan haluaa ladata molempien käyttöjärjestelmien tiedostoja samalle koneelle kuitenkaan... 
Tämäkin on vain väliaikainen ratkaisu, jaan tiedostot paremmin gitin latauskuntoon kunhan kerkiän selvittämään sen toiminnan

HUOM WINDOWS, käytössä on jotain ongelmia, jotka esiintyvät joillain koneilla, mutta ei toisilla. Jos esimerkisi pdf-vastaukset zoomautuvat liikaa, kokeile toisella koneella. Jos PK toimii oikein yhdessä koneessa mutta ei toisessa, niin pyydän kertomaan tästä, ja koneiden mahdollisista eroista. Ongelma korjataan jos sen syy saadaan joskus selville.

HUOM MAC, uusimmissa QT-versioissa on ongelmia tunnistaa ääkkösiä tiedostonimissä. Finderin kautta kirjoitetut ääkköset toimivat normaalisti, mutta terminaalin kautta (ja oletettavasti muista järjestelmistä) kirjoitetut tiedostonimet eivät näy QDirille.


# PunaKynä - Matematiikan tehtävien/Sähköisten tenttien automaattitarkastaja

PunaKynä on luotu helpottamaan sähköisissä tenttijärjestelmissä tehtyjä tenttejä, mikäli järjestelmät antavat tarkastajien ladata vastaukset lokaaliin kansioon. Samalla sen on tarkoitus toimia alustana tenttien automaattiselle tarkastamiselle. PunaKynä on osa diplomityötä sähköisten tenttien automaattitarkastamisesta <http://URN.fi/URN:NBN:fi:tty-201811012521>.

## PunaKynän ja sen osien kääntäminen (kunhan lähdekoodi on Gitissä)

Käyttöjärjestelmästä riippumatta Punakynä käyttää montaa ulkoista Qt-kirjastoa,
jotka täytyy asentaa erikseen ennen itse Punakynän kääntämistä paikallisesti.
Jos kaikki on mennyt hyvin repositoriota kloonatessa, ei käyttäjän kuitenkaan pitäisi joutua
asentamaan kuin seuraavat moduulit:

1. QtLabs PDF Module: <https://blog.qt.io/blog/2017/01/30/new-qtpdf-qtlabs-module/>
2. QtXlsx Writer: <https://github.com/VSRonin/QtXlsxWriter>

Qt-kirjastojen asennukseen vaaditaan tietysti ensin itse [Qt](https://www.qt.io/):n asentamista.


