//
// Created by Administrator on 2016/11/10.
//

#include "easyPlayer.h"


int PacketQueue::put_packet(AVPacket *pkt) {
    if (abort_request) {
        av_log(NULL, AV_LOG_INFO, "put_packet abort.\n");
        return -1;
    }
    int ret;
    std::unique_lock<std::mutex> lock(mutex);
    while (true) {
        if (queue.size() < MAX_SIZE) {
            AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
            if (packet == NULL) {
                av_log(NULL, AV_LOG_FATAL, "Could not create new AVPacket.\n");
                return -1;
            }
            ret = av_copy_packet(packet, pkt);
            if (ret != 0) {
                av_log(NULL, AV_LOG_FATAL, "Could not copy AVPacket.\n");
                return -1;
            }
            queue.push(*packet);
            duration += packet->duration;
            cond.notify_one();
            break;
        }else {
            full.wait(lock);
        }
    }
    return 0;
}


int PacketQueue::get_packet(AVPacket *pkt) {
    std::unique_lock<std::mutex> lock(mutex);
    for (;;) {
        if (abort_request) {
            return -1;
        }
        if (queue.size() > 0) {
            AVPacket tmp = queue.front();
            av_copy_packet(pkt, &tmp);
            duration -= tmp.duration;
            queue.pop();
            av_packet_unref(&tmp);
            full.notify_one();
            return 0;
        }else {
            cond.wait(lock);
        }
    }
}


int PacketQueue::put_nullpacket() {
    AVPacket *pkt = new AVPacket();
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;
    put_packet(pkt);
    return 0;
}


void PacketQueue::set_abort(int abort) {
    abort_request = abort;
}

int PacketQueue::get_abort() {
    return abort_request;
}

int PacketQueue::get_serial(){
    return serial;
}

size_t PacketQueue::get_queue_size() {
    return queue.size();
}


std::shared_ptr<Frame> FrameQueue::get_frame() {
    std::unique_lock<std::mutex> lock(mutex);
    for (;;) {
        if (queue.size() > 0) {
            auto tmp = queue.front();
            queue.pop();
            full.notify_one();
            return tmp;
        }else {
            empty.wait(lock);
        }
    }
}


void FrameQueue::put_frame(AVFrame *frame) {
    std::unique_lock<std::mutex> lock(mutex);
    while (true) {
        if (queue.size() < MAX_SIZE) {
            auto m_frame = std::make_shared<Frame>(frame);
            queue.push(m_frame);
            empty.notify_one();
            return;
        }else{
            full.wait(lock);
        }
    }

}

size_t FrameQueue::get_size() {
    return queue.size();
}


void Decoder::init(AVCodecContext *ctx) {
    avctx = ctx;

}


void Decoder::start_decode_thread() {
    pkt_queue.set_abort(0);
    std::thread t(&Decoder::decode, this);
    t.detach();
}

int VideoDecoder::get_height() {
    if (!avctx) return 0;
    return avctx->height;
}

int VideoDecoder::get_width(){
    if (!avctx) return 0;
    return avctx->width;
}



void VideoDecoder::decode() {
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
//    AVRational tb = video_st->time_base;
//    AVRational frame_rate = av_guess_frame_rate(ic, video_st, NULL);
//    if (!frame) {
//        return AVERROR(ENOMEM);
//    }
    for (;;) {
        if (pkt_queue.get_abort()) break;
        int got_picture;
        if ((got_picture = decoder_decode_frame()) < 0)
            return;

        if (got_picture) {
            double dpts = NAN;
//            if (frame->pts != AV_NOPTS_VALUE)
//                dpts = av_q2d(is->video_st->time_base) * frame->pts;
//
//            frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);
//
//            if (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
//                if (frame->pts != AV_NOPTS_VALUE) {
//                    double diff = dpts - get_master_clock(is);
//                    if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
//                        diff - is->frame_last_filter_delay < 0 &&
//                        is->viddec.pkt_serial == is->vidclk.serial &&
//                        is->videoq.nb_packets) {
//                        is->frame_drops_early++;
//                        av_frame_unref(frame);
//                        got_picture = 0;
//                    }
//                }
//            }
        }
//
//
//        if (ret < 0)
//            goto the_end;
//        if (!ret)
//            continue;
//
//        duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
//        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
//        ret = queue_picture(is, frame, pts, duration, av_frame_get_pkt_pos(frame), is->viddec.pkt_serial);
//        av_frame_unref(frame);
//
//        if (ret < 0)
//            goto the_end;
    }

//    av_frame_free(&frame);
}


int AudioDecoder::decoder_decode_frame() {
    int ret;

    do {

        if (pkt_queue.get_abort())
            return -1;
        if (!packet_pending || pkt_queue.get_serial() != pkt_serial) {

            if (pkt_queue.get_packet(&pkt) < 0) return -1;
//            av_packet_unref(&d->pkt);
//            d->pkt_temp = d->pkt = pkt;
//            d->packet_pending = 1;
        }
        if (pkt.data == NULL) {
            av_log(NULL, AV_LOG_FATAL, "reach eof.\n");
            return -1;
        }
        ret = avcodec_send_packet(avctx, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            break;
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avctx, frame);
        if (ret < 0 && ret != AVERROR_EOF)
            break;
        frame->pts = av_frame_get_best_effort_timestamp(frame);
        frame_queue.put_frame(frame);
        if (ret < 0) {
            packet_pending = 0;
        } else {

        }
    } while (ret != 0 && !finished);

    return 0;
}


void AudioDecoder::decode() {

    AVFrame *frame = av_frame_alloc();
    for (;;) {
        if (pkt_queue.get_abort()) break;
        int got;
        if ((got = decoder_decode_frame()) < 0)
            return;
    }
}


int AudioDecoder::get_channels() {
    return avctx->channels;
}

int AudioDecoder::get_sample_rate(){
    return avctx->sample_rate;
}




void EasyPlayer::init(const std::string input_filename) {
    if (input_filename.empty()) {
        av_log(nullptr, AV_LOG_FATAL, "An input file must be specified\n");
        return;
    }
    av_log(NULL, AV_LOG_INFO, "Easyplayer init on file %s.\n", input_filename.c_str());
    filename = av_strdup(input_filename.c_str());
    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    av_register_all();
    avformat_network_init();
    std::thread read_thread(&EasyPlayer::read, this);
    read_thread.detach();
}



void EasyPlayer::read() {
    int err, i, ret;
    av_log(NULL, AV_LOG_INFO, "start read thread.\n");
    AVPacket *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (pkt == NULL) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate avPacket.\n");
        return;
    }
    int64_t stream_start_time;
    int64_t pkt_ts;
    int pkt_in_play_range = 0;
    int st_index[AVMEDIA_TYPE_NB];
    memset(st_index, -1, sizeof(st_index));
    ic = avformat_alloc_context();
    if (!ic) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        return;
    }

    err = avformat_open_input(&ic, filename, NULL, NULL);
    if (err < 0) {
        av_log(NULL, AV_LOG_FATAL, "Could not open input file.\n");
        release();
    }
    this->ic = ic;
    err = avformat_find_stream_info(ic, NULL);
    if (err < 0) {
        av_log(NULL, AV_LOG_WARNING,
               "%s: could not find codec parameters\n", filename);
        release();
    }
    realtime = is_realtime();
    for(i = 0; i < ic->nb_streams; i++) {
        if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            st_index[AVMEDIA_TYPE_AUDIO] = i;
        }
        if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            st_index[AVMEDIA_TYPE_VIDEO] = i;
        }
    }
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        av_log(NULL, AV_LOG_INFO, "start open video component at id %d.\n",st_index[AVMEDIA_TYPE_VIDEO]);
        stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
    }
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        av_log(NULL, AV_LOG_INFO, "start open audio component at id %d.\n",st_index[AVMEDIA_TYPE_AUDIO]);
        stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
    }
    if (video_stream < 0 && audio_stream < 0) {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n", filename);
        release();
    }
    while(true) {
        if (abort_request)
            break;
        ret = av_read_frame(ic, pkt);
        if (ret < 0) {
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !eof) {
                if (video_stream >= 0)
                    viddec.pkt_queue.put_nullpacket();
                if (audio_stream >= 0)
                    auddec.pkt_queue.put_nullpacket();
                eof = 1;
            }
            if (ic->pb && ic->pb->error)
                break;
        } else {
            eof = 0;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */
        stream_start_time = ic->streams[pkt->stream_index]->start_time;
        pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                            (pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
                            av_q2d(ic->streams[pkt->stream_index]->time_base) -
                            (double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000
                            <= ((double)duration / 1000000);
        if (pkt->stream_index == audio_stream && pkt_in_play_range) {
            if (auddec.pkt_queue.put_packet(pkt) < 0) break;
        } else if (pkt->stream_index == video_stream && pkt_in_play_range
                   && !(video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            if (viddec.pkt_queue.put_packet(pkt) < 0) break;
        } else {
            av_packet_unref(pkt);
        }
    }
}



bool EasyPlayer::is_realtime() {
    if (ic == nullptr) return false;
    if(!strcmp(ic->iformat->name, "rtp")
       || !strcmp(ic->iformat->name, "rtsp")
       || !strcmp(ic->iformat->name, "sdp"))
        return true;

    if(ic->pb && (!strncmp(ic->filename, "rtp:", 4)
                 || !strncmp(ic->filename, "udp:", 4)))
        return 1;
    return 0;
}


int EasyPlayer::stream_component_open(int stream_index) {
    AVCodecContext *avctx;
    AVCodec *codec;
    int sample_rate, nb_channels;
    int64_t channel_layout;
    int ret = 0;
    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
        return AVERROR(ENOMEM);
    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avctx);
        return AVERROR(ENOMEM);
    }
    av_codec_set_pkt_timebase(avctx, ic->streams[stream_index]->time_base);
    codec = avcodec_find_decoder(avctx->codec_id);
    avctx->codec_id = codec->id;
    eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    ret = avcodec_open2(avctx, codec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_FATAL, "Fail to open codec on stream %d\n", stream_index);
        avcodec_free_context(&avctx);
        return ret;
    }
    switch (avctx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            swr_ctx = swr_alloc();
            swr_ctx = swr_alloc_set_opts(NULL,
                                         avctx->channel_layout, AV_SAMPLE_FMT_S16, avctx->sample_rate,
                                         avctx->channel_layout, avctx->sample_fmt, avctx->sample_rate,
                                         0, NULL);
            if (!swr_ctx || swr_init(swr_ctx) < 0) {
                av_log(NULL, AV_LOG_ERROR,
                       "Cannot create sample rate converter for conversion channels!\n");
                swr_free(&swr_ctx);
                return -1;
            }
            audio_stream = stream_index;
            audio_st = ic->streams[stream_index];
            auddec.init(avctx);
            auddec.start_decode_thread();
            break;
        case AVMEDIA_TYPE_VIDEO:
            video_stream = stream_index;
            video_st = ic->streams[stream_index];
            img_convert_ctx = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
                                             avctx->width, avctx->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
            viddec.init(avctx);
            viddec.start_decode_thread();
            break;
        default:
            break;
    }
    on_state_change(PlayerState::READY);
    return ret;
}





int VideoDecoder::decoder_decode_frame() {
    int ret;

    do {

        if (pkt_queue.get_abort())
            return -1;
        if (!packet_pending || pkt_queue.get_serial() != pkt_serial) {
//            do {
//                if (queue.get_packet(pkt) < 0) return -1;
//
//
//
//
//                if (queue.get_queue_size() == 0)
////                    SDL_CondSignal(d->empty_queue_cond);
//                if (packet_queue_get(d->queue, &pkt, 1, &d->pkt_serial) < 0)
//                    return -1;
//                if (pkt.data == flush_pkt.data) {
//                    avcodec_flush_buffers(d->avctx);
//                    d->finished = 0;
//                    d->next_pts = d->start_pts;
//                    d->next_pts_tb = d->start_pts_tb;
//                }
//            } while (pkt.data == flush_pkt.data || d->queue->serial != d->pkt_serial);
            if (pkt_queue.get_packet(&pkt) < 0) return -1;
            if (pkt.data == NULL) {
                av_log(NULL, AV_LOG_FATAL, "reach eof.\n");
                return -1;
            }
//            av_packet_unref(&d->pkt);
//            d->pkt_temp = d->pkt = pkt;
//            d->packet_pending = 1;
        }
        ret = avcodec_send_packet(avctx, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            av_log(NULL, AV_LOG_FATAL, "video avcodec_send_packet error %d.\n", ret);
            break;
        }

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avctx, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            av_log(NULL, AV_LOG_FATAL, "video avcodec_receive_frame error %d.\n", ret);
            break;
        }
        frame->pts = av_frame_get_best_effort_timestamp(frame);
        frame_queue.put_frame(frame);
        if (ret < 0) {
            packet_pending = 0;
        } else {

        }
    } while (ret != 0 && !finished);
    return 0;
}


bool EasyPlayer::has_video() {
    if (ic) {
        return video_stream >= 0;
    }
    return false;
}


bool EasyPlayer::get_img_frame(AVFrame *frame) {
    if (frame == nullptr) return false;
    auto av_frame = viddec.frame_queue.get_frame();
    sws_scale(img_convert_ctx, (const uint8_t* const*)av_frame->frame->data, av_frame->frame->linesize, 0, viddec.avctx->height,
              frame->data, frame->linesize);
    double timestamp = av_frame_get_best_effort_timestamp(av_frame->frame)*av_q2d(video_st->time_base);
    if (timestamp > audio_clock) {
        usleep((unsigned long)((timestamp - audio_clock)*1000000));
    }
    av_frame_unref(av_frame->frame);
    av_frame_free(&av_frame->frame);
    return true;
}


bool EasyPlayer::get_aud_buffer(int &nextSize, uint8_t *outputBuffer) {
    if (outputBuffer == nullptr) return false;
    auto av_frame = auddec.frame_queue.get_frame();
    if (auddec.avctx->sample_fmt == AV_SAMPLE_FMT_S16P) {
        nextSize = av_samples_get_buffer_size(av_frame->frame->linesize, auddec.avctx->channels, auddec.avctx->frame_size, auddec.avctx->sample_fmt, 1);
    }else {
        av_samples_get_buffer_size(&nextSize, auddec.avctx->channels, auddec.avctx->frame_size, auddec.avctx->sample_fmt, 1);
    }
    int ret = swr_convert(swr_ctx, &outputBuffer, av_frame->frame->nb_samples,
                      (uint8_t const **) (av_frame->frame->extended_data),
                          av_frame->frame->nb_samples);
    audio_clock = av_frame->frame->pkt_pts * av_q2d(audio_st->time_base);
    av_frame_unref(av_frame->frame);
    av_frame_free(&av_frame->frame);
    return ret >= 0;
}



void EasyPlayer::wait_state(PlayerState need_state) {
    std::unique_lock<std::mutex> lock(mutex);
    state_condition.wait(lock, [this, need_state] {
        return this->state >= need_state;
    });
}

void EasyPlayer::on_state_change(PlayerState state) {
    std::unique_lock<std::mutex> lock(mutex);
    this->state = state;
    state_condition.notify_all();
}

void EasyPlayer::release() {
    if (ic) {
        avformat_close_input(&ic);
    }
}


