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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->inputButton, &QPushButton::released, this, &MainWindow::inputFile);
    connect(ui->outputButton, &QPushButton::released, this, &MainWindow::outputFile);
    connect(ui->runButton, &QPushButton::released, this, &MainWindow::run);
}

void MainWindow::run()
{

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
    delete ui;
}