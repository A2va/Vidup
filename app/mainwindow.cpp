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

#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "worker.h"
#include <iostream>
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->inputButton, &QPushButton::released, this, &MainWindow::inputFile);
    connect(ui->outputButton, &QPushButton::released, this, &MainWindow::outputFile);
    connect(ui->runButton, &QPushButton::released, this, &MainWindow::run);

    // Setup  the worker

    Worker *worker = new Worker;
    // Move the worker inside workerThread
    worker->moveToThread(&workerThread);
    // Connect the QThread signal finished to delete worker 
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    // Connect signal operate to doWork of working 
    // emit operate will start the work
    connect(this, &MainWindow::operate, worker, &Worker::doWork);
    // Connect the signal resulReady to handleResult
    connect(worker, &Worker::resultReady, this, &MainWindow::handleResults);
    //Start the thread 
    workerThread.start();

}

void MainWindow::run()
{
    //Emit the signal to run the working function
    emit operate("Start the work");
}

void MainWindow::handleResults(const QString &s)
{
    std::cout << s.toStdString() << std::endl;
}

void MainWindow::inputFile()
{
    input_filename_ = QFileDialog::getOpenFileName(this, "Open Video File", "", "All Files (*)");
}

void MainWindow::outputFile()
{
    output_filename_ = QFileDialog::getOpenFileName(this, "Open Video File", "", "All Files (*)");
}

MainWindow::~MainWindow()
{
    workerThread.quit();
    workerThread.wait();
    delete ui;
}