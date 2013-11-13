#
# Basic stuff
#
TEMPLATE	= app
LANGUAGE        = C++
TARGET 		= QtBitcoinTrader
DEPENDPATH 	+= .
QT		+= network
INCLUDEPATH 	+= .
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib
INCLUDEPATH    += /usr/include/QtMultimediaKit
INCLUDEPATH    += /usr/include/QtMobility
MOBILITY       = multimedia

win32 {
LIBS		+= -lcrypt32 -llibeay32 -lssleay32 -luser32 -lgdi32 -ladvapi32
}
!win32 {
LIBS		+= -lcrypto -lz -lQtMultimediaKit
}

CONFIG		+= qt warn_off release mobility

#
# Headers
#
HEADERS += aboutdialog.h \
           addrulewindow.h \
           feecalculator.h \
           julyaes256.h \
           julylightchanges.h \
           julyrsa.h \
           julyhttp.h \
           julyspinboxfix.h \
           julytranslator.h \
           logthread.h \
           main.h \
           newpassworddialog.h \
           passworddialog.h \
           qtbitcointrader.h \
           ruleholder.h \
           exchange_mtgox.h \
           translationdialog.h \
           translationline.h \
           updaterdialog.h \
           audioplayer.h \
           exchange_btce.h \
           datafolderchusedialog.h \
           tradesmodel.h \
           depthmodel.h \
           exchange_bitstamp.h \
           rulesmodel.h \
           orderitem.h \
           ordersmodel.h \
           historymodel.h \
           exchange_btcchina.h \
           historyitem.h \
           exchange.h \
           debugviewer.h \
           julyscrolluponidle.h
#
# Forms
#
FORMS += addrulewindow.ui \
         feecalculator.ui \
         newpassworddialog.ui \
         passworddialog.ui \
         qtbitcointrader.ui \
         translationabout.ui \
         translationdialog.ui \
         updaterdialog.ui \
         datafolderchusedialog.ui \
         debugviewer.ui
#
# Sources
#
SOURCES += aboutdialog.cpp \
           addrulewindow.cpp \
           feecalculator.cpp \
           julyaes256.cpp \
           julylightchanges.cpp \
           julyrsa.cpp \
           julyhttp.cpp \
           julyspinboxfix.cpp \
           julytranslator.cpp \
           logthread.cpp \
           main.cpp \
           newpassworddialog.cpp \
           passworddialog.cpp \
           qtbitcointrader.cpp \
           ruleholder.cpp \
           exchange_mtgox.cpp \
           translationdialog.cpp \
           translationline.cpp \
           updaterdialog.cpp \
           audioplayer.cpp \
           exchange_btce.cpp \
           datafolderchusedialog.cpp \
           tradesmodel.cpp \
           depthmodel.cpp \
           exchange_bitstamp.cpp \
           rulesmodel.cpp \
           ordersmodel.cpp \
           historymodel.cpp \
           exchange_btcchina.cpp \
           exchange.cpp \
           debugviewer.cpp \
           julyscrolluponidle.cpp
#
# Resources
# 
RESOURCES += QtResource.qrc

#
# Platform dependent stuff
#
unix:!macx {
UI_DIR = .ui
MOC_DIR = .moc
OBJECTS_DIR = .obj
isEmpty( PREFIX ) {
    PREFIX=/usr
}
isEmpty( DESKTOPDIR ) {
    DESKTOPDIR=/usr/share/applications
}
isEmpty( ICONDIR ) {
    ICONDIR=/usr/share/pixmaps
}

target.path = $${PREFIX}/bin

INSTALLS = target

desktop.path = $${DESKTOPDIR}

desktop.files = QtBitcoinTrader.desktop
INSTALLS += desktop

icon.path = $${ICONDIR}

icon.files = QtBitcoinTrader.png
INSTALLS += icon
}
################################
win32 {
RC_FILE = WinResource.rc
}
