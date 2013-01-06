#ifndef MEDIAPLAYER_HPP
#define MEDIAPLAYER_HPP
#include <phonon>


class MediaPlayer
{
public:
    MediaPlayer();
    Phonon::MediaObject* media;
    Phonon::AudioOutput* output;
};

#endif // MEDIAPLAYER_HPP
