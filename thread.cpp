#include "thread.hpp"

Thread::Thread()
{
    db =  dbManager::getInstance();
    req = {0};
}

void Thread::setParam(QStringList wait)
{
    waitList = wait;
}

void Thread::run()
{
    link();
    exec();
}

void Thread::link()
{
    QString path;
    while(waitList.count() != 0)
    {
        path = waitList.takeFirst();
        QDir dir(path);
        QStringList nameFilters;
        //Génère le filtre des noms pour considérer uniquement les fichiers d'une certaine extension
        nameFilters << "*.mp3";
        dir.setNameFilters(nameFilters);
        //Génère le filtre pour afficher tous les dossiers sauf . et ..
        dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
        int nbItem = dir.count();

        //RegEx très (trop?) simple pour différencier dossiers et fichiers (de l'extension donnée) -> doit y avoir meilleure méthode
        //QRegExp rx("*.*");
        QRegExp rx("*.mp3");
        rx.setPatternSyntax(QRegExp::Wildcard);
        rx.setCaseSensitivity(Qt::CaseInsensitive);

        for(int i=0;i<nbItem;++i)
        {
            if(rx.exactMatch(dir[i]))
            {
                req.tv_sec = 0;
                req.tv_nsec = 200 * 1000000L;
                nanosleep(&req, (struct timespec *)NULL);
                //Traitement pour les fichiers
                db->addSong(path + "/" + dir[i]);
            }
            else
            {
                //Scan des sous-dossiers.
                waitList << path + "/" + dir[i];
            }
        }
        emit refresh();
    }
    emit finish(tr("Regénération terminée."));
}


