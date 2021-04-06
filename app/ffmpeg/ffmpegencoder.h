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

#ifndef FFMPEGENCODER_H
#define FFMPEGENCODER_H


extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <opencv2/opencv.hpp>
#include <string>

#include <QString>
#include <QDebug>

#include "codecparam.h"

class FFmpegEncoder
{
public:
  FFmpegEncoder(std::string filename);
  FFmpegEncoder(std::string filename, CodecParam &codec_param);
  ~FFmpegEncoder();

  // Call this method before open or use the second constructor 
  void SetCodecParam(CodecParam &codec_param);

  bool Open();
  void Close();

  bool InitSwsContext();

  void writeVideoFrame(AVFrame *frame);
  void writeVideoPacket(AVPacket *pkt);

  void writeAudioFrame(AVFrame *frame);
  void writeAudioPacket(AVPacket *pkt);

 void writeMatrix(cv::Mat &image,int64_t  pts);

  AVFrame *createFrame();
  void Matrix2Frame(const cv::Mat &image, AVFrame *frame);

  const AVStream * getVideoStream();

private:
  void writePktonStream(AVCodecContext *codec_ctx, AVStream *stream, AVPacket *pkt);

  bool encode(AVCodecContext *codec_ctx, AVStream *stream, AVFrame *frame);

  bool InitializeStream(AVMediaType type, AVStream **stream_ptr, AVCodecContext **codec_ctx_ptr, AVCodec *codec_ptr);
  bool SetupCodecContext(AVStream *stream, AVCodecContext *codec_ctx, AVCodec *codec);
  bool InitializeCodecContext(AVStream **stream, AVCodecContext **codec_ctx, AVCodec *codec);

  void InitPtr();
  void InitializeCodec();

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

  bool open_;

  CodecParam codec_param_;
};

#endif