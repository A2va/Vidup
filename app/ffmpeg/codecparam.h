#ifndef CODECPARAM_H
#define CODECPARAM_H

extern "C"
{
#include <libavutil/rational.h>
#include <libavutil/pixfmt.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/codec_id.h>
}

class CodecParam
{
public:
// Set 
// Video parameters
void width(int width);
void height(int height);
void time_base(AVRational time_base);
void framerate(AVRational framerate);
void pix_fmt(AVPixelFormat pix_fmt);
void bit_rate(int64_t bit_rate);
void video_codec_id(enum AVCodecID  codec_id);

// Audio parameters
void sample_rate(int sample_rate);
void channel_layout(uint64_t channel_layout);
void sample_fmt(AVSampleFormat sample_fmt);
void audio_codec_id(enum AVCodecID  codec_id);

// Get
// Video parameters
int width();
int height();
AVRational time_base();
AVRational framerate();
AVPixelFormat pix_fmt();
int64_t bit_rate();
enum AVCodecID video_codec_id();

// Audio parameters
int sample_rate();
uint64_t channel_layout();
AVSampleFormat sample_fmt();
enum AVCodecID audio_codec_id();

private:
    // Video parameters
    int width_;
    int height_;
    AVRational time_base_;
    AVRational framerate_;
    AVPixelFormat pix_fmt_;
    int64_t bit_rate_;
    enum AVCodecID video_codec_id_;
    
    // Audio parameters
    int sample_rate_;
    uint64_t channel_layout_;
    AVSampleFormat sample_fmt_;
    enum AVCodecID audio_codec_id_;

};

#endif
