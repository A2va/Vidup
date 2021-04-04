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

extern "C"
{
#include <libavutil/avutil.h>
}

#include "ffmpegerror.h"

FFmpegError::FFmpegError(const std::string &message,int error_code)
{
	char err[128];
	av_strerror(error_code, err, 128);

	std::string ffmpeg_message(err);

	error_code_= error_code;
	message_= message;
	ffmpeg_message_= ffmpeg_message;
}

FFmpegError::FFmpegError(const std::string &message)
{
	message_ = message;
	ffmpeg_message_ = "";
}

std::string FFmpegError::message()
{
	return message_ + ": "+  ffmpeg_message_;
}