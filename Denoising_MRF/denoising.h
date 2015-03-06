#ifndef DENOISING_H
#define DENOISING_H

#include <QMainWindow>
#include <stdio.h>
#include<iostream>
#include "/home/lex/2_Sistemic/3_Markov_random_fields/2_Libs/maxflow-v3.03/graph.h"
#include "opencv2/opencv.hpp"
#include <QString>
#include <QDebug>
#include<QFileDialog>


using namespace cv;
using namespace std;

namespace Ui {
class Denoising;
}

class Denoising : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Denoising(QWidget *parent = 0);
    ~Denoising();

    //images
    cv::Mat img_src;//source image
    cv::Mat img_out;//output image
    cv::Mat img_srcGray;//source image gray scaled
    cv::Mat img_srcBinary;//source image binary
    //PATHS
    string inputName;//path to image


    //Capacities

    std::vector<int> sinkCapacities;
    std::vector<int> sourceCapacities;


    
private slots:
    void on_pushButton_released();

    void on_pushButton_2_released();

private:
    Ui::Denoising *ui;
};

#endif // DENOISING_H
