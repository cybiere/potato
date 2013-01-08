#include "mainwindow.hpp"
#include "wikiinfo.hpp"
#include <fstream>

MainWindow::MainWindow()
{
    db = dbManager::getInstance();

    media = new Phonon::MediaObject();
    media->setTickInterval(1000);
    output = new Phonon::AudioOutput(Phonon::MusicCategory);
    createPath(media,output);

    connect(media,SIGNAL(totalTimeChanged(qint64)),this,SLOT(upTimeTot(qint64)));
    connect(media,SIGNAL(tick(qint64)),this,SLOT(incrTimeCur(qint64)));
    connect(media,SIGNAL(finished()),this,SLOT(songEnd()));
    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(saveCurrent()));

    playing = NULL;

    ////////////////////    Menu    ////////////////////
    //Menu Fichier
    QMenu *menuFichier = menuBar()->addMenu(tr("Fichier"));

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
    connect(this,SIGNAL(changeWikiInfo(QString)),wikiView,SLOT(search(QString)));
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

    regenBiblio();
    biblio->sortItems(0,Qt::AscendingOrder);
    regenPlaylists();
    loadCurrent();
}

void MainWindow::changeDockInfo(QTreeWidgetItem *item,int)
{
   emit changeWikiInfo(item->text(1));
}

void MainWindow::changeSearch(QString searchTerm)
{
    QRegExp rx("*" + searchTerm + "*");
    rx.setPatternSyntax(QRegExp::Wildcard);
    for(int i=0;i<searchRes->topLevelItemCount();++i)
    {
        if(!(rx.exactMatch(searchRes->topLevelItem(i)->text(0)) || rx.exactMatch(searchRes->topLevelItem(i)->text(1)) || rx.exactMatch(searchRes->topLevelItem(i)->text(2))))
            searchRes->topLevelItem(i)->setHidden(true);
        else
            searchRes->topLevelItem(i)->setHidden(false);
    }
}

void MainWindow::contextCurrent(QPoint pos)
{
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

void MainWindow::contextSearch(QPoint pos)
{
    if(searchRes->currentItem() == NULL)
        return;
    QMenu *context = new QMenu();
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

void MainWindow::addSearchToPl()
{
    insertPl(static_cast<QAction *>(sender())->text(),searchRes->currentItem(),0);
}

void MainWindow::addCurrentToPl()
{
    insertPl(static_cast<QAction *>(sender())->text(),current->currentItem(),0);
}

void MainWindow::addBiblioToPl()
{
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

void MainWindow::insertPl(QString playlist,QTreeWidgetItem *item,int niveau)
{
    if(item->treeWidget() == searchRes)
    {
        QString path;
        QStringList song = db->getSong(item->text(0),item->text(2),item->text(1));

        path = song.takeAt(6);
        QTreeWidgetItem *songItem = new QTreeWidgetItem(QStringList(item->text(0)) << path);
        bool found = false;
        QTreeWidgetItem *pl;
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
        QString titre;
        QString path;
        if(item->treeWidget() == current)
        {
            titre = item->text(0);
            path = item->text(6);
        }
        else
        {
            titre = item->text(0);
            QString album = item->parent()->text(0);
            QString artiste = item->parent()->parent()->text(0);
            QStringList song = db->getSong(titre,album,artiste);
            path = song.takeAt(6);
        }
        QTreeWidgetItem *songItem = new QTreeWidgetItem(QStringList(titre) << path);
        bool found = false;
        QTreeWidgetItem *pl;
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

void MainWindow::playSelected()
{
    if(biblio->currentItem() != NULL)
    {
        addToCurrent(biblio->currentItem(),0);
        play();
    }
}

void MainWindow::addSourceDir()
{
    //QFiledialog pour choisir le dossier en question, puis ajout à la base de données et à l'interface
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choisir un dossier"), getenv("HOME"));
    db->addSrc(dir);
    srcDirList->addItem(dir);
}

void MainWindow::delSourceDir()
{
    if(srcDirList->currentItem() == NULL)
        return;
    //suppression de l'élément courant dans la base de données puis dans l'interface
    db->delSrc(srcDirList->currentItem()->text());
    srcDirList->takeItem(srcDirList->currentRow());
}

void MainWindow::refresh()
{
    //scan récursif du dossier pour chaque entrée dans la liste des dossiers sources
    int nbItems = srcDirList->count();
    for(int i=0;i<nbItems;++i)
    {
        scanDir(srcDirList->item(i)->text());
    }
    biblio->sortItems(0,Qt::AscendingOrder);
}

void MainWindow::scanDir(QString path)
{
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
            //Traitement pour les fichiers
            insertSong(db->addSong(path + "/" + dir[i]));
        }
        else
        {
            //Scan des sous-dossiers.
            scanDir(path + "/" + dir[i]);
        }
    }
}

void MainWindow::regenBiblio()
{
    QList<QStringList> *songs = db->getBiblio();
    while(!songs->isEmpty())
    {
        insertSong(songs->takeFirst());
    }
}

void MainWindow::regenPlaylists()
{
    QStringList *pl = db->getPlaylists();
    while(!pl->isEmpty())
    {
        QTreeWidgetItem *playlist = new QTreeWidgetItem(QStringList(pl->takeFirst()));
        QStringList *assos = db->getAssos(playlist->text(0));
        while(!assos->isEmpty())
        {
            QString path = assos->takeFirst();
            QTreeWidgetItem *song = new QTreeWidgetItem(QStringList(db->getTitleFromPath(path)) << path);
            playlist->addChild(song);
        }
        plists->addTopLevelItem(playlist);
    }
}

void MainWindow::insertSong(QStringList song)
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
    searchRes->addTopLevelItem(new QTreeWidgetItem(QStringList(title) << artist << album));
    searchRes->sortItems(0,Qt::AscendingOrder);
}

void MainWindow::showSearch()
{
    //masque la bibliothèque et affiche les options
    searchBlock->show();
    biblio->hide();
    plBlock->hide();
    options->hide();
}

void MainWindow::showOptions()
{
    //masque la bibliothèque et affiche les options
    searchBlock->hide();
    biblio->hide();
    plBlock->hide();
    options->show();
}

void MainWindow::showBiblio()
{
    //affiche la bibliothèque et masque les options
    searchBlock->hide();
    options->hide();
    plBlock->hide();
    biblio->show();
}

void MainWindow::showPlists()
{
    //affiche la bibliothèque et masque les options
    searchBlock->hide();
    options->hide();
    plBlock->show();
    biblio->hide();
}

void MainWindow::upTimeTot(qint64 time)
{
    timeTotal->setText(convertTime(time));
    timeCurrent->setText(convertTime(0));
}

void MainWindow::incrTimeCur(qint64 time)
{
    timeCurrent->setText(convertTime(time));
}

QString MainWindow::convertTime(qint64 time)
{
    time = time/1000;
    int seconds = time%60;
    int minutes = (time-seconds)/60;
    QString converted = QString("%1:%2").arg(minutes,2,10,QChar('0')).arg(seconds,2,10,QChar('0'));
    return converted;
}

void MainWindow::addToCurrent(QTreeWidgetItem * item, int)
{
    //Définir la nature de l'item double cliqué (Artiste,Album,Chanson) --> ajouter playlist + tard
    //pas d'item parent = artiste
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

void MainWindow::addSearchToCurrent(QTreeWidgetItem * item, int)
{
    current->addTopLevelItem(new QTreeWidgetItem(db->getSong(item->text(0),item->text(2),item->text(1))));
}

void MainWindow::play()
{
    if(media->state() == Phonon::PlayingState)
    {
        actionPlay->setIcon(QIcon(":/ico/play.png"));
        actionPlay->setText(tr("Lecture"));
        media->pause();
        setWindowTitle("[" + tr("Pause") + "] " + windowTitle());
    }
    else
    {
        if(playing == NULL)
        {
            if(current->topLevelItemCount() == 0)
            {
                return;
            }
            playing = current->topLevelItem(0);
            media->setCurrentSource(playing->text(6));
        }
        setWindowTitle(playing->text(0) + " - " + playing->text(1) + " - PotatoPlayer");

        actionPlay->setIcon(QIcon(":/ico/pause.png"));
        actionPlay->setText(tr("Pause"));

        media->play();
        bold();
    }
}

void MainWindow::stop()
{
    setWindowTitle("PotatoPlayer");
    actionPlay->setIcon(QIcon(":/ico/play.png"));
    actionPlay->setText(tr("Lecture"));
    media->stop();
    clear();
}

void MainWindow::clear()
{
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

void MainWindow::bold()
{
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

void MainWindow::prev()
{
    if(current->topLevelItemCount() == 0)
        return;

    stop();
    if(playing != current->topLevelItem(0))
    {
        clear();
        playing = current->itemAbove(playing);
        bold();
    }
    else if(loopState == 2)
    {
        clear();
        playing = current->topLevelItem(current->topLevelItemCount()-1);
        bold();
    }

    media->setCurrentSource(playing->text(6));
    play();
}

void MainWindow::songEnd()
{
    db->incrNb_played(playing->text(6),playing->text(4).toInt());
    playing->setText(4,QString::number(db->incrNb_played(playing->text(6),playing->text(4).toInt())));
    if(loopState == 1)
    {
        media->play();
    }
    else
    {
        next();
    }
}

void MainWindow::next()
{
    if(current->topLevelItemCount() == 0)
        return;

    stop();
    if(playing != current->topLevelItem(current->topLevelItemCount()-1))
    {
        clear();
        playing = current->itemBelow(playing);
        bold();
        media->setCurrentSource(playing->text(6));
        play();
    }
    else if(loopState == 2)
    {
        clear();
        playing = current->topLevelItem(0);
        bold();
        media->setCurrentSource(playing->text(6));
        play();
    }
}

void MainWindow::loop()
{
    if(loopState == 0)
    {
        actionLoop->setIcon(QIcon(":/ico/loop_one.png"));
        actionLoop->setText(tr("Répéter tout"));
        loopState = 1;
    }
    else if(loopState == 1)
    {
        actionLoop->setIcon(QIcon(":/ico/loop_all.png"));
        actionLoop->setText(tr("Pas de répétition"));
        loopState = 2;
    }
    else
    {
        actionLoop->setIcon(QIcon(":/ico/loop_off.png"));
        actionLoop->setText(tr("Répéter chanson"));
        loopState = 0;
    }
}

void MainWindow::selectedSong(QTreeWidgetItem *item, int)
{
    stop();
    playing = item;
    bold();
    media->setCurrentSource(playing->text(6));
    play();
}

void MainWindow::addPl()
{
    QString plName = QInputDialog::getText(this,tr("Nouvelle liste de lecture"),tr("Choississez le nom de la nouvelle liste de lecture") + " :",QLineEdit::Normal,"");
    if (!plName.isEmpty())
        if(db->addPl(plName))
            plists->addTopLevelItem(new QTreeWidgetItem(QStringList(plName)));
}

void MainWindow::delPl()
{
    if(plists->currentItem() == 0)
        return;
    //suppression de l'élément courant dans la base de données puis dans l'interface
    db->delPl(plists->currentItem()->text(0));
    plists->takeTopLevelItem(plists->indexOfTopLevelItem(plists->currentItem()));
}

void MainWindow::saveCurrent()
{
    std::ofstream fichier("PotatoSvg");
    for(int i=0;i<current->topLevelItemCount();++i)
    {
        fichier << current->topLevelItem(i)->text(6).toStdString() << std::endl;
    }
    fichier.close();
}

void MainWindow::loadCurrent()
{
    std::ifstream fichier("PotatoSvg");
    std::string chaine;
    while(std::getline(fichier,chaine))
    {
        current->addTopLevelItem(new QTreeWidgetItem(db->getSong(QString::fromStdString(chaine))));
    }
    fichier.close();
}
