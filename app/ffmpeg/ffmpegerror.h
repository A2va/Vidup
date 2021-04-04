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

#ifndef FFMPEGERROR_H
#define FFMPEGERROR_H

#include <stdexcept>


class FFmpegError: public std::exception
{
public:
  FFmpegError(const std::string &message, int error_code);
  FFmpegError(const std::string &message);
  
  std::string message();
 

private:
	int error_code_;
  std::string ffmpeg_message_;
  std::string message_;
};

#endif // FFMPEGERROR_HEADER_H