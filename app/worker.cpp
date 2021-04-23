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

#include "worker.h"
#include <iostream>

void Worker::doWork(const QString &parameter)
{
    qDebug() << parameter; // Display the input parameter

    QString result = "End of work"; // Get parameter of the object has emitted the signal

    for (int i = 0; i <= 100; i++) // Some working stuff
    {
        qDebug() << i;
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
