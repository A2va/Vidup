#ifndef FRAME_H
#define FRAME_H

#include <libavcodec/avcodec.h>
#include <opencv2/opencv.hpp>
#include "ffmpegerror.h"


struct Frame
{

    void Frame(AVFrame *frame)
    {
        frame_ = frame;
    }

    void Frame(cv::Mat &mat)
    {
        setMatrix(mat);
    }

    void setFrame(AVFrame *frame)
    {
        frame_ = frame;
    }

    void setMatrix(cv::Mat &mat)
    {
        // Get rgb data

        if(frame_ == nullptr)
        {
            frame_ = av_frame_alloc();
        }

        struct SwsContext *sws_ctx = get_SwsCtx(mat.cols, mat.rows, AV_PIX_FMT_BGR24, AV_PIX_FMT_YUV420P);
        if(sws_ctx == nullptr)
        {
            return;
        }

        const uint8_t *rgbData[1] = {static_cast<const uint8_t*>(mat.data)};

        int rgbLineSize[1] = {3 * image.cols};

        sws_scale(sws_ctx, rgbData, rgbLineSize, 0, image.rows, frame->data, frame->linesize);

        sws_freeContext(sws_ctx);
    }

    AVFrame *getFrame()
    {
        return frame_;
    }

    cv::Mat getMatrix()
    {
        struct SwsContext *sws_ctx = get_SwsCtx(frame_->width, frame->height,frame->format, AV_PIX_FMT_BGR24);
        if(sws_ctx == nullptr)
        {
            return;
        }
    

        AVFrame *framebgr = nullptr;
        framebgr = av_frame_alloc();
        if (!framebgr)
        {
            throw FFmpegError("Frame.getMatrix: Failed to alloc frame");
            return cv::Mat{};
        }

        // Set the format and size of frame
        framebgr->format = static_cast<AVPixelFormat>(AV_PIX_FMT_BGR24);
        framebgr->width = frame->width;
        framebgr->height = frame->height;
        // Alloc the image buffer
        int error_code = av_image_alloc(framebgr->data, framebgr->linesize, framebgr->width, framebgr->height, AV_PIX_FMT_BGR24, 1);
        if (error_code < 0)
        {   
            av_frame_free(&framebgr);
            throw FFmpegError("Frame.getMatrix: Failed to alloc image", error_code);
            return cv::Mat{};
        }

        // Scale the original frame to bgr frame
        sws_scale(sws_ctx_, frame_->data, frame_->linesize, 0, height, framebgr->data, framebgr->linesize);

        // Convert the bgr frame to Matrix
        cv::Mat mat(height, width, CV_8UC3, framebgr->data[0], framebgr->linesize[0]);

        av_frame_free(&framebgr);
        sws_freeContext(sws_ctx);

        return mat;
    }

private:

    AVFrame *frame_;

    struct SwsContext * get_SwsCtx(int width, int height, AVPixelFormat srcFmt, AVPixelFormat destFmt)
    {
        // Init the sws context
        AVPixelFormat fmt = static_cast<AVPixelFormat>(video_stream_->codecpar->format);
        struct SwsContext *sws_ctx = sws_getContext(width, height, srcFmt, width, height, destFmt, SWS_BICUBIC, NULL, NULL, NULL);
        if (!sws_ctx_)
        {
            throw FFmpegError("Decoder.InitSwsContext: Sws context allocation error");
            return nullptr;
        }

        return sws_ctx;
    }
};



#endif
