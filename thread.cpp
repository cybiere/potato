#include "thread.hpp"

Thread::Thread()
{
    mut=new QMutex();
    db =  dbManager::getInstance();
}

void Thread::setParam(QTreeWidget* bib,QString str)
{
    path = str;
    biblio = bib;
}

void Thread::run()
{
    link(path);
    exec();
}

void Thread::link(QString path)
{
   mut->lock();
    std::cout << path.toStdString() << std::endl;
    QDir dir(path);
    QStringList nameFilters;
    //Génère le filtre des noms pour considérer uniquement les fichiers d'une certaine extension
    nameFilters << "*.mp3" << "*.ogg" << "*.wav" << ".m4a";
    dir.setNameFilters(nameFilters);
    //Génère le filtre pour afficher tous les dossiers sauf . et ..
    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    int nbItem = dir.count();

    //RegEx très (trop?) simple pour différencier dossiers et fichiers (de l'extension donnée) -> doit y avoir meilleure méthode
    QRegExp rx("*.*");
    rx.setPatternSyntax(QRegExp::Wildcard);

    for(int i=0;i<nbItem;++i)
    {
        if(rx.exactMatch(dir[i]))
        {
            sleep(1);
            //Traitement pour les fichiers
            insertSong(db->addSong(path + "/" + dir[i]));

        }
        else
        {
            mut->unlock();
            //Scan des sous-dossiers.
            link(path + "/" + dir[i]);
        }
    }


    emit complete();

}

void Thread::insertSong(QStringList song)
{
    QString title = song.takeFirst();
    QString artist = song.takeFirst();
    QString album = song.takeFirst();
    bool exists = false;
    QTreeWidgetItem *artistItem;
    QTreeWidgetItem *albumItem;

    for(int i=0;i < biblio->topLevelItemCount() && !exists;++i)
    {
        if(biblio->topLevelItem(i)->text(0) == artist)
        {
            artistItem = biblio->topLevelItem(i);
            exists = true;
        }
    }
    if(!exists)
    {
        artistItem = new QTreeWidgetItem(QStringList(artist));
        biblio->addTopLevelItem(artistItem);
    }
    exists = false;

    for(int j=0;j < artistItem->childCount() && !exists;++j)
    {
        if(artistItem->child(j)->text(0) == album)
        {
            albumItem = artistItem->child(j);
            exists = true;
        }
    }
    if(!exists)
    {
        albumItem = new QTreeWidgetItem(QStringList(album));
        artistItem->addChild(albumItem);
    }
    exists = false;

    for(int k=0;k < albumItem->childCount() && !exists;++k)
    {
        if(albumItem->child(k)->text(0) == title)
        {
            exists = true;
        }
    }
    if(!exists)
    {
        albumItem->addChild(new QTreeWidgetItem(QStringList(title)));
    }
}



