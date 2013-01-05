#include "mainwindow.hpp"
#include <iostream>

MainWindow::MainWindow()
{
    db = new dbManager;
    ////////////////////    Menu    ////////////////////
    QMenu *menuFichier = menuBar()->addMenu(tr("Fichier"));

        QAction *actionQuit = new QAction(tr("Quitter"),this);
        menuFichier->addAction(actionQuit);
        connect(actionQuit,SIGNAL(triggered()),qApp,SLOT(quit()));

    ////////////////////    Ctrl    ////////////////////

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
        //Song librairy
        biblio = new QTreeWidget;
        biblio->header()->close();

        //Options
        options = new QWidget;
            QLabel *titre = new QLabel(tr("Options"));
            QVBoxLayout *optionsLay = new QVBoxLayout;
                optionsLay->addWidget(titre);
                options->setLayout(optionsLay);
                options->hide();
                QHBoxLayout *srcDirLay = new QHBoxLayout;
                    srcDirLay->setMargin(0);
                    srcDirList = new QListWidget;
                    srcDirList->addItems(db->getSrcDirs());
                    QVBoxLayout *srcDirActionLay = new QVBoxLayout;
                        srcDirActionLay->setMargin(0);
                        QPushButton *addSrcDir = new QPushButton(QIcon(":/ico/add.png"),"");
                            connect(addSrcDir,SIGNAL(clicked()),this,SLOT(addSourceDir()));
                        QPushButton *delSrcDir = new QPushButton(QIcon(":/ico/del.png"),"");
                            connect(delSrcDir,SIGNAL(clicked()),this,SLOT(delSourceDir()));
                            QPushButton *refBiblio = new QPushButton(QIcon(":/ico/ref.png"),"");
                                connect(refBiblio,SIGNAL(clicked()),this,SLOT(refresh()));
                        srcDirActionLay->addWidget(addSrcDir);
                        srcDirActionLay->addWidget(delSrcDir);
                        srcDirActionLay->addWidget(refBiblio);
                        srcDirActionLay->addWidget(new QWidget);
                    srcDirLay->addWidget(srcDirList);
                    srcDirLay->addLayout(srcDirActionLay);
                optionsLay->addLayout(srcDirLay);

        QPushButton *bibl = new QPushButton(tr("Bibliothèque"));
            bibl->setFlat(true);
            connect(bibl,SIGNAL(clicked()),this,SLOT(showBiblio()));

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
    QTreeWidget *current = new QTreeWidget;
    QTreeWidgetItem *headers = new QTreeWidgetItem(QStringList("Titre") << "Auteur" << "Album" << "Genre" << "Path");
    current->setHeaderItem(headers);

    //rightDock
    QTabWidget *rightDock = new QTabWidget;

    main->addWidget(leftDock);
    main->addWidget(current);
    main->addWidget(rightDock);

    main->setCollapsible(0,false);
    main->setCollapsible(1,false);
    main->setCollapsible(2,false);
    setCentralWidget(main);

}

void MainWindow::addSourceDir()
{
    QString dir = QFileDialog::getExistingDirectory(this,"Choisir un dossier", getenv("HOME"));
}

void MainWindow::delSourceDir()
{

}

void MainWindow::refresh()
{

}

void MainWindow::showOptions()
{
        biblio->hide();
        options->show();
}

void MainWindow::showBiblio()
{
        options->hide();
        biblio->show();
}
