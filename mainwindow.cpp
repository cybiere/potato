#include "mainwindow.hpp"
#include "wikiinfo.hpp"
#include <fstream>

/** @brief Constructeur d'une MainWindow */
MainWindow::MainWindow()
{
    db = dbManager::getInstance();
    thr = new Thread;
    media = new Phonon::MediaObject();
    media->setTickInterval(1000);
    output = new Phonon::AudioOutput(Phonon::MusicCategory);
    createPath(media,output);

    ///////////////     Connect Thread      ////////////
    connect(thr,SIGNAL(refresh()),this,SLOT(regenBiblio()));
    connect(thr,SIGNAL(finish(QString)),this,SLOT(changeStatus(QString)));

    ///////////////     Connect media      ////////////
    connect(media,SIGNAL(totalTimeChanged(qint64)),this,SLOT(upTimeTot(qint64)));
    connect(media,SIGNAL(tick(qint64)),this,SLOT(incrTimeCur(qint64)));
    connect(media,SIGNAL(finished()),this,SLOT(songEnd()));

    ///////////////      Connect App      ////////////
    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(saveCurrent()));

    playing = NULL;

    ////////////////////    Menu    ////////////////////
    //Menu Fichier
    QMenu *menuFichier = menuBar()->addMenu(tr("Fichier"));


        QAction *actionClearCur = new QAction(QIcon(":/ico/clearCur.png"),tr("Vider la lecture en cours"),this);
            connect(actionClearCur,SIGNAL(triggered()),this,SLOT(clearCurrent()));
        menuFichier->addAction(actionClearCur);
        QAction *actionSaveCur = new QAction(QIcon(":/ico/saveCur.png"),tr("Sauvegarder la lecture en cours"),this);
            connect(actionSaveCur,SIGNAL(triggered()),this,SLOT(currentToPl()));
        menuFichier->addAction(actionSaveCur);
        menuFichier->addSeparator();
        QAction *actionQuit = new QAction(tr("Quitter"),this);
            connect(actionQuit,SIGNAL(triggered()),qApp,SLOT(quit()));
        menuFichier->addAction(actionQuit);

    //Menu Lecture
    QMenu *menuLecture = menuBar()->addMenu(tr("Lecture"));

        actionPlay = new QAction(QIcon(":/ico/play.png"),tr("Lecture"), this);
            connect(actionPlay,SIGNAL(triggered()),this,SLOT(play()));
        actionStop = new QAction(QIcon(":/ico/stop.png"),tr("Stop"), this);
            connect(actionStop,SIGNAL(triggered()),this,SLOT(stop()));
        actionPrev = new QAction(QIcon(":/ico/prev.png"),tr("Précédent"), this);
            connect(actionPrev,SIGNAL(triggered()),this,SLOT(prev()));
        actionNext = new QAction(QIcon(":/ico/next.png"),tr("Suivant"), this);
            connect(actionNext,SIGNAL(triggered()),this,SLOT(next()));
        actionLoop = new QAction(QIcon(":/ico/loop_off.png"),tr("Répéter chanson"), this);
            connect(actionLoop,SIGNAL(triggered()),this,SLOT(loop()));
            loopState = 0;

        menuLecture->addAction(actionPlay);
        menuLecture->addAction(actionStop);
        menuLecture->addSeparator();
        menuLecture->addAction(actionPrev);
        menuLecture->addAction(actionNext);
        menuLecture->addSeparator();
        menuLecture->addAction(actionLoop);
    ////////////////////    Ctrl    ////////////////////

        //Barre de controle
        //Actions à mettre en place
        QToolBar *ctrlBar = addToolBar("Controles");
        ctrlBar->setFloatable(false);
        ctrlBar->setMovable(false);

        ctrlBar->addAction(actionPlay);
        ctrlBar->addAction(actionStop);
        ctrlBar->addSeparator();

        ctrlBar->addAction(actionPrev);
        ctrlBar->addAction(actionNext);
        ctrlBar->addSeparator();

        timeCurrent = new QLabel("--:--");
        timeTotal = new QLabel("--:--");


        ctrlBar->addWidget(timeCurrent);

        Phonon::SeekSlider *slider = new Phonon::SeekSlider(media);
        ctrlBar->addWidget(slider);

        ctrlBar->addWidget(timeTotal);
        ctrlBar->addSeparator();

        ctrlBar->addAction(actionLoop);
        ctrlBar->addAction(actionClearCur);
        ctrlBar->addAction(actionSaveCur);
        ctrlBar->addSeparator();

        Phonon::VolumeSlider *volumeSlider = new Phonon::VolumeSlider(output);
            volumeSlider->setMaximumWidth(180);
            ctrlBar->addWidget(volumeSlider);


    ////////////////////    Main    ////////////////////
    QSplitter *main = new QSplitter;

    //leftDock
    QWidget *leftDock = new QWidget;
    leftDock->setMinimumWidth(200);
    leftDock->setMaximumWidth(300);
        QVBoxLayout *leftLay = new QVBoxLayout;
        leftLay->setMargin(0);
        //Recherche
        searchBlock = new QWidget;
            QVBoxLayout *searchLay = new QVBoxLayout;
                searchField = new QLineEdit;
                connect(searchField,SIGNAL(textChanged(QString)),this,SLOT(changeSearch(QString)));

                searchLay->addWidget(searchField);
                searchRes = new QTreeWidget;
                    searchRes->header()->close();
                    searchRes->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(searchRes,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextSearch(QPoint)));
                    connect(searchRes,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(addSearchToCurrent(QTreeWidgetItem*,int)));
                searchLay->addWidget(searchRes);
            searchBlock->setLayout(searchLay);
            searchBlock->hide();

        //Bibliothèque
        biblio = new QTreeWidget;
            //Pas de headers, pas d'expand au double clic
            biblio->header()->close();
            biblio->setExpandsOnDoubleClick(false);
            biblio->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(biblio,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextBiblio(QPoint)));
            connect(biblio,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(addToCurrent(QTreeWidgetItem*,int)));

        //Playlists
        plBlock = new QWidget;
            QVBoxLayout *plLay = new QVBoxLayout;
                plLay->setMargin(0);
                plists = new QTreeWidget;
                    plists->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(plists,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextPl(QPoint)));
                    //Pas de headers, pas d'expand au double clic
                    plists->header()->close();
                    plists->setExpandsOnDoubleClick(false);
                    connect(plists,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(addToCurrent(QTreeWidgetItem*,int)));
                plLay->addWidget(plists);
                //Gestion des listes de lecture
                QHBoxLayout *ctrlLay = new QHBoxLayout;
                    ctrlLay->setMargin(0);
                    //ajouter une liste de lecture
                    QPushButton *addPlist = new QPushButton(QIcon(":/ico/add.png"),"");
                        connect(addPlist,SIGNAL(clicked()),this,SLOT(addPl()));
                    //retirer la liste de lecture sélectionnée
                    QPushButton *delPlist = new QPushButton(QIcon(":/ico/del.png"),"");
                        connect(delPlist,SIGNAL(clicked()),this,SLOT(delPl()));
                    ctrlLay->addWidget(addPlist);
                    ctrlLay->addWidget(delPlist);
                plLay->addLayout(ctrlLay);
            plBlock->hide();
            plBlock->setLayout(plLay);


        //Options
        options = new QWidget;
            QVBoxLayout *optionsLay = new QVBoxLayout;
                optionsLay->setMargin(0);
                options->setLayout(optionsLay);
                options->hide();

                //Gestion des dossiers sources de chansons
                QHBoxLayout *srcDirLay = new QHBoxLayout;
                    srcDirLay->setMargin(0);
                    //liste pour afficher
                    srcDirList = new QListWidget;
                    srcDirList->addItems(db->getSrcDirs());
                    srcDirList->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(srcDirList,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextSrc(QPoint)));
                    QVBoxLayout *srcDirActionLay = new QVBoxLayout;
                        srcDirActionLay->setMargin(0);
                        //trois boutons :
                        //ajouter une source
                        QPushButton *addSrcDir = new QPushButton(QIcon(":/ico/add.png"),"");
                            connect(addSrcDir,SIGNAL(clicked()),this,SLOT(addSourceDir()));
                        //retirer la source sélectionnée
                        QPushButton *delSrcDir = new QPushButton(QIcon(":/ico/del.png"),"");
                            connect(delSrcDir,SIGNAL(clicked()),this,SLOT(delSourceDir()));
                        //rafraichier la bibliothèque
                        QPushButton *refBiblio = new QPushButton(QIcon(":/ico/ref.png"),"");
                            connect(refBiblio,SIGNAL(clicked()),this,SLOT(refresh()));
                        srcDirActionLay->addWidget(addSrcDir);
                        srcDirActionLay->addWidget(delSrcDir);
                        srcDirActionLay->addWidget(refBiblio);
                        //QWidget vide pour aligner les trois boutons vers le haut
                        srcDirActionLay->addWidget(new QWidget);
                    srcDirLay->addWidget(srcDirList);
                    srcDirLay->addLayout(srcDirActionLay);
                optionsLay->addLayout(srcDirLay);

        //bouton pour afficher la recherche
        QPushButton *search = new QPushButton(tr("Recherche"));
            search->setFlat(true);
            connect(search,SIGNAL(clicked()),this,SLOT(showSearch()));

        //bouton pour afficher la bibliothèque
        QPushButton *bibl = new QPushButton(tr("Bibliothèque"));
            bibl->setFlat(true);
            connect(bibl,SIGNAL(clicked()),this,SLOT(showBiblio()));

        //bouton pour afficher les playlists
        QPushButton *playl = new QPushButton(tr("Listes de lecture"));
            playl->setFlat(true);
            connect(playl,SIGNAL(clicked()),this,SLOT(showPlists()));

        //bouton pour afficher les options
        QPushButton *opt = new QPushButton(tr("Options"));
            opt->setFlat(true);
            connect(opt,SIGNAL(clicked()),this,SLOT(showOptions()));

        QFrame *separator = new QFrame();
            separator->setFrameShape(QFrame::HLine);
            separator->setFrameShadow(QFrame::Sunken);
        QFrame *separator2 = new QFrame();
            separator2->setFrameShape(QFrame::HLine);
        separator2->setFrameShadow(QFrame::Sunken);
        QFrame *separator3 = new QFrame();
            separator3->setFrameShape(QFrame::HLine);
            separator3->setFrameShadow(QFrame::Sunken);

        leftLay->addWidget(search);
        leftLay->addWidget(searchBlock);
        leftLay->addWidget(separator);
        leftLay->addWidget(bibl);
        leftLay->addWidget(biblio);
        leftLay->addWidget(separator2);
        leftLay->addWidget(playl);
        leftLay->addWidget(plBlock);
        leftLay->addWidget(separator3);
        leftLay->addWidget(opt);
        leftLay->addWidget(options);
    leftDock->setLayout(leftLay);

    //mainZone
    //QTreeWidget des chansons de la liste "lecture en cours"
    current = new QTreeWidget;
        QTreeWidgetItem *headers = new QTreeWidgetItem(QStringList(tr("Titre")) << tr("Auteur") << tr("Album") << tr("Genre") << tr("Jouée") << tr("Note") << tr("Path"));
        current->setHeaderItem(headers);
        current->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(current,SIGNAL(itemClicked(QTreeWidgetItem *,int)),this,SLOT(changeDockInfo(QTreeWidgetItem *,int)));
        connect(current,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextCurrent(QPoint)));
        connect(current,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(selectedSong(QTreeWidgetItem*,int)));

    //rightDock
    //Cover, wikipédia
    QWidget *rightDock = new QWidget;
    rightDock->setMinimumWidth(200);
    rightDock->setMaximumWidth(300);
        QVBoxLayout *rightLay = new QVBoxLayout;
        rightLay->setMargin(0);


    QPushButton *cov = new QPushButton(tr("Pochette"));
        cov->setFlat(true);
        //connect(cov,SIGNAL(clicked()),this,SLOT(showCover()));
    QWidget *cover = new QWidget;

    QFrame *separator4 = new QFrame();
        separator4->setFrameShape(QFrame::HLine);
        separator4->setFrameShadow(QFrame::Sunken);

    WikiInfo *wikiView = new WikiInfo;
    connect(this,SIGNAL(changeWikiInfo(QStringList)),wikiView,SLOT(search(QStringList)));
    rightLay->addWidget(wikiView);
    rightLay->addWidget(separator4);
    rightLay->addWidget(cov);
    rightLay->addWidget(cover);

    rightDock->setLayout(rightLay);




    main->addWidget(leftDock);
    main->addWidget(current);
    main->addWidget(rightDock);

    //les différentes parties ne peuvent pas être réduites jusqu'à disparaître
    main->setCollapsible(0,false);
    main->setCollapsible(1,false);
    main->setCollapsible(2,false);
    setCentralWidget(main);

    /////////////////    Barre de statut    //////////////////
    statusBar = new QStatusBar(this);
        statusBar->setSizeGripEnabled(false);
        statusBar->showMessage(tr("Bibliothèque à jour"));
    setStatusBar(statusBar);


    regenBiblio();
    biblio->sortItems(0,Qt::AscendingOrder);
    regenPlaylists();
    loadCurrent();
}

/** @brief Méthode pour changer la page du block Wiki
 * @param item : item sélectionner pour obtenir l'artiste à charger.
 */
void MainWindow::changeDockInfo(QTreeWidgetItem *item,int)
{
    QStringList song = QStringList(item->text(0)) << item->text(1) << item->text(2);
    emit changeWikiInfo(song);
}

/** @brief Méthode pour la recherche dans la bibliothèque
 * @param searchTerm : terme pour la recherche
 */
void MainWindow::changeSearch(QString searchTerm)
{
    QRegExp rx("*" + searchTerm + "*");
    rx.setPatternSyntax(QRegExp::Wildcard);
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    //Masque tous les éléments de searchRes ne correspondant pas à la recherche, affiche les autres.
    for(int i=0;i<searchRes->topLevelItemCount();++i)
    {
        if(!(rx.exactMatch(searchRes->topLevelItem(i)->text(0)) || rx.exactMatch(searchRes->topLevelItem(i)->text(1)) || rx.exactMatch(searchRes->topLevelItem(i)->text(2))))
            searchRes->topLevelItem(i)->setHidden(true);
        else
            searchRes->topLevelItem(i)->setHidden(false);
    }
}

/** @brief Méthode pour gérer le menu déroulant dans le bloc current
 * @param pos : position de l'object cliqué
 */
void MainWindow::contextCurrent(QPoint pos)
{
    //n'affiche pas le menu si aucun item n'est sélectionné (current vide par exemple)
    if(current->currentItem() == NULL)
        return;
    QMenu *context = new QMenu();
    QAction *titre = new QAction(current->currentItem()->text(0),this);
    titre->setEnabled(false);
    context->addAction(titre);
    QMenu *playlists = context->addMenu(tr("Ajouter à une liste de lecture"));
    for(int i=0;i<plists->topLevelItemCount();++i)
    {
        QAction *action = new QAction(plists->topLevelItem(i)->text(0),this);
            connect(action,SIGNAL(triggered()),this,SLOT(addCurrentToPl()));
        playlists->addAction(action);
    }
    context->exec(current->mapToGlobal(pos));
}

/** @brief Méthode pour gérer le menu déroulant dans le bloc Rechercher
 * @param pos : position de l'object cliqué
 */
void MainWindow::contextSearch(QPoint pos)
{
    if(searchRes->currentItem() == NULL)
        return;
    QMenu *context = new QMenu();
    //rappelle le titre, l'artiste et l'album de la chanson sélectionnée
    QAction *titre = new QAction(searchRes->currentItem()->text(0),this);
    titre->setEnabled(false);
    QAction *album = new QAction(searchRes->currentItem()->text(1),this);
    album->setEnabled(false);
    QAction *artiste = new QAction(searchRes->currentItem()->text(2),this);
    artiste->setEnabled(false);
    context->addAction(titre);
    context->addAction(album);
    context->addAction(artiste);
    QMenu *playlists = context->addMenu(tr("Ajouter à une liste de lecture"));
    for(int i=0;i<plists->topLevelItemCount();++i)
    {
        QAction *action = new QAction(plists->topLevelItem(i)->text(0),this);
            connect(action,SIGNAL(triggered()),this,SLOT(addSearchToPl()));
        playlists->addAction(action);
    }
    context->exec(searchRes->mapToGlobal(pos));
}

/** @brief Méthode pour gérer le menu déroulant dans le bloc Bibliothèque
 * @param pos : position de l'object cliqué
 */
void MainWindow::contextBiblio(QPoint pos)
{
    QMenu *context = new QMenu();
    QAction *actionPlay = new QAction(QIcon(":/ico/play.png"),tr("Lire"),this);
        connect(actionPlay,SIGNAL(triggered()),this,SLOT(playSelected()));
        if(biblio->currentItem()==NULL)
            actionPlay->setEnabled(false);
    context->addAction(actionPlay);
    QMenu *playlists = context->addMenu(tr("Ajouter à une liste de lecture"));
    for(int i=0;i<plists->topLevelItemCount();++i)
    {
        QAction *action = new QAction(plists->topLevelItem(i)->text(0),this);
            connect(action,SIGNAL(triggered()),this,SLOT(addBiblioToPl()));
        playlists->addAction(action);
    }
    context->exec(biblio->mapToGlobal(pos));
}

/** @brief Méthode pour gérer le menu déroulant dans le bloc des dossiers sources
 * @param pos : position de l'object cliqué
 */
void MainWindow::contextSrc(QPoint pos)
{
    QMenu *context = new QMenu();
    QAction *actionAdd = new QAction(QIcon(":/ico/add.png"),tr("Ajouter") + "...",this);
        connect(actionAdd,SIGNAL(triggered()),this,SLOT(addSourceDir()));
    context->addAction(actionAdd);
    QAction *actionDel = new QAction(QIcon(":/ico/del.png"),tr("Supprimer"),this);
        connect(actionDel,SIGNAL(triggered()),this,SLOT(delSourceDir()));
        if(srcDirList->currentItem()==NULL)
            actionDel->setEnabled(false);
    context->addAction(actionDel);
    QAction *actionRef = new QAction(QIcon(":/ico/ref.png"),tr("Rafraîchir"),this);
        connect(actionRef,SIGNAL(triggered()),this,SLOT(refresh()));
    context->addAction(actionRef);
    context->exec(srcDirList->mapToGlobal(pos));
}

/** @brief Méthode pour gérer le menu déroulant du bloc Listes de Lecture
 * @param pos : position de l'object cliqué
 */
void MainWindow::contextPl(QPoint pos)
{
    QMenu *context = new QMenu();
    QAction *actionAdd = new QAction(QIcon(":/ico/add.png"),tr("Ajouter") + "...",this);
        connect(actionAdd,SIGNAL(triggered()),this,SLOT(addPl()));
    context->addAction(actionAdd);
    QAction *actionDel = new QAction(QIcon(":/ico/del.png"),tr("Supprimer"),this);
        connect(actionDel,SIGNAL(triggered()),this,SLOT(delPl()));
        if(plists->currentItem()==NULL)
            actionDel->setEnabled(false);
    context->addAction(actionDel);
    context->exec(plists->mapToGlobal(pos));
}

/** @brief Méthode pour ajouter un résultat de la recherche dans une playlist (depuis le menu déroulant)*/
void MainWindow::addSearchToPl()
{
    (static_cast<QAction *>(sender())->text(),searchRes->currentItem(),0);
}

/** @brief Méthode pour ajouter le contenu de la playlist courante dans une playlist (depuis le menu déroulant) */
void MainWindow::addCurrentToPl()
{
    insertPl(static_cast<QAction *>(sender())->text(),current->currentItem(),0);
}

/** @brief Méthode pour ajouter des musiques de la bibliothèque à une playlist (depuis le menu déroulant)*/
void MainWindow::addBiblioToPl()
{
    //Niveau : 0 : chanson, 1 : album, 2 : artiste
    int niveau;
    QTreeWidgetItem *item = biblio->currentItem();
    if(item == NULL)
        return;

    //Définir la nature de l'item double cliqué (Artiste,Album,Chanson) --> ajouter playlist + tard
    //pas d'item parent = artiste
    if(item->parent() == NULL)
        niveau = 2;
    //pas d'item enfant = chanson
    else if(item->childCount() == 0)
        niveau = 0;
    //a la fois parent et enfant(s) = album
    else
        niveau = 1;

    insertPl(static_cast<QAction *>(sender())->text(),item,niveau);
}

/** @brief Méthode pour insérer un item dans une playlist
 * @param playlist : playlist dans laquelle on insère.
 * @param item : item à insérer
 * @param niveau : niveau du QTreeWidgetItem :  0 : chanson, 1 : album, 2 : artiste
 */
void MainWindow::insertPl(QString playlist,QTreeWidgetItem *item,int niveau)
{
    //Traite différement si l'ajout provient de la recherche (pas les mêmes infos dans l'arbre)
    if(item->treeWidget() == searchRes)
    {
        QString path;
        QStringList song = db->getSong(item->text(0),item->text(2),item->text(1));

        path = song.takeAt(6);
        QTreeWidgetItem *songItem = new QTreeWidgetItem(QStringList(item->text(0)) << path);
        bool found = false;
        QTreeWidgetItem *pl;
        //recherche l'item de la playlist en question
        for(int k=0;k<plists->topLevelItemCount() && !found;++k)
        {
            if(plists->topLevelItem(k)->text(0) == playlist)
            {
                db->addSgToPl(path,playlist);
                pl = plists->topLevelItem(k);
                found = true;
            }
        }
        if(!found)
        {
            //playlist pas dans la view mais dans base de données (vu qu'envoyée via QAction) => ERREUR
            std::cerr << "erreur : Playlist inexistante." << std::endl;
            exit(1);
        }
        pl->addChild(songItem);
        return;
    }


    //Définir la nature de l'item double cliqué (Artiste,Album,Chanson) --> ajouter playlist + tard
    //artiste
    if(niveau == 2)
    {
        for(int i=0;i<item->childCount();++i)
            insertPl(playlist,item->child(i),1);
    }
    //album
    else if(niveau == 1)
    {
        for(int j=0;j<item->childCount();++j)
            insertPl(playlist,item->child(j),0);
    }
    //chanson
    else
    {
        //rassemble les infos
        QString titre;
        QString path;
        //si l'appel vient de la playlist en cours, déjà tout a disposition dans les différents champs
        if(item->treeWidget() == current)
        {
            titre = item->text(0);
            path = item->text(6);
        }
        else
        {
            //sinon, on rassemble les infos (titre, artiste, album) pour récupérer le path
            titre = item->text(0);
            QString album = item->parent()->text(0);
            QString artiste = item->parent()->parent()->text(0);
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
            if(plists->topLevelItem(k)->text(0) == playlist)
            {
                db->addSgToPl(path,playlist);
                pl = plists->topLevelItem(k);
                found = true;
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

/** @brief Slot pour ajouter la musique à la playlist courante et le jouer */
void MainWindow::playSelected()
{
    if(biblio->currentItem() != NULL)
    {
        addToCurrent(biblio->currentItem(),0);
        play();
    }
}

/** @brief Slot pour ajouter un répertoire aux répertoires à scanner */
void MainWindow::addSourceDir()
{
    //QFiledialog pour choisir le dossier en question, puis ajout à la base de données et à l'interface
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choisir un dossier"), getenv("HOME"));
    //Test pour éviter les erreurs si l'utilisateur annule le QFileDialog
    if(dir == "")
        return;
    db->addSrc(dir);
    srcDirList->addItem(dir);
}

/** @brief Slot pour supprimer un répertoire dans la liste des répertoires à scanner */
void MainWindow::delSourceDir()
{
    //rien de sélectionné, rien à supprimer !
    if(srcDirList->currentItem() == NULL)
        return;
    //suppression de l'élément courant dans la base de données puis dans l'interface
    db->delSrc(srcDirList->currentItem()->text());
    srcDirList->takeItem(srcDirList->currentRow());
}

/** @brief Slot pour lister les répertoires à scanner et envoyer la liste au thread pour l'import */
void MainWindow::refresh()
{
    //waitlist = ensemble des dossiers que le thread doit scanner.
    //On lui fourgue une fois et après il fait son boulot tranquille
    QStringList waitList;

    int nbItems = srcDirList->count();
    statusBar->showMessage(tr("Importation en cours..."));
    for(int i=0;i<nbItems;++i)
    {
        waitList << srcDirList->item(i)->text();
    }

    //on passe la file d'attente au thread et on le lance
    thr->setParam(waitList);
    thr->start();
}

/** @brief Slot pour regénérer le QTreeWidgetItem de la bibliothèque */
void MainWindow::regenBiblio()
{
    //On récupère la liste de toutes les chansons auprès de la db (une chanson = 1 QStringList) et on MàJ
    QList<QStringList> *songs = db->getBiblio();
    while(!songs->isEmpty())
    {
        insertSong(songs->takeFirst());
    }
    //Un petit coup de tri à la fin pour faire tout beau tout joli
    biblio->sortItems(0,Qt::AscendingOrder);
}

/** @brief Slot pour regénéner les playlist */
void MainWindow::regenPlaylists()
{
    //On fait le tour de toutes les playlists
    QStringList *pl = db->getPlaylists();
    while(!pl->isEmpty())
    {
        //on leur créée un QTreeWidgetItem
        QTreeWidgetItem *playlist = new QTreeWidgetItem(QStringList(pl->takeFirst()));
        QStringList *assos = db->getAssos(playlist->text(0));
        //Et on leur attache toutes les chansons associées
        while(!assos->isEmpty())
        {
            QString path = assos->takeFirst();
            QTreeWidgetItem *song = new QTreeWidgetItem(QStringList(db->getTitleFromPath(path)) << path);
            playlist->addChild(song);
        }
        plists->addTopLevelItem(playlist);
    }
}

/** @brief Méthode pour insérer une chanson dans le QTreeWidgetItem de la bibliothèque */
void MainWindow::insertSong(QStringList song)
{
    QString title = song.takeFirst();
    QString artist = song.takeFirst();
    QString album = song.takeFirst();
    bool exists = false;
    QTreeWidgetItem *artistItem;
    QTreeWidgetItem *albumItem;

    //on cherche le QTreeWidgetItem de l'artiste pour ajouter dedans. Si il existe pas, on le créé.
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

    //idem pour l'album
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

    //idem pour la chanson. Si elle existe pas on ajoute, sinon... Bah rien.
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
    exists = false;

    //Ajoute la chanson dans la liste des résultats de recherche, si elle n'y est pas déjà.
    for(int l=0;l < searchRes->topLevelItemCount() && !exists;++l)
    {
        if(searchRes->topLevelItem(l)->text(0) == title && searchRes->topLevelItem(l)->text(1) == artist && searchRes->topLevelItem(l)->text(2) == album )
        {
            exists = true;
        }
    }
    if(!exists)
    {
        searchRes->addTopLevelItem(new QTreeWidgetItem(QStringList(title) << artist << album));
        searchRes->sortItems(0,Qt::AscendingOrder);
    }
}

/** @brief Slot pour afficher le bloc de recherche et cacher les autres*/
void MainWindow::showSearch()
{
    searchBlock->show();
    biblio->hide();
    plBlock->hide();
    options->hide();
}

/** @brief Slot pour afficher le bloc des options et cacher les autres*/
void MainWindow::showOptions()
{
    searchBlock->hide();
    biblio->hide();
    plBlock->hide();
    options->show();
}

/** @brief Slot pour afficher le bloc de la bibliothèque et cacher les autres*/
void MainWindow::showBiblio()
{
    searchBlock->hide();
    options->hide();
    plBlock->hide();
    biblio->show();
}

/** @brief Slot pour afficher le bloc des playlists et cacher les autres*/
void MainWindow::showPlists()
{
    searchBlock->hide();
    options->hide();
    plBlock->show();
    biblio->hide();
}

/** @brief Slot pour afficher le temps total de la chanson en lecture et initialiser le temps de lecture
 * @param time : temps total
 */
void MainWindow::upTimeTot(qint64 time)
{
    timeTotal->setText(convertTime(time));
    timeCurrent->setText(convertTime(0));
}

/** @brief Slot pour incrémenter temps de lecture
 * @param time : incrémentation
 */
void MainWindow::incrTimeCur(qint64 time)
{
    timeCurrent->setText(convertTime(time));
}

/** @brief Méthode pour convertir un qint64 en temps (mm:ss) et l'envoyer dans une QString pour l'afficher dans un QLabel
 * @param time : temps à convertir
 * @return temps converti
 */
QString MainWindow::convertTime(qint64 time)
{
    time = time/1000;
    int seconds = time%60;
    int minutes = (time-seconds)/60;
    QString converted = QString("%1:%2").arg(minutes,2,10,QChar('0')).arg(seconds,2,10,QChar('0'));
    return converted;
}

/** @brief Slot pour ajouter depuis la bibliothèque ou une playlist vers la playlist courante
 * @param item : item double cliqué
 */
void MainWindow::addToCurrent(QTreeWidgetItem * item, int)
{
    //Définir la nature de l'item double cliqué (Artiste,Album,Chanson,Playlist)
    //pas d'item parent = artiste ou playlist (même comportement)
    if(item->parent() == NULL)
    {
        for(int i=0;i<item->childCount();++i)
        {
            addToCurrent(item->child(i),0);
        }
    }
    //pas d'item enfant = chanson
    else if(item->childCount() == 0)
    {
        if(item->treeWidget() == plists)
            current->addTopLevelItem(new QTreeWidgetItem(db->getSong(item->text(1))));
        else
            current->addTopLevelItem(new QTreeWidgetItem(db->getSong(item->text(0),item->parent()->text(0),item->parent()->parent()->text(0))));
    }
    //a la fois parent et enfant(s) = album
    else
    {
        for(int j=0;j<item->childCount();++j)
        {
            addToCurrent(item->child(j),0);
        }
    }
}

/** @brief Slot pour ajouter un résultat de la recherche à la playlist courante
 * @param item : item double cliqué
 */
void MainWindow::addSearchToCurrent(QTreeWidgetItem * item, int)
{
    //appel à la base de données pour récupérer les infos complètes de la chanson
    current->addTopLevelItem(new QTreeWidgetItem(db->getSong(item->text(0),item->text(2),item->text(1))));
}

/** @brief Slot pour lire/mettre en pause une musique */
void MainWindow::play()
{
    //Si on est déjà en train de jouer, on met la musique en pause, change le QAction (play) et le titre
    if(media->state() == Phonon::PlayingState)
    {
        actionPlay->setIcon(QIcon(":/ico/play.png"));
        actionPlay->setText(tr("Lecture"));
        media->pause();
        setWindowTitle("[" + tr("Pause") + "] " + windowTitle());
    }
    else
    {
        //Si on jouait pas déjà quelque chose, on lance le premier morceau de la liste en cours
        if(playing == NULL)
        {
            //a condition qu'il y en ait un, pour éviter la segfault
            if(current->topLevelItemCount() == 0)
            {
                return;
            }
            playing = current->topLevelItem(0);
            media->setCurrentSource(playing->text(6));
        }
        //on modifie le titre en conséquence, et le QAction pour mettre en pause.
        setWindowTitle(playing->text(0) + " - " + playing->text(1) + " - PotatoPlayer");

        actionPlay->setIcon(QIcon(":/ico/pause.png"));
        actionPlay->setText(tr("Pause"));

        //On envoie la musique et on affiche la chanson en cours en gras
        media->play();
        bold();
    }
}

/** @brief Slot pour arreter la lecture d'une musique */
void MainWindow::stop()
{
    //on réinitialise le titre, remet le play/pause sur play, coupe la musique et efface le gras
    setWindowTitle("PotatoPlayer");
    actionPlay->setIcon(QIcon(":/ico/play.png"));
    actionPlay->setText(tr("Lecture"));
    media->stop();
    clear();
}

/** @brief Slot pour enlever le gras sur les différents champs de la musique en cours */
void MainWindow::clear()
{
    //Evitons les segfault...
    if(playing == NULL)
        return;
    QFont font;
    font.setBold(false);
    playing->setFont(0,font);
    playing->setFont(1,font);
    playing->setFont(2,font);
    playing->setFont(3,font);
    playing->setFont(4,font);
    playing->setFont(5,font);
    playing->setFont(6,font);
}

/** @brief Slot pour mettre en gras les différents champs de la musique en cours */
void MainWindow::bold()
{
    //Evitons les segfault...
    if(playing == NULL)
        return;
    QFont font;
    font.setBold(true);
    playing->setFont(0,font);
    playing->setFont(1,font);
    playing->setFont(2,font);
    playing->setFont(3,font);
    playing->setFont(4,font);
    playing->setFont(5,font);
    playing->setFont(6,font);
}

/** @brief Slot pour lire la musique précédente */
void MainWindow::prev()
{
    //On vérifie que la playlist en cours ne soit pas vide.
    if(current->topLevelItemCount() == 0)
        return;

    //On coupe la musique en cours
    stop();
    //Si la chanson n'est pas la première de la liste en cours, pas de probleme.
    if(playing != current->topLevelItem(0))
    {
        clear();
        playing = current->itemAbove(playing);
        bold();
    }
    //Si c'est la première, mais qu'on lit en boucle, on prend la dernière de la liste en cours
    else if(loopState == 2)
    {
        clear();
        playing = current->topLevelItem(current->topLevelItemCount()-1);
        bold();
    }
    //Si c'est la première et qu'on boucle pas, on revient juste au début de la chanson
    media->setCurrentSource(playing->text(6));
    play();
}

/** @brief Fin du morceau (gestion boucle, incrémentation du nombre de lectures) */
void MainWindow::songEnd()
{
    //Vérifier que l'utilisateur n'ait pas vidé la liste en cours entre temps (si oui on arrête simplement)
    if(current->indexOfTopLevelItem(playing) != -1)
    {
        //mise à jour de l'affichage & de la base de données pour le nombre de lectures
        playing->setText(4,QString::number(db->incrNb_played(playing->text(6),playing->text(4).toInt())));
        //si on repète un morceau, on relit, sinon on envoie le suivant
        if(loopState == 1)
            media->play();
        else
            next();
    }
    else
        stop();
}

/** @brief Slot pour lire la musique suivante */
void MainWindow::next()
{
    //vérifier que l'utilisateur n'ait pas vidé la liste de lecture en cours.
    if(current->topLevelItemCount() == 0)
        return;

    //coupe la musique actuelle
    stop();
    //si c'est pas le dernier de la liste, on envoie le suivant
    if(playing != current->topLevelItem(current->topLevelItemCount()-1))
    {
        clear();
        playing = current->itemBelow(playing);
        bold();
        media->setCurrentSource(playing->text(6));
        play();
    }
    //si c'est le dernier mais qu'on répète tout, on renvoie le premier
    else if(loopState == 2)
    {
        clear();
        playing = current->topLevelItem(0);
        bold();
        media->setCurrentSource(playing->text(6));
        play();
    }
    //sinon on stoppe
}

/** @brief Slot pour gérer la lecture en boucle */
void MainWindow::loop()
{
    //Si on répètait rien, on passe a répéter une chanson (loopstate = 1)
    if(loopState == 0)
    {
        actionLoop->setIcon(QIcon(":/ico/loop_one.png"));
        actionLoop->setText(tr("Répéter tout"));
        loopState = 1;
    }
    //si on répétait une chanson, on répète toute la playlist (loopstate = 2)
    else if(loopState == 1)
    {
        actionLoop->setIcon(QIcon(":/ico/loop_all.png"));
        actionLoop->setText(tr("Pas de répétition"));
        loopState = 2;
    }
    //Si on répétait tout, on répète plus rien (loopstate = 0)
    else
    {
        actionLoop->setIcon(QIcon(":/ico/loop_off.png"));
        actionLoop->setText(tr("Répéter chanson"));
        loopState = 0;
    }
}

/** @brief Slot quand on double clique sur un item de la playlist en cours pour le jouer */
void MainWindow::selectedSong(QTreeWidgetItem *item, int)
{
    stop();
    playing = item;
    bold();
    media->setCurrentSource(playing->text(6));
    play();
}

/** @brief Slot pour ajouter une playlist */
void MainWindow::addPl()
{
    //récupère le nom que veut l'utilisateur
    QString plName = QInputDialog::getText(this,tr("Nouvelle liste de lecture"),tr("Choississez le nom de la nouvelle liste de lecture") + " :",QLineEdit::Normal,"");
    //Vérification en cas d'annulation
    if (!plName.isEmpty())
        if(db->addPl(plName))
            plists->addTopLevelItem(new QTreeWidgetItem(QStringList(plName)));
}

/** @brief Slot pour supprimer une playlist */
void MainWindow::delPl()
{
    //Si rien de sélectionné, rien à supprimer
    if(plists->currentItem() == 0)
        return;
    //suppression de l'élément courant dans la base de données puis dans l'interface
    db->delPl(plists->currentItem()->text(0));
    plists->takeTopLevelItem(plists->indexOfTopLevelItem(plists->currentItem()));
}

/** @brief Slot pour sauvegarder la playlist courante à la fermeture du player */
void MainWindow::saveCurrent()
{
    //Ouvre le fichier PotatoSvg (écrase si déjà rempli, créé si inexistant)
    std::ofstream fichier("PotatoSvg");
    //ajoute le chemin de chaque chanson dans le fichier (1 chemin/ligne)
    for(int i=0;i<current->topLevelItemCount();++i)
    {
        fichier << current->topLevelItem(i)->text(6).toStdString() << std::endl;
    }
    fichier.close();
}

/** @brief Slot pour charger au lancement la playlist sauvegardée précédemment. */
void MainWindow::loadCurrent()
{
    //ouverture et lecture du fichier ligne par ligne, ajout dans la playlist en cours
    std::ifstream fichier("PotatoSvg");
    std::string chaine;
    QStringList liste;
    while(std::getline(fichier,chaine))
    {
        liste = db->getSong(QString::fromStdString(chaine));
        if(liste.at(0) != "")
        {
            current->addTopLevelItem(new QTreeWidgetItem(liste));
        }
    }
    fichier.close();
}

/** @brief Slot pour changer le message de la statusBar */
void MainWindow::changeStatus(QString str){
    statusBar->showMessage(str);
}

/** @brief Slot pour effacer la playlist courante */
void MainWindow::clearCurrent()
{
    while(current->topLevelItemCount() != 0)
        current->takeTopLevelItem(0);
}

/** @brief Slot pour enregistrer la playlist courante dans une playlist */
void MainWindow::currentToPl()
{
    //sélection du nom de la playlist et création
    QString plName = QInputDialog::getText(this,tr("Nouvelle liste de lecture"),tr("Choississez le nom de la nouvelle liste de lecture") + " :",QLineEdit::Normal,"");
    if (!plName.isEmpty())
        if(db->addPl(plName))
            plists->addTopLevelItem(new QTreeWidgetItem(QStringList(plName)));
    //ajout de chaque chanson dans la playlist
    for(int i=0;i<current->topLevelItemCount();++i)
    {
        insertPl(plName,current->topLevelItem(i),0);
    }
    statusBar->showMessage(tr("Liste de lecture enregistrée"));
}
