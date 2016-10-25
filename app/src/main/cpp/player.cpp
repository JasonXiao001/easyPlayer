//
// Created by JasonXiao on 2016/10/24.
//

#include "player.h"


void Player::init(const std::string url){
    assert(url.length() > 0);
    this->url = url;
    int result;
    av_register_all();
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
    assert(audioIndex >= 0 && videoIndex >= 0);
    aCodecCtx = pFormatCtx->streams[audioIndex]->codec;
    aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
    vCodecCtx=pFormatCtx->streams[videoIndex]->codec;
    vCodec = avcodec_find_decoder(vCodecCtx->codec_id);
    assert(aCodec != NULL);
    assert(vCodec != NULL);
    result = avcodec_open2(aCodecCtx, aCodec, NULL);
    assert(result >= 0);
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
    queueMutex.unlock();
}

AVFrame* Player::deQueuePic() {
    queueMutex.lock();
    if (picQueue.size() == 0) return nullptr;
    AVPacket packet = picQueue.front();
    picQueue.pop();
    queueMutex.unlock();
    int ret = avcodec_send_packet(vCodecCtx, &packet);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return nullptr;
    ret = avcodec_receive_frame(vCodecCtx, &vFrame);
    if (ret < 0 && ret != AVERROR_EOF)
        return nullptr;
    sws_scale(imgConvertCtx, (const uint8_t* const*)vFrame.data, vFrame.linesize, 0, vCodecCtx->height,
              frameRGBA->data, frameRGBA->linesize);
    av_packet_unref(&packet);
    return frameRGBA;
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