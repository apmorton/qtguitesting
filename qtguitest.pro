#-------------------------------------------------
#
# Project created by QtCreator 2013-10-26T16:07:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = qtguitest
TEMPLATE = app

LIBS += "$$_PRO_FILE_PWD_/vendor/portaudio/lib/libportaudio.a" -lasound

SOURCES += main.cpp \
    mainwindow.cpp \
    qfreqbin.cpp \
    vendor/qcustomplot/qcustomplot.cpp

INCLUDEPATH += vendor \
    vendor/portaudio/include \
    vendor/qcustomplot

HEADERS  += mainwindow.h \
    qfreqbin.h \
    vendor/ffft/OscSinCos.hpp \
    vendor/ffft/OscSinCos.h \
    vendor/ffft/FFTRealUseTrigo.hpp \
    vendor/ffft/FFTRealUseTrigo.h \
    vendor/ffft/FFTRealSelect.hpp \
    vendor/ffft/FFTRealSelect.h \
    vendor/ffft/FFTRealPassInverse.hpp \
    vendor/ffft/FFTRealPassInverse.h \
    vendor/ffft/FFTRealPassDirect.hpp \
    vendor/ffft/FFTRealPassDirect.h \
    vendor/ffft/FFTRealFixLenParam.h \
    vendor/ffft/FFTRealFixLen.hpp \
    vendor/ffft/FFTRealFixLen.h \
    vendor/ffft/FFTReal.hpp \
    vendor/ffft/FFTReal.h \
    vendor/ffft/DynArray.hpp \
    vendor/ffft/DynArray.h \
    vendor/ffft/def.h \
    vendor/ffft/Array.hpp \
    vendor/ffft/Array.h \
    vendor/portaudio/include/portaudio.h \
    vendor/portaudio/include/pa_linux_alsa.h \
    vendor/portaudio/include/pa_win_wmme.h \
    vendor/portaudio/include/pa_win_wdmks.h \
    vendor/portaudio/include/pa_win_waveformat.h \
    vendor/portaudio/include/pa_win_wasapi.h \
    vendor/portaudio/include/pa_win_ds.h \
    vendor/portaudio/include/pa_asio.h \
    vendor/qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui

OTHER_FILES +=
