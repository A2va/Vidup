#include "codecparam.h"

void CodecParam::width(int width)
{
    width_ = width;
}
void CodecParam::height(int height)
{
    height_ = height;
}
void CodecParam::time_base(AVRational time_base)
{
    time_base_ = time_base;
}
void CodecParam::framerate(AVRational framerate)
{
    framerate_ = framerate;
}
void CodecParam::pix_fmt(AVPixelFormat pix_fmt)
{
    pix_fmt_ =  pix_fmt;
}
void CodecParam::bit_rate(int64_t bit_rate)
{
    bit_rate_ = bit_rate;
}
void CodecParam::video_codec_id(enum AVCodecID  codec_id)
{
    video_codec_id_ = codec_id;
}

void CodecParam::sample_rate(int sample_rate)
{
    sample_rate_ = sample_rate;
}
void CodecParam::channel_layout(uint64_t channel_layout)
{
    channel_layout_ = channel_layout;
}
void CodecParam::sample_fmt(AVSampleFormat sample_fmt)
{
    sample_fmt_ = sample_fmt;
}
void CodecParam::audio_codec_id(enum AVCodecID codec_id)
{
    audio_codec_id_ = codec_id;
}


int CodecParam::width()
{
    return width_;
}
int CodecParam::height()
{
    return height_;
}
AVRational CodecParam::time_base()
{
    return time_base_;
}
AVRational CodecParam::framerate()
{
    return framerate_;
}
AVPixelFormat CodecParam::pix_fmt()
{
    return pix_fmt_;
}
int64_t CodecParam::bit_rate()
{
    return bit_rate_;
}
enum AVCodecID CodecParam::video_codec_id()
{
    return video_codec_id_;
}

int CodecParam::sample_rate()
{
    return sample_rate_;
}
uint64_t CodecParam::channel_layout()
{
    return channel_layout_;
}
AVSampleFormat CodecParam::sample_fmt()
{
    return sample_fmt_;
}
enum AVCodecID CodecParam::audio_codec_id()
{
    return audio_codec_id_;
}
