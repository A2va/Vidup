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

#include <opencv2/opencv.hpp>
#include <opencv2/dnn_superres.hpp>

#include "ffmpeg_tests.h"

#include "ffmpeg/ffmpegencoder.h"
#include "ffmpeg/ffmpegdecoder.h"

#include "ffmpeg/ffmpegerror.h"

#include "ffmpeg/codecparam.h"

extern "C"
{
#include <libavutil/rational.h>
}

void encode_image1()
{
    std::string filename = "./tests/encode_image1.mp4";

    std::cout << "Test: Encode image 1" << std::endl;
    try
    {
        FFmpegEncoder encoder(filename);
        CodecParam codec_param;
        codec_param.video_codec_id(AV_CODEC_ID_H264);
        codec_param.width(1920);
        codec_param.height(1080);
        codec_param.bit_rate(400000);
        codec_param.time_base(av_make_q(1, 25));
        codec_param.framerate(av_make_q(25, 1));
        codec_param.pix_fmt(AV_PIX_FMT_YUV420P);

        codec_param.audio_codec_id(AV_CODEC_ID_NONE); // With this codec id the encoder doesn't create a audio stream

        encoder.SetCodecParam(codec_param);
        encoder.Open();
        encoder.InitSwsContext();

        AVFrame *frame;
        cv::Mat image = cv::imread("./tests/encode_image.png");
        frame = encoder.createFrame();
        encoder.Matrix2Frame(image, frame);

        // Encode 4 second of video at 25 fps
        for (int i = 0; i < 100; i++)
        {
            frame->pts = i;
            encoder.writeVideoFrame(frame);
        }

        encoder.Close();
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
    catch (FFmpegError &e)
    {
        std::cout << e.message();
    }
    std::cout << "Test: End Encode image 1" << std::endl;
}

void encode_image2()
{
    std::string filename = "./tests/encode_image2.mp4";

    std::cout << "Test: Encode image 2" << std::endl;
    try
    {
        FFmpegEncoder encoder(filename);

        CodecParam codec_param;
        codec_param.video_codec_id(AV_CODEC_ID_H264);
        codec_param.width(1920);
        codec_param.height(1080);
        codec_param.bit_rate(400000);
        codec_param.time_base(av_make_q(1, 25));
        codec_param.framerate(av_make_q(25, 1));
        codec_param.pix_fmt(AV_PIX_FMT_YUV420P);

        codec_param.audio_codec_id(AV_CODEC_ID_NONE); // With this codec id the encoder doesn't create a audio stream

        encoder.SetCodecParam(codec_param);

        encoder.Open();
        encoder.InitSwsContext();

        cv::Mat image = cv::imread("./tests/encode_image.png");

        // Encode 4 second of video at 25 fps
        for (int i = 0; i < 100; i++)
        {
            encoder.writeMatrix(image, i);
        }

        encoder.Close();
    }
    catch (FFmpegError &e)
    {
        std::cerr << e.message() << std::endl;
    }
    std::cout << "Test: End Encode image 2" << std::endl;
}

void decode_image1()
{
    std::string filename = "./tests/decode_image.mp4";

    std::cout << "Test: Decode image 1" << std::endl;
    try
    {
        FFmpegDecoder decoder(filename);

        decoder.Open();

        decoder.InitSwsContext();

        AVFrame *frame;
        frame = decoder.readVideoFrame();

        cv::Mat img = decoder.Frame2Matrix(frame);

        cv::imwrite("./tests/decode_image1.png", img);

        decoder.Close();
    }
    catch (FFmpegError &e)
    {
        std::cerr << e.message() << std::endl;
    }
    std::cout << "Test: End Decode image 1" << std::endl;
}

void decode_image2()
{
    std::string filename = "./tests/decode_image.mp4";

    std::cout << "Test: Decode image 2" << std::endl;
    try
    {
        FFmpegDecoder decoder(filename);

        decoder.Open();

        decoder.InitSwsContext();

        cv::Mat img = decoder.readMatrix();

        cv::imwrite("./tests/decode_image2.png", img);

        decoder.Close();
    }
    catch (FFmpegError &e)
    {
        std::cerr << e.message() << std::endl;
    }
    std::cout << "Test: End Decode image 2" << std::endl;
}

void decode_encode1()
{
    std::string filename = "./tests/decode_encode.mp4";
    std::string filename_out = "./tests/decode_encode_out.mp4";

    std::cout << "Test: Decode encode 1" << std::endl;
    try
    {
        FFmpegEncoder encoder(filename_out);
        FFmpegDecoder decoder(filename);

        decoder.Open();
        CodecParam codec_param = decoder.GetCodecParam();
        // Apparently the encoder needs different framerate and time base than the decoder
        // See this: https://gist.github.com/yohhoy/50ea5fe868a2b3695e19
        codec_param.time_base(av_make_q(1, 25));
        codec_param.framerate(av_make_q(25, 1));

        encoder.SetCodecParam(codec_param);
        encoder.Open();
        encoder.InitSwsContext();

        decoder.InitSwsContext();
        AVFrame *frame;

        for (int64_t i = 0; i < decoder.GetNumberAudioFrame(); i++)
        {
            frame = decoder.readAudioFrame();
            if (frame != nullptr)
            {
                encoder.writeAudioFrame(frame);
            }
        }

        av_frame_unref(frame);

        const AVStream *in_stream = decoder.getVideoStream();
        const AVStream *out_stream = encoder.getVideoStream();
        for (int64_t i = 0; i < decoder.GetNumberVideoFrame(); i++)
        {
            frame = decoder.readVideoFrame();
            if (frame != nullptr)
            {
                frame->pts = i; // Set presentation timestamp to current
                //av_rescale_q(frame->pts, in_stream->time_base, out_stream->time_base);
                encoder.writeVideoFrame(frame);
            }
        }

        decoder.Close();
        encoder.Close();
    }
    catch (FFmpegError &e)
    {
        std::cerr << e.message() << std::endl;
    }

    std::cout << "Test: End Decode encode 1" << std::endl;
}

void decode_encode2()
{
    std::string filename = "./tests/decode_encode.mp4";
    std::string filename_out = "./tests/decode_encode_out2.mp4";
    // Same test as decode_encode1 but with packet (not working at the moment)
    std::cout << "Test: Decode encode 2" << std::endl;
    try
    {
        FFmpegEncoder encoder(filename_out);
        FFmpegDecoder decoder(filename);

        decoder.Open();
        CodecParam codec_param = decoder.GetCodecParam();
        codec_param.time_base(av_make_q(1, 25));
        codec_param.framerate(av_make_q(25, 1));

        encoder.SetCodecParam(codec_param);
        encoder.Open();
        encoder.InitSwsContext();

        decoder.InitSwsContext();
        AVPacket *packet;

        // for (int64_t i = 0; i < decoder.GetNumberAudioFrame(); i++)
        // {
        //     packet = decoder.readAudioPacket();
        //     if(packet!=nullptr)
        //     {
        //         packet->pts=i;
        //         encoder.writeAudioPacket(packet);
        //     }
        // }

        //av_packet_unref(packet);
        const AVStream *in_stream = decoder.getVideoStream();
        const AVStream *out_stream = encoder.getVideoStream();
        for (int64_t i = 0; i < decoder.GetNumberVideoFrame(); i++)
        {
            packet = decoder.readVideoPacket();
            if (packet != nullptr)
            {
                packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, out_stream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                packet->dts = av_rescale_q_rnd(packet->dts, in_stream->time_base, out_stream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                packet->duration = av_rescale_q(packet->duration, in_stream->time_base, out_stream->time_base);
                packet->pos = -1;
                encoder.writeVideoPacket(packet);
            }
        }

        decoder.Close();
        encoder.Close();
    }
    catch (FFmpegError &e)
    {
        std::cerr << e.message() << std::endl;
    }

    std::cout << "Test: End Decode encode 2" << std::endl;
}

void upscale()
{
    std::string filename = "./tests/decode_encode.mp4";
    std::string filename_out = "./tests/decode_encode_out_up.mp4";

    std::cout << "Test: Decode encode 1" << std::endl;
    try
    {
        int scale = 2;
        cv::dnn_superres::DnnSuperResImpl sr;
        sr.readModel("./models/ESPCN_x2.pb");
        sr.setModel("espcn", scale);
        FFmpegEncoder encoder(filename_out);
        FFmpegDecoder decoder(filename);

        decoder.Open();
        CodecParam codec_param = decoder.GetCodecParam();
        // Apparently the encoder needs different framerate and time base than the decoder
        // See this: https://gist.github.com/yohhoy/50ea5fe868a2b3695e19
        codec_param.time_base(av_make_q(1, 25));
        codec_param.framerate(av_make_q(25, 1));

        codec_param.width(codec_param.width() * scale);
        codec_param.height(codec_param.height() * scale);

        encoder.SetCodecParam(codec_param);
        encoder.Open();
        encoder.InitSwsContext();

        decoder.InitSwsContext();

        AVFrame *frame_dec;
        AVFrame *frame_enc = encoder.createFrame();
        cv::Mat mat;
        cv::Mat mat_enc;

        const AVStream *in_stream = decoder.getVideoStream();
        const AVStream *out_stream = encoder.getVideoStream();
        for (int64_t i = 0; i < decoder.GetNumberVideoFrame(); i++)
        {
            mat = decoder.readMatrix();
            if (!mat.empty())
            {
                sr.upsample(mat, mat_enc);
                encoder.Matrix2Frame(mat_enc, frame_enc);

                frame_enc->pts = i; // Set presentation timestamp to current
                //av_rescale_q(frame->pts, in_stream->time_base, out_stream->time_base);

                encoder.writeVideoFrame(frame_enc);
                // av_frame_unref(frame_enc);
            }
        }

        for (int64_t i = 0; i < decoder.GetNumberAudioFrame(); i++)
        {
            frame_dec = decoder.readAudioFrame();
            if (frame_dec != nullptr)
            {
                encoder.writeAudioFrame(frame_dec);
            }
        }

        av_frame_unref(frame_dec);

        decoder.Close();
        encoder.Close();
    }
    catch (FFmpegError &e)
    {
        std::cerr << e.message() << std::endl;
    }

    std::cout << "Test: End Decode encode 1" << std::endl;
}