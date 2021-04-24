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


#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QThread>
#include <QString>

class Worker : public QObject
{
Q_OBJECT

private:
    QString output_filename_;
    QString input_filename_;
    QString algorithm_;
    int scale_;

public:
    void setInputFile(const QString &file);
    void setOutputFile(const QString &file);
    void setScale(int scale);
    void setAlgorithm(const QString &algorithm);

public Q_SLOTS:
    void doWork();

Q_SIGNALS:
    void resultReady(const QString &result);
};

#endif