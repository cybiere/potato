#include "thread.hpp"

/** @brief Constructeur du thread */
Thread::Thread()
{
    db =  dbManager::getInstance();
    req = {0};
}

/** @brief Méthode pour récupérer la liste des répertoires à scanner
 * @param wait : liste des répertoires à scanner
 */
void Thread::setParam(QStringList wait)
{
    waitList = wait;
}

/** @brief Code exécuté par le thread */
void Thread::run()
{
    link();
    exec();
}

/** @brief Méthode pour scanner les répertoires et sous-répertoires en ajoutant les musiques dans la bdd */
void Thread::link()
{
    QString path;
    //pour chaque dossier de la file d'attente
    while(waitList.count() != 0)
    {
        path = waitList.takeFirst();
        QDir dir(path);
        QStringList nameFilters;
        //Génère le filtre des noms pour considérer uniquement les fichiers mp3 & les dossiers
        nameFilters << "*.mp3";
        dir.setNameFilters(nameFilters);
        //Génère le filtre pour afficher tous les dossiers sauf . et ..
        dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

        int nbItem = dir.count();

        //déterminer si l'entrée est un dossier ou un mp3
        QRegExp rx("*.mp3");
        rx.setPatternSyntax(QRegExp::Wildcard);
        rx.setCaseSensitivity(Qt::CaseInsensitive);

        for(int i=0;i<nbItem;++i)
        {
            if(rx.exactMatch(dir[i]))
            {
                //mp3 = on dort un peu pour être sur de pas aller plus vite que la base de données
                req.tv_sec = 0;
                req.tv_nsec = 200 * 1000000L;
                nanosleep(&req, (struct timespec *)NULL);
                //Et on ajoute
                db->addSong(path + "/" + dir[i]);
            }
            else
            {
                //dossier : on l'ajoute a la fin de la waitlist, il sera traité plus tard
                waitList << path + "/" + dir[i];
            }
        }
        //on a scanné un dossier, on rafraichit la bibliothèque pour que l'utilisateur puisse commencer à écouter les chansons déjà scannées sans attendre la fin
        emit refresh();
    }
    //modification du texte de la statusBar
    emit finish(tr("Regénération terminée."));
}


