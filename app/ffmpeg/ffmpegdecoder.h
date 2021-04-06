// Copyright (C) 2021  A2va

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include <string>
#include <iostream>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <opencv2/opencv.hpp>

#include <QString>
#include <QDebug>

#include <queue>

class FFmpegDecoder
{
public:
  FFmpegDecoder(std::string filename);
  ~FFmpegDecoder();

  CodecParam GetCodecParam();

  bool Open();
  void Close();

  bool InitSwsContext();


  /**
   * All read frame and packet used internal pointer that free as close
   * decoder. If you want to save the packet you must clone it,
   * otherwise there will be overwritten.
  */  
  AVPacket *readVideoPacket();
  AVFrame *readVideoFrame();

  AVPacket *readAudioPacket();
  AVFrame *readAudioFrame();

  /**
   * For use this function you MUST alloc the packet of frame and pass it
   * to the function. No need to call av_frame_unref (or packet) before but 
   * free the frame (or packet) after you use it.
  */
  void readVideoPacket(AVPacket *pkt);
  void readVideoFrame(AVFrame *frame);

  void readAudioPacket(AVPacket *pkt);
  void readAudioFrame(AVFrame *frame);

  cv::Mat readMatrix();

  int64_t GetNumberVideoFrame();
  int64_t GetNumberAudioFrame();

  cv::Mat Frame2Matrix(AVFrame *frame);

  const AVStream * getVideoStream();

private:
  void InitPtr();

  int readPacketonStream(AVCodecContext *codec_ctx, AVStream *stream, AVPacket *pkt);

  bool InitCodecContext(AVStream *stream, AVCodec **codec, AVCodecContext **codec_ctx);
  int decode(AVCodecContext *codec_ctx, AVStream *stream, AVFrame *frame);

  AVFormatContext *fmt_ctx_;
  AVStream *video_stream_;
  AVCodec *video_codec_;
  AVCodecContext *video_codec_ctx_;

  AVStream *audio_stream_;
  AVCodec *audio_codec_;
  AVCodecContext *audio_codec_ctx_;

  AVFrame *frame_;
  AVPacket *pkt_;

  struct SwsContext *sws_ctx_;

  std::string filename_;

  std::queue<AVPacket*> fifo_packet_[2];
  
  bool find_video_;
  bool find_audio_;

  bool open_;
};

#endif