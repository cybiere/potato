#include "mainwindow.hpp"

MainWindow::MainWindow()
{
    db = new dbManager;

    media = new Phonon::MediaObject();
    media->setTickInterval(1000);
    output = new Phonon::AudioOutput(Phonon::MusicCategory);
    createPath(media,output);

    connect(media,SIGNAL(totalTimeChanged(qint64)),this,SLOT(upTimeTot(qint64)));
    connect(media,SIGNAL(tick(qint64)),this,SLOT(incrTimeCur(qint64)));

    ////////////////////    Menu    ////////////////////
    //Menu Fichier
    QMenu *menuFichier = menuBar()->addMenu(tr("Fichier"));

        QAction *actionQuit = new QAction(tr("Quitter"),this);
            connect(actionQuit,SIGNAL(triggered()),qApp,SLOT(quit()));
        menuFichier->addAction(actionQuit);

    //Menu Lecture
    QMenu *menuLecture = menuBar()->addMenu(tr("Lecture"));

        actionPlay = new QAction(QIcon(":/ico/play.png"),tr("Lecture"), this);
        actionStop = new QAction(QIcon(":/ico/stop.png"),tr("Stop"), this);
        actionPrev = new QAction(QIcon(":/ico/prev.png"),tr("Précédent"), this);
        actionNext = new QAction(QIcon(":/ico/next.png"),tr("Suivant"), this);
        actionLoop = new QAction(QIcon(":/ico/loop.png"),tr("Répéter"), this); actionLoop->setCheckable(true);

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
        ctrlBar->addAction(actionQuit);

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
        //Bibliothèque
        biblio = new QTreeWidget;
            //Pas de headers, pas d'expand au double clic
            biblio->header()->close();
            biblio->setExpandsOnDoubleClick(false);
            connect(biblio,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(addToCurrent(QTreeWidgetItem*,int)));


        //Options
        options = new QWidget;
            QLabel *titre = new QLabel(tr("Options"));
            QVBoxLayout *optionsLay = new QVBoxLayout;
                optionsLay->addWidget(titre);
                options->setLayout(optionsLay);
                options->hide();

                //Gestion des dossiers sources de chansons
                QHBoxLayout *srcDirLay = new QHBoxLayout;
                    srcDirLay->setMargin(0);
                    //liste pour afficher
                    srcDirList = new QListWidget;
                    srcDirList->addItems(db->getSrcDirs());
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

        //bouton pour afficher la bibliothèque
        QPushButton *bibl = new QPushButton(tr("Bibliothèque"));
            bibl->setFlat(true);
            connect(bibl,SIGNAL(clicked()),this,SLOT(showBiblio()));

        //bouton pour afficher les options
        QPushButton *opt = new QPushButton(tr("Options"));
            opt->setFlat(true);
            connect(opt,SIGNAL(clicked()),this,SLOT(showOptions()));

        QFrame *separator = new QFrame();
            separator->setFrameShape(QFrame::HLine);
            separator->setFrameShadow(QFrame::Sunken);

        leftLay->addWidget(bibl);
        leftLay->addWidget(biblio);
        leftLay->addWidget(separator);
        leftLay->addWidget(opt);
        leftLay->addWidget(options);
    leftDock->setLayout(leftLay);

    //mainZone
    //QTreeWidget des chansons de la liste "lecture en cours"
    current = new QTreeWidget;
    QTreeWidgetItem *headers = new QTreeWidgetItem(QStringList(tr("Titre")) << tr("Auteur") << tr("Album") << tr("Genre") << tr("Jouée") << tr("Note") << tr("Path"));
    current->setHeaderItem(headers);

    //rightDock
    //Cover, wikipédia, playlists
    QTabWidget *rightDock = new QTabWidget;

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
}

void MainWindow::showOptions()
{
    //masque la bibliothèque et affiche les options
    biblio->hide();
    options->show();
}

void MainWindow::showBiblio()
{
    //affiche la bibliothèque et masque les options
    options->hide();
    biblio->show();
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
