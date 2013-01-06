#include "mediaplayer.hpp"

MediaPlayer::MediaPlayer()
{
    media = new Phonon::MediaObject();
    media->setTickInterval(1000);
    output = new Phonon::AudioOutput(Phonon::MusicCategory);
    createPath(media,output);
}
