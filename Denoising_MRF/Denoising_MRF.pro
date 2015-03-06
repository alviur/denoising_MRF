#-------------------------------------------------
#
# Project created by QtCreator 2015-01-05T17:06:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Denoising_MRF
TEMPLATE = app



unix{
    CONFIG += link_pkgconfig
    PKGCONFIG +=opencv
}
win32{
    INCLUDEPATH += C:\opencv246bin\install\include \
                   C:\opencv246bin\install\include\opencv \
                   C:\opencv246bin\install\include\opencv2

    LIBS += -LC:\\opencv246bin\\install\\lib \
            -lopencv_core246.dll \
            -lopencv_highgui246.dll \
            -lopencv_imgproc246.dll \
            -lopencv_features2d246.dll \
            -lopencv_calib3d246.dll \
            -lopencv_video246.dll \
            -lopencv_videostab246.dll

}


SOURCES += main.cpp\
        denoising.cpp \
    ../../2_Libs/maxflow-v3.03/maxflow.cpp \
    ../../2_Libs/maxflow-v3.03/graph.cpp

HEADERS  += denoising.h \
    ../../2_Libs/maxflow-v3.03/instances.inc \
    ../../2_Libs/maxflow-v3.03/graph.h

FORMS    += denoising.ui
