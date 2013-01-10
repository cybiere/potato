#ifndef WIKIINFO_HPP
#define WIKIINFO_HPP

#include <QtWebKit>
#include <QtGui>

/** @class Classe de gestion du bloc "Wiki" permettant d'afficher les informations sur la chanson en cours */

class WikiInfo : public QTabWidget
{
    Q_OBJECT

    QString current;

public:
    WikiInfo(void);
    QWebView *wiki;
    QWebView *lyrics;
    QProgressBar *loadWiki;
    QProgressBar *loadLyrics;

public slots:
    void search(QStringList);
    void startLoadWiki();
    void startLoadLyrics();
    void loadingWiki(int);
    void loadingLyrics(int);
    void endLoadWiki(bool);
    void endLoadLyrics(bool);

};

#endif // WIKIINFO_HPP
