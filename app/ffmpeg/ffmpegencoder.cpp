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

#include "ffmpegencoder.h"
#include "ffmpegerror.h"

extern "C"
{
#include <libavutil/rational.h>
#include <libavutil/imgutils.h>
}

FFmpegEncoder::FFmpegEncoder(std::string filename)
{
	InitPtr();
	filename_ = filename;
	open_ = false;
}

FFmpegEncoder::FFmpegEncoder(std::string filename, CodecParam &codec_param)
{
	InitPtr();
	codec_param_ = codec_param;

	InitializeCodec();

	open_ = false;
}

void FFmpegEncoder::SetCodecParam(CodecParam &codec_param)
{
	codec_param_ = codec_param;
	InitializeCodec();
}

bool FFmpegEncoder::Open()
{
	int error_code;
	if (open_)
	{
		return true;
	}

	error_code = avformat_alloc_output_context2(&fmt_ctx_, nullptr, nullptr, filename_.c_str());
	if (error_code < 0)
	{
		throw FFmpegError("Encoder.Open: Failed to alloc output context", error_code);
		return false;
	}

	if (video_codec_ != nullptr)
	{
		InitializeStream(AVMEDIA_TYPE_VIDEO, &video_stream_, &video_codec_ctx_, video_codec_);
	}
	if (audio_codec_ != nullptr)
	{
		InitializeStream(AVMEDIA_TYPE_AUDIO, &audio_stream_, &audio_codec_ctx_, audio_codec_);
	}

	// Open output file for writing
	error_code = avio_open(&fmt_ctx_->pb, filename_.c_str(), AVIO_FLAG_WRITE);
	if (error_code < 0)
	{
		throw FFmpegError("Encoder.Open: Failed to open IO context", error_code);
		return false;
	}

	error_code = avformat_write_header(fmt_ctx_, nullptr);
	if (error_code < 0)
	{
		throw FFmpegError("Encoder.Open: Write header error", error_code);
		return false;
	}

	//Allocate frame and packet for future usage
	pkt_ = av_packet_alloc();
	frame_ = av_frame_alloc();
	if (!frame_)
	{
		throw FFmpegError("Encoder.Open: Could not allocate frame");
		return false;
	}
	if (!pkt_)
	{
		throw FFmpegError("Encoder.Open: Could not allocate frame");
		return false;
	}

	open_ = true;
	return true;
}

const AVStream *FFmpegEncoder::getVideoStream()
{
	return video_stream_;
}

void FFmpegEncoder::writeAudioFrame(AVFrame *frame)
{
	encode(audio_codec_ctx_, audio_stream_, frame);
}

void FFmpegEncoder::writeAudioPacket(AVPacket *pkt)
{
	writePktonStream(audio_codec_ctx_, audio_stream_, pkt);
}

void FFmpegEncoder::writeVideoFrame(AVFrame *frame)
{
	encode(video_codec_ctx_, video_stream_, frame);
}

void FFmpegEncoder::writeVideoPacket(AVPacket *pkt)
{
	writePktonStream(video_codec_ctx_, video_stream_, pkt);
}

void FFmpegEncoder::writeMatrix(cv::Mat &image, int64_t pts)
{
	frame_->format = static_cast<AVPixelFormat>(video_stream_->codecpar->format);
	frame_->width = video_stream_->codecpar->width;
	frame_->height = video_stream_->codecpar->height;
	int error_code = av_image_alloc(frame_->data, frame_->linesize, frame_->width, frame_->height, static_cast<AVPixelFormat>(frame_->format), 1);

	if (error_code < 0)
	{
		throw FFmpegError("Encoder.createFrame: Could not alloc image", error_code);
		return;
	}

	Matrix2Frame(image, frame_);
	frame_->pts = pts;
	writeVideoFrame(frame_);
	av_frame_unref(frame_);
}

AVFrame *FFmpegEncoder::createFrame()
{
	AVFrame *frame = av_frame_alloc();
	frame->format = static_cast<int>(video_stream_->codecpar->format);

	frame->width = video_stream_->codecpar->width;
	frame->height = video_stream_->codecpar->height;
	int error_code = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, static_cast<AVPixelFormat>(frame->format), 1);

	char err[128];
	av_strerror(error_code, err, 128);
	if (error_code < 0)
	{
		throw FFmpegError("Encoder.createFrame: Could not alloc image", error_code);
		return nullptr;
	}
	// av_frame_unref(frame);
	return frame;
}

void FFmpegEncoder::Matrix2Frame(const cv::Mat &image, AVFrame *frame)
{
	const uint8_t *rgbData[1] = {(const uint8_t *)image.data};
	int rgbLineSize[1] = {3 * image.cols};

	sws_scale(sws_ctx_, rgbData, rgbLineSize, 0, image.rows, frame->data, frame->linesize);
}

bool FFmpegEncoder::InitSwsContext()
{
	int width = video_stream_->codecpar->width;
	int height = video_stream_->codecpar->height;
	AVPixelFormat fmt = static_cast<AVPixelFormat>(video_stream_->codecpar->format);
	sws_ctx_ = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, fmt, 0, NULL, NULL, NULL);
	if (!sws_ctx_)
	{
		throw FFmpegError("Encoder.InitSwsContext: Sws context init error");
		return false;
	}

	return true;
}

bool FFmpegEncoder::InitializeStream(AVMediaType type, AVStream **stream_ptr, AVCodecContext **codec_ctx_ptr, AVCodec *codec_ptr)
{

	if (!InitializeCodecContext(stream_ptr, codec_ctx_ptr, codec_ptr))
	{
		return false;
	}

	AVCodecContext *codec_ctx = *codec_ctx_ptr;
	AVStream *stream = *stream_ptr;

	switch (type)
	{
	case AVMEDIA_TYPE_VIDEO:
		// codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		// codec_ctx->time_base = av_make_q(1,25);
		// codec_ctx->framerate = av_make_q(25,1);

		codec_ctx->width = codec_param_.width();
		codec_ctx->height = codec_param_.height();
		codec_ctx->bit_rate = codec_param_.bit_rate();
		codec_ctx->time_base = codec_param_.time_base();
		codec_ctx->framerate = codec_param_.framerate();
		codec_ctx->pix_fmt = codec_param_.pix_fmt();
		break;

	case AVMEDIA_TYPE_AUDIO:
		codec_ctx->sample_rate = codec_param_.sample_rate();
		codec_ctx->channel_layout = codec_param_.channel_layout();
		codec_ctx->channels = av_get_channel_layout_nb_channels(codec_ctx->channel_layout);
		codec_ctx->sample_fmt = codec_param_.sample_fmt();
		codec_ctx->time_base = av_make_q(1, codec_param_.sample_rate());
		break;
	default:
		break;
	}

	if (!SetupCodecContext(stream, codec_ctx, codec_ptr))
	{
		return false;
	}
	return false;
}

void FFmpegEncoder::InitializeCodec()
{
	if (codec_param_.video_codec_id() != AV_CODEC_ID_NONE)
	{
		video_codec_ = avcodec_find_encoder(codec_param_.video_codec_id());
		if (!video_codec_)
		{
			throw FFmpegError("Encoder: Codec not found");
		}
	}

	if (codec_param_.audio_codec_id() != AV_CODEC_ID_NONE)
	{
		audio_codec_ = avcodec_find_encoder(codec_param_.audio_codec_id());
		if (!audio_codec_)
		{
			throw FFmpegError("Encoder: Codec not found");
		}
	}
}

bool FFmpegEncoder::InitializeCodecContext(AVStream **stream, AVCodecContext **codec_ctx, AVCodec *codec)
{
	*stream = avformat_new_stream(fmt_ctx_, nullptr);
	if (!(*stream))
	{
		throw FFmpegError("Encoder.InitializeCodecContext:Failed to alloc stream");
		return false;
	}

	// Allocate a codec context
	*codec_ctx = avcodec_alloc_context3(codec);
	if (!(*codec_ctx))
	{
		throw FFmpegError("Encoder.InitializeCodecContext: Failed to alloc codec context");
		return false;
	}

	return true;
}

bool FFmpegEncoder::SetupCodecContext(AVStream *stream, AVCodecContext *codec_ctx, AVCodec *codec)
{
	int error_code;

	if (fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
	{
		codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	
	AVDictionary *codec_opts = nullptr;
	if(codec_param_.thread()==0)
	{
		av_dict_set(&codec_opts, "threads", "auto", 0);
	}
	else
	{
    	av_dict_set(&codec_opts, "threads", std::to_string(codec_param_.thread()).c_str(), 0);
	}
	// Try to open encoder
	error_code = avcodec_open2(codec_ctx, codec, &codec_opts);
	if (error_code < 0)
	{
		throw FFmpegError("Encoder.SetupCodecContext: Failed to open encoder", error_code);
		return false;
	}

	// Copy context settings to codecpar object
	error_code = avcodec_parameters_from_context(stream->codecpar, codec_ctx);
	if (error_code < 0)
	{
		throw FFmpegError("Encoder.SetupCodecContext: Failed to copy codec parameters to stream", error_code);
		return false;
	}

	return true;
}

void FFmpegEncoder::Close()
{
	if (open_)
	{
		//Flush the encoder
		encode(video_codec_ctx_, video_stream_, NULL);

		av_write_trailer(fmt_ctx_);
		avio_closep(&fmt_ctx_->pb);

		open_ = false;
	}

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
}

void FFmpegEncoder::writePktonStream(AVCodecContext *codec_ctx, AVStream *stream, AVPacket *pkt)
{
	pkt_->stream_index = stream->index;

	av_packet_rescale_ts(pkt_, codec_ctx->time_base, stream->time_base);

	// Write packet to file
	av_interleaved_write_frame(fmt_ctx_, pkt_);
}

bool FFmpegEncoder::encode(AVCodecContext *codec_ctx, AVStream *stream, AVFrame *frame)
{
	int error_code = 0;
	/* send the frame to the encoder */
	error_code = avcodec_send_frame(codec_ctx, frame);
	if (error_code < 0)
	{
		throw FFmpegError("Encoder.encode: Error sending a frame for encoding", error_code);
		return false;
	}
	while (error_code >= 0)
	{
		error_code = avcodec_receive_packet(codec_ctx, pkt_);

		//// EAGAIN just means the encoder wants another frame before encoding
		if (error_code == AVERROR(EAGAIN) || error_code == AVERROR_EOF)
		{
			//return false;
			break;
		}
		else if (error_code < 0)
		{
			throw FFmpegError("Encoder.encode: Error during encoding", error_code);
		}

		writePktonStream(codec_ctx, stream, pkt_);

		// Unref packet in case we're getting another
		av_packet_unref(pkt_);
	}
	return true;
}

FFmpegEncoder::~FFmpegEncoder()
{
	Close();
}

void FFmpegEncoder::InitPtr()
{
	fmt_ctx_ = nullptr;

	video_codec_ctx_ = nullptr;
	audio_codec_ctx_ = nullptr;

	video_codec_ = nullptr;
	audio_codec_ = nullptr;

	video_stream_ = nullptr;
	audio_stream_ = nullptr;

	sws_ctx_ = nullptr;

	frame_ = nullptr;
	pkt_ = nullptr;
}