//
// Created by jasonxiao on 2019/2/1.
//

#ifndef EASYPLAYER_OPENSLES_H
#define EASYPLAYER_OPENSLES_H

#include <stdint.h>

class AudioDataProvider {
public:
    virtual void GetData(uint8_t **buffer, int &buffer_size) = 0;
    virtual ~AudioDataProvider() = default;
};


void initAudioPlayer(int sampleRate, int channel, AudioDataProvider *p);
void startAudioPlay();
void stopAudioPlay();

#endif //EASYPLAYER_OPENSLES_H
