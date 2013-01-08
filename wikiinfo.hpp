#ifndef WIKIINFO_HPP
#define WIKIINFO_HPP

#include <QtWebKit>
#include <QtGui>

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
