//
// Created by jasonxiao on 2019/2/1.
//

#ifndef EASYPLAYER_OPENSLES_H
#define EASYPLAYER_OPENSLES_H

#include <SLES/OpenSLES_Android.h>

typedef void (*audioPlayCallback) (SLAndroidSimpleBufferQueueItf bq, void *context);


void initAudioPlayer(int sampleRate, int channel, audioPlayCallback cb);
void startAudioPlay(void *context);

#endif //EASYPLAYER_OPENSLES_H
