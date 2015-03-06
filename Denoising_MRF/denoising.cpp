#include "denoising.h"
#include "ui_denoising.h"

Denoising::Denoising(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Denoising)
{
    ui->setupUi(this);
}

Denoising::~Denoising()
{
    delete ui;
}


/***************************************************************************************************
Methods
Authors:
Alexander Gómez villa (alexander.gomezv@udea.edu.co)
German Diez Valencia  (german.diezv@udea.edu.co)
***************************************************************************************************/

std::vector<int> cutCapacities(cv::Mat img,int lamda,int xp)
{
    std::vector<int> out;
    for(int i=0;i<img.cols-1;i++)
    {
        for(int j=0;j<img.rows -1;j++)
        {
            if(img.at<int>(j,i)>1) out.push_back(pow((xp-1),2));

            else

                 out.push_back(pow((xp-img.at<int>(j,i)),2));
        }
    }
    return out;
}

/***************************************************************************************************
Methods
Authors:
Alexander Gómez villa (alexander.gomezv@udea.edu.co)
German Diez Valencia  (german.diezv@udea.edu.co)
***************************************************************************************************/




/***************************************************************************************************
Methods
Authors:
Alexander Gómez villa (alexander.gomezv@udea.edu.co)
German Diez Valencia  (german.diezv@udea.edu.co)
***************************************************************************************************/


void Denoising::on_pushButton_released()
{
    //ABRE VENTANA CARGA ARCHIVO
    QString fileName = QFileDialog::getOpenFileName(this,
    tr("Open Video"), ".",
    tr("Video Files(*.png *.jpg *.jpeg *.bmp *.tif)"));
    inputName=fileName.toStdString().c_str();
    img_src=imread(inputName);

    //MUESTRA IMAGEN EN INTERFAZ
    QImage img= QImage((const unsigned char*)(img_src.data),img_src.cols,img_src.rows,img_src.step,QImage::Format_RGB888);//extrae imagen
    ui->label->setPixmap(QPixmap::fromImage(img));//muestra en label el video

}

/***************************************************************************************************
Methods
Authors:
Alexander Gómez villa (alexander.gomezv@udea.edu.co)
German Diez Valencia  (german.diezv@udea.edu.co)
***************************************************************************************************/

void Denoising::on_pushButton_2_released()
{

    ////Structures for Maxflow--------
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType( img_src.cols*img_src.rows, img_src.cols*img_src.rows*4);

    ////image preprocessing---------
    cvtColor(img_src,img_srcGray,cv::COLOR_RGB2GRAY);//chenge to gray scale
    threshold(img_srcGray,img_srcBinary,0,255,cv::THRESH_OTSU);//binarize  image
    imshow("In",img_srcBinary);//show image

    ///Graph building----------

    sinkCapacities=cutCapacities(img_srcBinary,1,1);
    sourceCapacities=cutCapacities(img_srcBinary,1,1);

    //Node setting
    for(int i=0;i<img_src.cols*img_src.rows-1;i++)
    {

        g -> add_node();//add node to graph
        g -> add_tweights( i,sinkCapacities[i],sourceCapacities[i]);//add weight beetween sink-node and source-node

    }

    //Edge setting
    int n=0;//node counter

    for(int i=0;i<img_src.cols-1;i++)
    {
        for(int j=0;j<img_src.rows -1;j++)
        {


            if(i>0 && i<img_src.cols-1 && j>0 && j<img_src.rows-1)
            {

               // qDebug()<<i<<pow(img_srcBinary.at<int>(j,i)-img_srcBinary.at<int>(j,i-1),2);
                qDebug()<<j<<img_srcBinary.rows<<i<<img_srcBinary.cols<<( img_srcBinary.cols*(j+1)+i)<<( img_srcBinary.cols*(j)+i)<<img_src.cols*img_src.rows-1<<img_srcBinary.cols*(j)+i-1;
                //qDebug()<<i<<(img_srcBinary.at<int>(j,i));
                g -> add_edge( n, n+1,pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j,i+1),2), pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j,i+1),2));
                qDebug()<<"Derecha";
                g -> add_edge( n, n-1,pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j,i-1),2),pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j,i-1),2) );
                qDebug()<<"izquierda";
                g -> add_edge( n,( img_srcBinary.cols*(j)+i),pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j+1,i),2),pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j+1,i),2));
                qDebug()<<"Arriba";
                g -> add_edge( n,( img_srcBinary.cols*(j+1)+i-1),pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j-1,i),2),pow(img_srcBinary.at<uchar>(j,i)-img_srcBinary.at<uchar>(j-1,i),2));
                qDebug()<<"abajo";



            }
             n++;

//            else
//            {


//            }


        }
    }



    //Maxflow solution

      int flow = g -> maxflow();


      //Edge setting
      int n2=0;//node counter


      for(int i=0;i<img_src.cols-1;i++)
      {
          for(int j=0;j<img_src.rows -1;j++)
          {


              if(i>0 && i<img_src.cols-1 && j>0 && j<img_src.rows-1)
              {


                 // img_srcBinary.at<uchar>(j,i)=255;
              if (g->what_segment(n2) == GraphType::SINK)
              {
                  img_srcBinary.at<uchar>(j,i)=0;
                  qDebug()<<"Estoy en Source!!";


              }
//              else

//                  img_srcBinary.at<uchar>(j,i)=255;


                }

               n2++;


          }
      }


      //MUESTRA IMAGEN EN INTERFAZ
      QImage img2= QImage((const unsigned char*)(img_srcBinary.data),img_srcBinary.cols,img_srcBinary.rows,img_srcBinary.step,QImage::Format_Mono);//extrae imagen
      ui->label_2->setPixmap(QPixmap::fromImage(img2));//muestra en label el video


    imshow("Output",img_srcBinary);//show image
}
