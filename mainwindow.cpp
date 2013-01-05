#include "mainwindow.hpp"
#include <iostream>

MainWindow::MainWindow()
{
    db = new dbManager;
    ////////////////////    Menu    ////////////////////
    QMenu *menuFichier = menuBar()->addMenu(tr("Fichier"));

        //Action quitter
        QAction *actionQuit = new QAction(tr("Quitter"),this);
        menuFichier->addAction(actionQuit);
        connect(actionQuit,SIGNAL(triggered()),qApp,SLOT(quit()));

    ////////////////////    Ctrl    ////////////////////

        //Barre de controle
        //Actions à mettre en place
        QToolBar *ctrlBar = addToolBar("Controles");
        ctrlBar->setFloatable(false);
        ctrlBar->setMovable(false);
        ctrlBar->addAction(actionQuit);

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
    QTreeWidget *current = new QTreeWidget;
    QTreeWidgetItem *headers = new QTreeWidgetItem(QStringList("Titre") << "Auteur" << "Album" << "Genre" << "Jouée" << "Note" << "Path");
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
    QString dir = QFileDialog::getExistingDirectory(this,"Choisir un dossier", getenv("HOME"));
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
