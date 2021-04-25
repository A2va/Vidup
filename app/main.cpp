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

#include <QApplication>

#include <string>
#include <iostream>
#include <filesystem>

#include "ffmpeg_tests.h"

#include "mainwindow.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

extern "C"
{
    #include <libavutil/avutil.h>
}
#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P /* default pix_fmt */

int main(int argc, char **argv)
{

    av_log_set_level(AV_LOG_DEBUG);

    // encode_image1();
    // encode_image2();

    // decode_image1();
    // decode_image2();

    // decode_encode1();
    // decode_encode2();
    
    // upscale();

    QApplication app(argc, argv);
    MainWindow win;
    win.show();
    return app.exec();

    return 0;
}
