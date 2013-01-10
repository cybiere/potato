#include "wikiinfo.hpp"

/** @brief Constructeur du WikiInfo */
WikiInfo::WikiInfo(void)
{
    current = "";

    QWidget *wikiBlock = new QWidget;
    wiki = new QWebView;
    loadWiki = new QProgressBar;
    QVBoxLayout *wikiLay = new QVBoxLayout();

    wikiLay->addWidget(wiki);
    wikiLay->addWidget(loadWiki);
    loadWiki->hide();

    wikiBlock->setLayout(wikiLay);

    addTab(wikiBlock,QIcon(),tr("Wikipedia"));

    wiki->load(QUrl("http://fr.m.wikipedia.org"));

    connect(wiki, SIGNAL(loadStarted()), this, SLOT(startLoadWiki()));
    connect(wiki, SIGNAL(loadProgress(int)), this, SLOT(loadingWiki(int)));
    connect(wiki, SIGNAL(loadFinished(bool)), this, SLOT(endLoadWiki(bool)));


    QWidget *lyricsBlock = new QWidget;
    lyrics = new QWebView;
    loadLyrics = new QProgressBar;
    QVBoxLayout *lyricsLay = new QVBoxLayout();

    lyricsLay->addWidget(lyrics);
    lyricsLay->addWidget(loadLyrics);
    loadLyrics->hide();

    lyricsBlock->setLayout(lyricsLay);

    addTab(lyricsBlock,QIcon(),tr("Paroles"));

    lyrics->load(QUrl("http://lyrics.wikia.com/"));

    connect(lyrics, SIGNAL(loadStarted()), this, SLOT(startLoadLyrics()));
    connect(lyrics, SIGNAL(loadProgress(int)), this, SLOT(loadingLyrics(int)));
    connect(lyrics, SIGNAL(loadFinished(bool)), this, SLOT(endLoadLyrics(bool)));
}

/** @brief Slot pour charger la page correspondant à l'artiste
 * @param artist : artiste de la musique en lecture
 */
void WikiInfo::search(QStringList song)
{
    QString title = song.takeFirst();
    QString artist = song.takeFirst();
    QString art = artist;
    //remplacer des espaces par le %20
    artist.replace(QChar(' '),"%20");
    //on verifie que l'artiste est pas le même que celui de la page déjà chargée, inutile de recharger pour rien.
    if(artist != current)
    {
        current = artist;
        wiki->load(QUrl("http://fr.m.wikipedia.org/w/index.php?search=" + artist));
    }
    art.replace(QChar(' '),"_");
    title.replace(QChar(' '),"_");
    lyrics->load(QUrl("http://lyrics.wikia.com/" + art + ":" + title));

}

/** @brief Slot pour afficher la barre de chargement au début de celui ci*/
void WikiInfo::startLoadWiki()
{
    loadWiki->show();
}

/** @brief Slot pour incrémenter la barre de chargement
 * @param val : valeur du chargement
 */
void WikiInfo::loadingWiki(int val)
{
    loadWiki->setValue(val);
}

/** @brief Slot pour cacher la barre de chargement à la fin de celui ci */
void WikiInfo::endLoadWiki(bool)
{
    loadWiki->hide();
}


/** @brief Slot pour afficher la barre de chargement au début de celui ci*/
void WikiInfo::startLoadLyrics()
{
    loadLyrics->show();
}

/** @brief Slot pour incrémenter la barre de chargement
 * @param val : valeur du chargement
 */
void WikiInfo::loadingLyrics(int val)
{
    loadLyrics->setValue(val);
}

/** @brief Slot pour cacher la barre de chargement à la fin de celui ci */
void WikiInfo::endLoadLyrics(bool)
{
    loadLyrics->hide();
}
