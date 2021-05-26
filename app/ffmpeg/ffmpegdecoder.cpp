
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

#include "ffmpegdecoder.h"
#include "ffmpegerror.h"

extern "C"
{
#include <libavutil/rational.h>
#include <libavutil/imgutils.h>
}

FFmpegDecoder::FFmpegDecoder(std::string filename)
{
	filename_ = filename;
}

CodecParam FFmpegDecoder::getCodecParam()
{
	CodecParam codec_param;

	if (video_codec_ctx_ != nullptr)
	{

		// Store all codec parameters
		codec_param.width(video_codec_ctx_->width);
		codec_param.height(video_codec_ctx_->height);
		codec_param.framerate(video_codec_ctx_->framerate);
		codec_param.time_base(video_codec_ctx_->time_base);
		codec_param.pix_fmt(video_codec_ctx_->pix_fmt);
		codec_param.bit_rate(video_codec_ctx_->bit_rate);
		codec_param.video_codec_id(video_codec_ctx_->codec_id);
	}
	else
	{
		// Set none codec
		codec_param.video_codec_id(AV_CODEC_ID_NONE);
	}

	if(audio_codec_ctx_!= nullptr)
	{
		// Store all codec parameters
		codec_param.sample_rate(audio_codec_ctx_->sample_rate);
		codec_param.channel_layout(audio_codec_ctx_->channel_layout);
		codec_param.sample_fmt(audio_codec_ctx_->sample_fmt);
		codec_param.audio_codec_id(audio_codec_ctx_->codec_id);
	}
	else
	{
		// Set none codec
		codec_param.audio_codec_id(AV_CODEC_ID_NONE);
	}

	return codec_param;
}

bool FFmpegDecoder::Open()
{
	// If already open
	if (open_)
	{
		return true;
	}

	int error_code = 0;
	// Open format context
	error_code = avformat_open_input(&fmt_ctx_, filename_.c_str(), nullptr, nullptr);
	if (error_code != 0)
	{
		throw FFmpegError("Decoder.Open: Failed to alloc output context", error_code);
		return false;
	}

	// Find stream info in format ctx
	error_code = avformat_find_stream_info(fmt_ctx_, nullptr);
	if (error_code < 0)
	{
		throw FFmpegError("Decoder.Open: Failed to find stream info:", error_code);
	}

	find_video_ = false;
	find_audio_ = false;

	// Find video or audio streams
	for (unsigned int i = 0; i < fmt_ctx_->nb_streams; i++)
	{
		if ((fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) && (find_video_ == false))
		{
			find_video_ = true;
			video_stream_ = fmt_ctx_->streams[i];
		}
		if ((fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) && (find_audio_ == false))
		{
			find_audio_ = true;
			audio_stream_ = fmt_ctx_->streams[i];
		}
	}

	// If video or audio founded, init the codec context
	if (find_video_)
	{
		InitCodecContext(video_stream_, &video_codec_, &video_codec_ctx_);
	}
	if (find_audio_)
	{
		InitCodecContext(audio_stream_, &audio_codec_, &audio_codec_ctx_);
	}

	//Allocate frame and packet for future usage
	pkt_ = av_packet_alloc();
	frame_ = av_frame_alloc();

	if (!frame_)
	{
		throw FFmpegError("Decoder.Open: Failed to alloc frame");
		return false;
	}
	if (!pkt_)
	{
		throw FFmpegError("Decoder.Open: Failed to alloc packet");
		return false;
	}

	// Set the decoder as open
	open_ = true;
	return true;
}

cv::Mat FFmpegDecoder::Frame2Matrix(AVFrame *frame)
{
	int width = video_stream_->codecpar->width;
	int height = video_stream_->codecpar->height;

	// Alloc a temp frame
	AVFrame *framebgr = nullptr;
	framebgr = av_frame_alloc();
	if (!framebgr)
	{
		throw FFmpegError("Decoder.convertFrameToMatrix: Failed to alloc frame");
		return cv::Mat{};
	}

	// Set the format and size of frame
	framebgr->format = static_cast<AVPixelFormat>(AV_PIX_FMT_BGR24);
	framebgr->width = width;
	framebgr->height = height;
	// Alloc the image buffer
	int error_code = av_image_alloc(framebgr->data, framebgr->linesize, framebgr->width, framebgr->height, AV_PIX_FMT_BGR24, 1);
	if (error_code < 0)
	{
		throw FFmpegError("Decoder.convertFrameToMatrix: Failed to alloc image", error_code);
		return cv::Mat{};
	}

	// Scale the original frame to bgr frame
	sws_scale(sws_ctx_, frame->data, frame->linesize, 0, height, framebgr->data, framebgr->linesize);

	// Convert the bgr frame to Matrix
	cv::Mat mat(height, width, CV_8UC3, framebgr->data[0], framebgr->linesize[0]);
	av_frame_free(&framebgr);
	return mat;
}

bool FFmpegDecoder::InitSwsContext()
{
	if (!find_video_)
	{
		return false;
	}
	// Init the sws context
	int width = video_stream_->codecpar->width;
	int height = video_stream_->codecpar->height;
	AVPixelFormat fmt = static_cast<AVPixelFormat>(video_stream_->codecpar->format);
	sws_ctx_ = sws_getContext(width, height, fmt, width, height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	if (!sws_ctx_)
	{
		throw FFmpegError("Decoder.InitSwsContext: Sws context allocation error");
		return false;
	}

	return true;
}

void FFmpegDecoder::Close()
{
	// Free all pointer
	if (sws_ctx_)
	{
		sws_freeContext(sws_ctx_);
		sws_ctx_ = nullptr;
	}

	if (video_codec_ctx_)
	{
		avcodec_free_context(&video_codec_ctx_);
		video_codec_ctx_ = nullptr;
	}

	if (audio_codec_ctx_)
	{
		avcodec_free_context(&audio_codec_ctx_);
		audio_codec_ctx_ = nullptr;
	}

	if (frame_)
	{
		av_frame_free(&frame_);
		frame_ = nullptr;
	}

	if (pkt_)
	{
		av_packet_free(&pkt_);
		pkt_ = nullptr;
	}

	if (fmt_ctx_)
	{
		// NOTE: This also frees video_stream_ and audio_stream_
		avformat_free_context(fmt_ctx_);
		fmt_ctx_ = nullptr;
	}
	open_=false;
}
const AVStream *FFmpegDecoder::getVideoStream()
{
	return video_stream_;
}

int64_t FFmpegDecoder::GetNumberAudioFrame()
{
	if (!find_audio_)
	{
		return -1;
	}
	return audio_stream_->nb_frames;
}

int64_t FFmpegDecoder::GetNumberVideoFrame()
{
	if (!find_video_)
	{
		return -1;
	}
	return video_stream_->nb_frames;
}

void FFmpegDecoder::readVideoPacket(AVPacket *pkt)
{
	if (!find_video_)
	{
		return;
	}
	readPacketonStream(video_codec_ctx_, video_stream_, pkt);
}

void FFmpegDecoder::readVideoFrame(AVFrame *frame)
{
	if (!find_video_)
	{
		return;
	}
	int ret = decode(video_codec_ctx_, video_stream_, frame);
	if (ret < 0)
	{
		frame = nullptr;
	}
}

void FFmpegDecoder::readAudioPacket(AVPacket *pkt)
{
	if (!find_audio_)
	{
		return;
	}
	readPacketonStream(audio_codec_ctx_, audio_stream_, pkt);
}

void FFmpegDecoder::readAudioFrame(AVFrame *frame)
{
	if (!find_audio_)
	{
		return;
	}
	int ret = decode(audio_codec_ctx_, audio_stream_, frame);
	if (ret < 0)
	{
		frame = nullptr;
	}
}

AVPacket *FFmpegDecoder::readVideoPacket()
{
	if (!find_video_)
	{
		return nullptr;
	}
	readPacketonStream(video_codec_ctx_, video_stream_, pkt_);
	return pkt_;
}

AVFrame *FFmpegDecoder::readVideoFrame()
{
	if (!find_video_)
	{
		return nullptr;
	}
	int ret = decode(video_codec_ctx_, video_stream_, frame_);
	if (ret < 0)
	{
		return nullptr;
	}

	return frame_;
}

cv::Mat FFmpegDecoder::readMatrix()
{
	AVFrame *frame;
	frame = readVideoFrame();
	if(frame == nullptr)
	{
		return cv::Mat{};
	}
	return Frame2Matrix(frame);
}

AVPacket *FFmpegDecoder::readAudioPacket()
{
	if (!find_audio_)
	{
		return nullptr;
	}
	readPacketonStream(audio_codec_ctx_, audio_stream_, pkt_);
	return pkt_;
}

AVFrame *FFmpegDecoder::readAudioFrame()
{
	if (!find_audio_)
	{
		return nullptr;
	}

	int ret = decode(audio_codec_ctx_, audio_stream_, frame_);
	if (ret < 0)
	{
		return nullptr;
	}
	return frame_;
}

int FFmpegDecoder::readPacketonStream(AVCodecContext *codec_ctx, AVStream *stream, AVPacket *pkt)
{
	int ret;
	// Free buffer in packet if there is one
	av_packet_unref(pkt);
	if (fifo_packet_[stream->index].size() > 0)
	{
		AVPacket *front_pkt = fifo_packet_[stream->index].front();
		// Move the data reference of front_pkt
		// to pkt and free front_pkt
		av_packet_move_ref(pkt, front_pkt);
		// Free front_packet
		av_packet_free(&front_pkt);
		//Pop the fifo
		fifo_packet_[stream->index].pop();
		return 0;
	}
	do
	{
		// Free buffer in packet if there is one
		av_packet_unref(pkt);

		// Read packet from file
		ret = av_read_frame(fmt_ctx_, pkt);

		// If the packet come from different stream index save it
		if (pkt->stream_index != stream->index)
		{
			// av_packet_clone = av_packet_alloc + av_packet_ref
			fifo_packet_[pkt->stream_index].push(av_packet_clone(pkt));
		}

	} while (pkt->stream_index != stream->index && ret >= 0);

	return ret;
}

int FFmpegDecoder::decode(AVCodecContext *codec_ctx, AVStream *stream, AVFrame *frame)
{
	bool eof = false;

	int ret;

	// Clear any previous frames
	av_frame_unref(frame);

	while ((ret = avcodec_receive_frame(codec_ctx, frame)) == AVERROR(EAGAIN) && !eof)
	{

		ret = readPacketonStream(codec_ctx, stream, pkt_);

		if (ret == AVERROR_EOF)
		{
			// Don't break so that receive gets called again, but don't try to read again
			eof = true;

			// Send a null packet to signal end of
			avcodec_send_packet(codec_ctx, nullptr);
		}
		else if (ret < 0)
		{
			// Handle other error by breaking loop and returning the code we received
			break;
		}
		else
		{
			// Successful read, send the packet
			ret = avcodec_send_packet(codec_ctx, pkt_);
			// We don't need the packet anymore, so free it
			av_packet_unref(pkt_);

			if (ret < 0)
			{
				break;
			}
		}
	}

	return ret;
}

bool FFmpegDecoder::InitCodecContext(AVStream *stream, AVCodec **codec, AVCodecContext **codec_ctx)
{
	int error_code;
	// Find the codec from stream
	*codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!codec)
	{
		throw FFmpegError("Decoder.InitCodecContext: Codec not found");
		return false;
	}
	// Alloc the codec context
	*codec_ctx = avcodec_alloc_context3(*codec);
	if (codec_ctx == nullptr)
	{
		throw FFmpegError("Decoder.InitCodecContext: Failed to allocate codec context");
		return false;
	}
	// Copy the stream codec parameters to codec contexts
	error_code = avcodec_parameters_to_context(*codec_ctx, stream->codecpar);
	if (error_code < 0)
	{
		throw FFmpegError("Decoder.InitCodecContext: Failed to copy parameters from stream to codec context");
		return false;
	}
	// Open the codec context
	error_code = avcodec_open2(*codec_ctx, *codec, NULL);
	if (error_code < 0)
	{
		throw FFmpegError("Decoder.InitCodecContext: Failed to open codec", error_code);
		return false;
	}
	return true;
}

FFmpegDecoder::~FFmpegDecoder()
{
	Close();
}