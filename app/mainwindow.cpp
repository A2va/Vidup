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
#include <QButtonGroup>

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

    connect(ui->inputFile,&QLineEdit::textChanged,this,&MainWindow::inputFileChanged);
    connect(ui->outputFile,&QLineEdit::textChanged,this,&MainWindow::outputFileChanged);

    connect(ui->buttonGroup,qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked),this,&MainWindow::algorithmChanged);
    
    // Setup  the worker

    Worker *worker_ = new Worker;
    // Move the worker inside workerThread
    worker_->moveToThread(&workerThread);
    // Connect the QThread signal finished to delete worker 
    connect(&workerThread, &QThread::finished, worker_, &QObject::deleteLater);
    // Connect signal operate to doWork of working 
    // emit operate will start the work
    connect(this, &MainWindow::operate, worker_, &Worker::doWork);
    // Connect the signal resulReady to handleResult
    connect(worker_, &Worker::resultReady, this, &MainWindow::handleResults);
    //Start the thread 
    workerThread.start();

    worker_->setInputFile(input_filename_);
    worker_->setOutputFile(output_filename_);
    worker_->setAlgorithm(algorithm_);
}

void MainWindow::algorithmChanged(QAbstractButton *button)
{
    algorithm_ = button->text().toLower();
}

void MainWindow::inputFileChanged(const  QString &s)
{
    input_filename_ = s;
    ui->inputFile->setText(input_filename_);
}

void MainWindow::outputFileChanged(const  QString &s)
{
    output_filename_ = s;
    ui->outputFile->setText(output_filename_);
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
    QString file = QFileDialog::getOpenFileName(this, "Open Video File", "", "All Files (*)");
    inputFileChanged(file);
}

void MainWindow::outputFile()
{
    QString file = QFileDialog::getOpenFileName(this, "Open Video File", "", "All Files (*)");
    outputFileChanged(file);
}

MainWindow::~MainWindow()
{
    workerThread.quit();
    workerThread.wait();
    delete ui;
}