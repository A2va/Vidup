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

#include <QDebug>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn_superres.hpp>

#include "worker.h"
#include "ffmpeg/ffmpegencoder.h"
#include "ffmpeg/ffmpegdecoder.h"

#include "ffmpeg/ffmpegerror.h"

#include "ffmpeg/codecparam.h"

void Worker::doWork()
{
    QString result = "End of work"; // Get parameter of the object has emitted the signal

    cv::dnn_superres::DnnSuperResImpl sr;
   
    sr.readModel(QString("./models/%1_x%2.pb").arg(algorithm_,QString::number(scale_)).toStdString());
    sr.setModel(algorithm_.toStdString(), scale_);

    // Create encoder and decoder object
    FFmpegEncoder encoder(output_filename_.toStdString());
    FFmpegDecoder decoder(input_filename_.toStdString());
	try
    {

        // Open decoder and get stream and codec param
        decoder.Open();
        const AVStream *in_stream = decoder.getVideoStream();
        CodecParam codec_param = decoder.getCodecParam();

        // Set some codec parameters
        codec_param.time_base(av_inv_q(in_stream->r_frame_rate));
        codec_param.framerate(in_stream->r_frame_rate);

        codec_param.width(codec_param.width() * scale_);
        codec_param.height(codec_param.height() * scale_);

        // Set codec param and open
        encoder.setCodecParam(codec_param);
        encoder.Open();

        // Init Sws context
        decoder.InitSwsContext();
        encoder.InitSwsContext();

		
	}
	catch(FFmpegError &e)
	{
        qCritical() << QString::fromStdString(e.message());
        encoder.Close();
        decoder.Close();
        return;
	}
	AVFrame *frame_enc;
	try
	{
        AVFrame *frame_dec;
        frame_enc = encoder.createFrame();
        cv::Mat mat;
        cv::Mat mat_enc;

        // Upscale video
        for (int64_t i = 0; i < decoder.GetNumberVideoFrame(); i++)
        {
            qDebug() << "Processing video frame" << i << "number";
            mat = decoder.readMatrix();
            if (!mat.empty())
            {
                sr.upsample(mat, mat_enc);
                encoder.Matrix2Frame(mat_enc, frame_enc);
                
                frame_enc->pts = i; // Set presentation timestamp to current

                encoder.writeVideoFrame(frame_enc);
            }
        }

        // Audio
        for (int64_t i = 0; i < decoder.GetNumberAudioFrame(); i++)
        {
            qDebug() << "Processing audio frame" << i << "number";
            frame_dec = decoder.readAudioFrame();
            if (frame_dec != nullptr)
            {
                encoder.writeAudioFrame(frame_dec);
            }
        }
	}
	catch(FFmpegError &e)
	{
        qCritical() << QString::fromStdString(e.message());
        av_frame_unref(frame_enc);
        av_frame_free(&frame_enc);
        decoder.Close();
        encoder.Close();
        return;
	}
	
	
    emit resultReady(result); // Emit a signal to tell that finish
}

void Worker::setInputFile(const QString &file)
{
    input_filename_ = file;
}

void Worker::setOutputFile(const QString &file)
{
    output_filename_ = file;
}

void Worker::setScale(int scale)
{
    scale_ = scale;
}

void Worker::setAlgorithm(const QString &algorithm)
{
    algorithm_ = algorithm;
}
