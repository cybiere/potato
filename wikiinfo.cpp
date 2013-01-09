#include "wikiinfo.hpp"

/** @brief Constructeur du WikiInfo */
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

    page->load(QUrl("http://fr.m.wikipedia.org"));

    connect(page, SIGNAL(loadStarted()), this, SLOT(startLoad()));
    connect(page, SIGNAL(loadProgress(int)), this, SLOT(loading(int)));
    connect(page, SIGNAL(loadFinished(bool)), this, SLOT(endLoad(bool)));
}

/** @brief Slot pour charger la page correspondant à l'artiste
 * @param artist : artiste de la musique en lecture
 */
void WikiInfo::search(QString artist)
{
    //remplacer des espaces par le %20
    artist.replace(QChar(' '),"%20");
    //on verifie que l'artiste est pas le même que celui de la page déjà chargée, inutile de recharger pour rien.
    if(artist != current)
    {
        current = artist;
        page->load(QUrl("http://fr.m.wikipedia.org/w/index.php?search=" + artist));
    }
}

/** @brief Slot pour afficher la barre de chargement au début de celui ci*/
void WikiInfo::startLoad()
{
    load->show();
}

/** @brief Slot pour incrémenter la barre de chargement
 * @param val : valeur du chargement
 */
void WikiInfo::loading(int val)
{
    load->setValue(val);
}

/** @brief Slot pour cacher la barre de chargement à la fin de celui ci */
void WikiInfo::endLoad(bool)
{
    load->hide();
}
