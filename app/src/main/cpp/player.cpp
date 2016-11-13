//
// Created by JasonXiao on 2016/10/24.
//

#include "player.h"



void Player::init(const std::string url){
    assert(url.length() > 0);
    this->url = url;
    int result;
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    result = avformat_open_input(&pFormatCtx, this->url.c_str(), NULL, NULL);
    assert(result == 0);
    result = avformat_find_stream_info(pFormatCtx, NULL);
    assert(result >= 0);
    for(int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioIndex = i;
        }
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
        }
    }
    if (audioIndex >= 0) {
        aStream = pFormatCtx->streams[audioIndex];
        aCodecCtx = pFormatCtx->streams[audioIndex]->codec;
        aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
        result = avcodec_open2(aCodecCtx, aCodec, NULL);
        assert(result >= 0);
        audioConvertCtx = swr_alloc();
        audioConvertCtx = swr_alloc_set_opts(NULL,
                                             aCodecCtx->channel_layout, AV_SAMPLE_FMT_S16, aCodecCtx->sample_rate,
                                             aCodecCtx->channel_layout, aCodecCtx->sample_fmt, aCodecCtx->sample_rate,
                                             0, NULL);
        swr_init(audioConvertCtx);
    }
    if (videoIndex >= 0) {
        vStream = pFormatCtx->streams[videoIndex];
        vCodecCtx=pFormatCtx->streams[videoIndex]->codec;
        vCodec = avcodec_find_decoder(vCodecCtx->codec_id);
        result = avcodec_open2(vCodecCtx, vCodec,NULL);
        assert(result >= 0);
        frameRGBA = av_frame_alloc();
        width = vCodecCtx->width;
        height = vCodecCtx->height;
        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height,1);
        vOutBuffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
        av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, vOutBuffer, AV_PIX_FMT_RGBA, width, height, 1);
        imgConvertCtx = sws_getContext(vCodecCtx->width, vCodecCtx->height, vCodecCtx->pix_fmt,
                                       vCodecCtx->width, vCodecCtx->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
    }



}

void Player::decode() {
    while (true) {
        AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
        int ret = av_read_frame(pFormatCtx, packet);
        if (ret < 0) break;
        enQueue(packet);
    }
}

void Player::enQueue(AVPacket *packet) {
    if (packet == nullptr) return;
    queueMutex.lock();
    if (packet->stream_index == videoIndex) {
        picQueue.push(*packet);
    }
    if (packet->stream_index == audioIndex) {
        audioQueue.push(*packet);
    }
    queueMutex.unlock();
}

AVFrame* Player::deQueuePic() {
    begin:
    queueMutex.lock();
    if (picQueue.size() == 0) return nullptr;
    AVPacket packet = picQueue.front();
    picQueue.pop();
    queueMutex.unlock();
    av_gettime_relative();
    int ret = avcodec_send_packet(vCodecCtx, &packet);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        goto begin;
    ret = avcodec_receive_frame(vCodecCtx, &vFrame);
    if (ret < 0 && ret != AVERROR_EOF)
        goto begin;
    sws_scale(imgConvertCtx, (const uint8_t* const*)vFrame.data, vFrame.linesize, 0, vCodecCtx->height,
              frameRGBA->data, frameRGBA->linesize);
    double timestamp;
    if(packet.pts == AV_NOPTS_VALUE) {
        timestamp = 0;
    } else {
        timestamp = av_frame_get_best_effort_timestamp(&vFrame)*av_q2d(vStream->time_base);
    }
    double frameRate = av_q2d(vStream->avg_frame_rate);
    frameRate += vFrame.repeat_pict * (frameRate * 0.5);
    if (timestamp == 0.0) {
        usleep((unsigned long)(frameRate*1000));
    }else {
        if (fabs(timestamp - audioClock) > AV_SYNC_THRESHOLD_MIN &&
                fabs(timestamp - audioClock) < AV_NOSYNC_THRESHOLD) {
            if (timestamp > audioClock) {
                usleep((unsigned long)((timestamp - audioClock)*1000000));
            }
        }
    }
    av_packet_unref(&packet);
    return frameRGBA;
}


void Player::deQueueAudio(int &nextSize, uint8_t *outputBuffer) {
    begin:
    queueMutex.lock();
    if (audioQueue.size() == 0) return;
    AVPacket packet = audioQueue.front();
    audioQueue.pop();
    queueMutex.unlock();
    int ret = avcodec_send_packet(aCodecCtx, &packet);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        goto begin;
    ret = avcodec_receive_frame(aCodecCtx, &aFrame);
    if (ret < 0 && ret != AVERROR_EOF)
        goto begin;

    audioClock = aFrame.pkt_pts * av_q2d(aStream->time_base);
    if (aCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P) {
        nextSize = av_samples_get_buffer_size(aFrame.linesize, aCodecCtx->channels, aCodecCtx->frame_size, aCodecCtx->sample_fmt, 1);
    }else {
        av_samples_get_buffer_size(&nextSize, aCodecCtx->channels, aCodecCtx->frame_size, aCodecCtx->sample_fmt, 1);
    }
    // 音频格式转换
    ret = swr_convert(audioConvertCtx, &outputBuffer, aFrame.nb_samples,
                (uint8_t const **) (aFrame.extended_data),
                aFrame.nb_samples);
    av_packet_unref(&packet);
}



void Player::release() {
    for (int i = 0; i < picQueue.size(); ++i) {
        picQueue.pop();
    }
    sws_freeContext(imgConvertCtx);
    av_free(frameRGBA);
    av_free(vOutBuffer);
    avcodec_close(vCodecCtx);
    avcodec_close(aCodecCtx);
    avformat_close_input(&pFormatCtx);
}

void Player::start() {
//    clock.start();
}
