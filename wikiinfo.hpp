#ifndef WIKIINFO_HPP
#define WIKIINFO_HPP

#include <QtWebKit>
#include <QtGui>

/** @class Classe de gestion du bloc "Wiki" permettant d'afficher les informations sur l'artiste de la chanson en cours */

class WikiInfo : public QWidget
{
    Q_OBJECT

    QString current;

public:
    WikiInfo(void);
    QWebView *page;
    QProgressBar *load;

public slots:
    void search(QString);
    void startLoad();
    void loading(int);
    void endLoad(bool);

};

#endif // WIKIINFO_HPP
