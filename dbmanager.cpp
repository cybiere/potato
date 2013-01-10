#include "dbmanager.hpp"

/** @brief Constructeur privé du dbManager:
 * Création des tables, et initialisation du QSqlQuery.
 */
dbManager::dbManager()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("PotatoBase");

    query = QSqlQuery(db);

    if(!db.open())
    {
        std::cerr << "Unable to create/open database" << std::endl;
        exit(1);
    }

    //Création des tables si nécessaire
    query = db.exec("CREATE TABLE IF NOT EXISTS song(file string primary key, artist string, title string, album string, genre string, nb_played integer, rating interger)");
    query = db.exec("CREATE TABLE IF NOT EXISTS playlist(name string primary key)");
    query = db.exec("CREATE TABLE IF NOT EXISTS asso_song_pl(pl string, sg string)");
    query = db.exec("CREATE TABLE IF NOT EXISTS srcDir(dir string)");
}

/** @brief Mise en place du singleton.
 * Retourne une instance de dbManager ou la crée si elle n'existe pas
 * @return singleton : instance de dbManager
 */
dbManager* dbManager::getInstance(){
    if (singleton == NULL)
         {
           std::cout << "creating singleton." << std::endl;
           singleton =  new dbManager;
         }
       else
         {
           std::cout << "singleton already created!" << std::endl;
         }

       return singleton;
}

/** @brief Méthode d'ajout d'une musique dans la BDD avec lecture des tags
 * @param file : chemin de la musique à ajouter
 * @return song : musique ajoutée sous format prêt pour insertion dans la bibliothèque
 */
QStringList dbManager::addSong(QString file)
{
    QStringList song;
    QString artiste,titre,album,genre;
    //on vérifie que la chanson ne soit pas déjà dans la base de données
    query.prepare("SELECT artist,title,album,genre,nb_played,rating FROM song WHERE file=?");
        query.bindValue(1,file);
    query.exec();
    if(!query.isValid())
    {
        //lecture des tags ID3 (UTF-8)
        TagLib::FileRef f(file.toStdString().c_str());
        TagLib::Tag *tag = f.tag();

        artiste = QString::fromUtf8(tag->artist().toCString(true));
        titre = QString::fromUtf8(tag->title().toCString(true));
        album = QString::fromUtf8(tag->album().toCString(true));
        genre = QString::fromUtf8(tag->genre().toCString(true));

        if(artiste == "") artiste = tr("Inconnu");
        if(titre == "") titre = tr("Inconnu");
        if(album == "") album = tr("Inconnu");

        query.prepare("INSERT INTO song (file, artist, title, album, genre, nb_played, rating) VALUES (?, ?, ?, ?, ?, 0, 0)");
             query.bindValue("1", file);
             query.bindValue("2", artiste);
             query.bindValue("3", titre);
             query.bindValue("4", album);
             query.bindValue("5", genre);
        query.exec();

    }
    else
    {
        //si elle existait déjà, on récupère les tags depuis la db (requête select au dessus) pour le return
        query.first();
        artiste = query.value(0).toString();
        titre = query.value(1).toString();
        album = query.value(2).toString();
    }


    song << titre << album << artiste;
    return song;
}

/** @brief Méthode pour récupérer dans la bdd les répertoires à scanner.
 * @return : sources : liste des répertoires à scanner
 */
QStringList dbManager::getSrcDirs()
{
    QStringList sources;
    query.prepare("SELECT dir FROM srcDir");
    query.exec();
    while (query.next()) {
        sources << query.value(0).toString();
    }
    return sources;
}

/** @brief Méthode pour ajouter un répertoire à scanner dans la BDD
 * @param dir : répertoire à ajouter;
 */
void dbManager::addSrc(QString dir)
{
    query.prepare("INSERT INTO srcDir (dir) VALUES (?)");
         query.bindValue("1", dir);
    query.exec();
}

/** @brief Méthode pour supprimer un répertoire à scanner de la BDD
 * @param dir : répertoire à supprimer
 */
void dbManager::delSrc(QString dir)
{
    query.prepare("DELETE FROM srcDir WHERE dir=?");
         query.bindValue("1", dir);
    query.exec();
}

/** @brief Méthode pour récupérer la bibliothèque dans la BDD
 * @return songs : liste des chansons de la bibliothèque
 */
QList<QStringList> *dbManager::getBiblio()
{
    //Chaque chanson est représentée par un QStringList pour l'ajout dans la bibliothèque
    QString artiste;
    QString album;
    QString titre;
    QList<QStringList> *songs = new QList<QStringList>;
    query.prepare("SELECT artist,title,album FROM song");
    query.exec();
    while(query.next())
    {
        if((titre = query.value(1).toString()) == "")
            titre = tr("Inconnu");
        if((artiste = query.value(0).toString()) == "")
            artiste = tr("Inconnu");
        if((album = query.value(2).toString()) == "")
            album = tr("Inconnu");
        QStringList song(titre);
        song << artiste << album;
        songs->append(song);
    }

    return songs;
}

/** @brief Méthode pour récupérer les playlists dans la BDD
 * @return pl : liste des playlist
 */
QStringList *dbManager::getPlaylists()
{
    QStringList *pl = new QStringList;
    query.prepare("SELECT name FROM playlist");
    query.exec();
    while(query.next())
    {
        pl->append(query.value(0).toString());
    }

    return pl;
}

/** @brief Méthode pour récupérer les musiques d'une playlist
 * @param pl : id de la playlist
 * @return songs : liste des chemins des musiques de la playlist
 */
QStringList *dbManager::getAssos(QString pl)
{
    QStringList *songs = new QStringList;
    query.prepare("SELECT sg FROM asso_song_pl WHERE pl=?");
        query.bindValue("1",pl);
    query.exec();
    while(query.next())
    {
        songs->append(query.value(0).toString());
    }
    return songs;
}

/** @brief Méthode pour récupérer les informations d'une musique dans la BDD à partir de son chemin
 * @param path : chemin de la musique
 * @return liste des informations de la musique (format complet, ajout à la lecture en cours)
 **/
QStringList dbManager::getSong(QString path)
{
    query.prepare("SELECT title,artist,album,genre,nb_played,rating,file FROM song WHERE file=?");
        query.bindValue("1",path);
    query.exec();
    query.first();

    return QStringList(query.value(0).toString()) << query.value(1).toString() << query.value(2).toString() << query.value(3).toString() << query.value(4).toString() << query.value(5).toString() << query.value(6).toString();
}

/** @brief Méthode pour récupérer les informations d'une musique dans la BDD à partir de son titre, album et artiste
 * @param titre : titre de la musique
 * @param album : album de la musique
 * @param artist : artiste de la musique
 * @return liste des informations de la musique (format complet, ajout à la lecture en cours)
 */
QStringList dbManager::getSong(QString titre,QString album,QString artist)
{
    query.prepare("SELECT title,artist,album,genre,nb_played,rating,file FROM song WHERE title=? AND album=? AND artist=?");
        query.bindValue("1",titre);
        query.bindValue("2",album);
        query.bindValue("3",artist);
    query.exec();
    query.first();

    return QStringList(query.value(0).toString()) << query.value(1).toString() << query.value(2).toString() << query.value(3).toString() << query.value(4).toString() << query.value(5).toString() << query.value(6).toString();
}

/** @brief Méthode pour ajouter une musique dans une playlist.
 * @param sg : chemin de la musique à ajouter
 * @param pl : identifiant de la playlist
 */
void dbManager::addSgToPl(QString sg, QString pl)
{
    query.prepare("INSERT INTO asso_song_pl (sg,pl) VALUES (?,?)");
         query.bindValue("1", sg);
         query.bindValue("2", pl);
    query.exec();
}

/** @brief Méthode pour ajouter une playlist si elle n'existe pas
 * @param nom : nom de la playlist
 * @return true si ajout réussi, false si déjà présente.
 */
bool dbManager::addPl(QString nom)
{
    query.prepare("SELECT *FROM playlist WHERE name=?");
        query.bindValue("1", nom);
    query.exec();
    if(query.first())
        return false;

    query.prepare("INSERT INTO playlist (name) VALUES (?)");
         query.bindValue("1", nom);
    query.exec();
    return true;
}

/** @brief Méthode pour incrémenter le nombre de lectures d'une musique
 * @param file : musique
 * @param oldNumber : nombre de lectures précédentes
 * @return oldNumber : nouveau nombre de lectures
 */
int dbManager::incrNb_played(QString file, int oldNumber)
{
    ++oldNumber;

    query.prepare("UPDATE song SET nb_played=? WHERE file=?");
        query.bindValue("1", oldNumber);
        query.bindValue("2",file);
    query.exec();
    return oldNumber;
}

/** @brief Méthode pour supprimer une playlist et les associations avec les chansons.
 * @param name : nom de la musique à supprimer.
 */
void dbManager::delPl(QString name)
{
    //supprime la playlist
    query.prepare("DELETE FROM playlist WHERE name=?");
         query.bindValue("1", name);
    query.exec();
    //supprime les associations avec les chansons
    query.prepare("DELETE FROM asso_song_pl WHERE pl=?");
         query.bindValue("1", name);
    query.exec();
}

/** @brief Méthode pour obtenir le titre d'une musique à partir de son chemin
 * @param path : chemin de la musique dont on veut obtenir le titre
 * @return titre de la musique
 */
QString dbManager::getTitleFromPath(QString path)
{
    query.prepare("SELECT title FROM song WHERE file=?");
        query.bindValue("1",path);
    query.exec();
    query.first();

    return query.value(0).toString();
}

/** @variable instance de notre dbManager
 * Initialisé à NULL
 */
dbManager *dbManager::singleton = NULL;
