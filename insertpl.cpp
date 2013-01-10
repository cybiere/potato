#include "insertpl.hpp"

/** @brief Constructeur du InsertPl */
InsertPl::InsertPl(QMutex* mutex,QTreeWidget* cur,QTreeWidget* search,QTreeWidget* lists)
{
    db =  dbManager::getInstance();
    mut = mutex;
    req = {0};
    plists = lists;
    current = cur;
    searchRes = search;
    waitlist = NULL;
}

/** @brief Méthode pour récupérer la liste des répertoires à scanner
 * @param wait : liste des répertoires à scanner
 */
void InsertPl::setParam(QString pl,QTreeWidgetItem * it,int niv,QTreeWidgetItem* wait)
{
    playlist = pl;
    item = it;
    niveau = niv;
    waitlist = wait;
}

/** @brief Code exécuté par le thread */
void InsertPl::run()
{
    insert(playlist,item,niveau);
    emit changestatusBar(tr("Enregistrement terminé"));
    return;
    exec();
}

/** @brief Méthode pour scanner les répertoires et sous-répertoires en ajoutant les musiques dans la bdd */
void InsertPl::insert(QString play,QTreeWidgetItem* it,int niv)
{
    //Traite différement si l'ajout provient de la recherche (pas les mêmes infos dans l'arbre)
    if(it->treeWidget() == searchRes)
    {
        QString path;
        QStringList song = db->getSong(it->text(0),it->text(2),it->text(1));

        path = song.takeAt(6);
        QTreeWidgetItem *songItem = new QTreeWidgetItem(QStringList(it->text(0)) << path);
        bool found = false;
        QTreeWidgetItem *pl;
        //recherche l'item de la playlist en question
        for(int k=0;k<plists->topLevelItemCount() && !found;++k)
        {
            if(plists->topLevelItem(k)->text(0) == play)
            {
                mut->lock();
                db->addSgToPl(path,play);
                pl = plists->topLevelItem(k);
                found = true;
                mut->unlock();
            }
        }
        if(!found)
        {
            //playlist pas dans la view mais dans base de données (vu qu'envoyée via QAction) => ERREUR
            std::cerr << "erreur : Playlist inexistante.search" << std::endl;
            exit(1);
        }
        pl->addChild(songItem);
        return;
    }


    //Définir la nature de l'item double cliqué (Artiste,Album,Chanson) --> ajouter playlist + tard
    //artiste
    if(niv== 2)
    {
        for(int i=0;i<it->childCount();++i)
            insert(play,it->child(i),1);
    }
    //album
    else if(niv== 1)
    {
        for(int j=0;j<it->childCount();++j)
        {

            insert(play,it->child(j),0);
        }
    }
    else
    {
        //rassemble les infos
        QString titre;
        QString path;
        //si l'appel vient de la playlist en cours, déjà tout a disposition dans les différents champs
        if( it->parent() == waitlist)
        {
            titre = it->text(0);
            path = it->text(6);

        }
        else if(it->treeWidget() == current)
        {
            titre = it->text(0);
            path = it->text(6);
        }
        //chanson
        else if(waitlist != NULL)
        {
            titre = it->text(0);
            path = it->text(6);
        }
        else
        {
            //sinon, on rassemble les infos (titre, artiste, album) pour récupérer le path
            titre = it->text(0);
            QString album = it->parent()->text(0);
            QString artiste = it->parent()->parent()->text(0);
            QStringList song = db->getSong(titre,album,artiste);
            path = song.takeAt(6);
        }
        //on a le path, le titre & le nom de l'item, on peut créer l'asso dans la db & afficher
        QTreeWidgetItem *songItem = new QTreeWidgetItem(QStringList(titre) << path);
        bool found = false;
        QTreeWidgetItem *pl;
        //retrouver l'item, cf plus haut
        for(int k=0;k<plists->topLevelItemCount() && !found;++k)
        {
            if(plists->topLevelItem(k)->text(0) == play)
            {
                mut->lock();
                db->addSgToPl(path,play);
                pl = plists->topLevelItem(k);
                found = true;
                mut->unlock();
            }
        }
        if(!found)
        {
            std::cerr << "erreur : Playlist inexistante." << std::endl;
            exit(1);
        }
        pl->addChild(songItem);
    }
}


