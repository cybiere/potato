#include "wikiinfo.hpp"

WikiInfo::WikiInfo(void)
{
    current = "";
    page = new QWebView;
    load = new QProgressBar;
    QVBoxLayout *vertLay = new QVBoxLayout();

    vertLay->addWidget(page);
    vertLay->addWidget(load);
    load->hide();

    setLayout(vertLay);
    /*
    Gérer les size policy
    setSizePolicy(QSizePolicy::Minimum);
    */
    page->load(QUrl("http://fr.m.wikipedia.org"));

    connect(page, SIGNAL(loadStarted()), this, SLOT(startLoad()));
    connect(page, SIGNAL(loadProgress(int)), this, SLOT(loading(int)));
    connect(page, SIGNAL(loadFinished(bool)), this, SLOT(endLoad(bool)));
}

void WikiInfo::search(QString artist)
{
    artist.replace(QChar(' '),"%20");
    if(artist != current)
    {
        current = artist;
        page->load(QUrl("http://fr.m.wikipedia.org/w/index.php?search=" + artist));
    }
}

void WikiInfo::startLoad()
{
    load->show();
}

void WikiInfo::loading(int val)
{
    load->setValue(val);
}

void WikiInfo::endLoad(bool)
{
    load->hide();
}
