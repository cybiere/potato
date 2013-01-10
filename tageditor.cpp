#include "tageditor.hpp"
#include <iostream>

/** @brief Constructeur de TagEditor, connecte le dbManager et affiche le GUI
 *  @param song : le chemin de la chanson à éditer
 */
TagEditor::TagEditor(QString song):path(song)
{
    db = dbManager::getInstance();
    QVBoxLayout *bigLay = new QVBoxLayout;
    QHBoxLayout *ctrlLay = new QHBoxLayout;
    QPushButton *save = new QPushButton(tr("Enregister"));
    connect(save,SIGNAL(clicked()),this,SLOT(save()));
    QPushButton *cancel = new QPushButton(tr("Annuler"));
    connect(cancel,SIGNAL(clicked()),this,SLOT(reject()));
    setLayout(bigLay);
    QGroupBox *group = new QGroupBox("Edition des tags :");
    QVBoxLayout *smallLay = new QVBoxLayout;
    group->setLayout(smallLay);

    QStringList info = db->getSong(path);

    QHBoxLayout *titreLay = new QHBoxLayout;
    QLabel *labTitre = new QLabel(tr("Titre") + " :");
    titre = new QLineEdit(info.takeFirst());
    titreLay->addWidget(labTitre);
    titreLay->addWidget(titre);
    smallLay->addLayout(titreLay);

    QHBoxLayout *artistLay = new QHBoxLayout;
    QLabel *labArtist = new QLabel(tr("Artiste") + " :");
    artist = new QLineEdit(info.takeFirst());
    artistLay->addWidget(labArtist);
    artistLay->addWidget(artist);
    smallLay->addLayout(artistLay);

    QHBoxLayout *albumLay = new QHBoxLayout;
    QLabel *labAlbum = new QLabel(tr("Album") + " :");
    album = new QLineEdit(info.takeFirst());
    albumLay->addWidget(labAlbum);
    albumLay->addWidget(album);
    smallLay->addLayout(albumLay);

    QHBoxLayout *genreLay = new QHBoxLayout;
    QLabel *labGenre = new QLabel(tr("Genre") + " :");
    genre = new QLineEdit(info.takeFirst());
    genreLay->addWidget(labGenre);
    genreLay->addWidget(genre);
    smallLay->addLayout(genreLay);

    bigLay->addWidget(group);
    ctrlLay->addWidget(cancel);
    ctrlLay->addWidget(save);
    bigLay->addLayout(ctrlLay);
}

/** @brief Slot de sauvegarde des tags en cliquant sur le bouton Enregistrer
 */
void TagEditor::save()
{
    //enregistre dans la db & les tags id3, puis retourne a la mainwindow que c'est bon.
    db->upSong(path,titre->text(),album->text(),artist->text(),genre->text());
    accept();
}
