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
           addrulegroup.h \
           addrulewindow.h \
           audioplayer.h \
           currencyinfo.h \
           currencypairitem.h \
           datafolderchusedialog.h \
           debugviewer.h \
           depthitem.h \
           depthmodel.h \
           donatebtc.h \
           donatepanel.h \
           donatewebmoney.h \
           exchange.h \
           exchange_bitfinex.h \
           exchange_bitstamp.h \
           exchange_btcchina.h \
           exchange_btce.h \
           exchange_cryptsy.h \
           exchange_mtgox.h \
           exchangemtgox.h \
           feecalculator.h \
           historyitem.h \
           historymodel.h \
           julyaes256.h \
           julyhttp.h \
           julylightchanges.h \
           julyrsa.h \
           julyscrolluponidle.h \
           julyspinboxfix.h \
           julyspinboxpicker.h \
           julytranslator.h \
           logthread.h \
           main.h \
           newpassworddialog.h \
           orderitem.h \
           ordersmodel.h \
           orderstablecancelbutton.h \
           passworddialog.h \
           percentpicker.h \
           qtbitcointrader.h \
           ruleholder.h \
           rulesmodel.h \
           rulewidget.h \
           thisfeatureunderdevelopment.h \
           tradesitem.h \
           tradesmodel.h \
           translationdialog.h \
           translationline.h \
           updaterdialog.h \
           apptheme.h
FORMS += addrulegroup.ui \
         addrulewindow.ui \
         datafolderchusedialog.ui \
         debugviewer.ui \
         donatebtc.ui \
         donatepanel.ui \
         donatewebmoney.ui \
         feecalculator.ui \
         newpassworddialog.ui \
         passworddialog.ui \
         percentpicker.ui \
         qtbitcointrader.ui \
         rulewidget.ui \
         thisfeatureunderdevelopment.ui \
         translationabout.ui \
         translationdialog.ui \
         updaterdialog.ui
SOURCES += aboutdialog.cpp \
           addrulegroup.cpp \
           addrulewindow.cpp \
           audioplayer.cpp \
           currencypairitem.cpp \
           datafolderchusedialog.cpp \
           debugviewer.cpp \
           depthitem.cpp \
           depthmodel.cpp \
           donatebtc.cpp \
           donatepanel.cpp \
           donatewebmoney.cpp \
           exchange.cpp \
           exchange_bitfinex.cpp \
           exchange_bitstamp.cpp \
           exchange_btcchina.cpp \
           exchange_btce.cpp \
           exchange_cryptsy.cpp \
           exchange_mtgox.cpp \
           exchangemtgox.cpp \
           feecalculator.cpp \
           historyitem.cpp \
           historymodel.cpp \
           julyaes256.cpp \
           julyhttp.cpp \
           julylightchanges.cpp \
           julyrsa.cpp \
           julyscrolluponidle.cpp \
           julyspinboxfix.cpp \
           julyspinboxpicker.cpp \
           julytranslator.cpp \
           logthread.cpp \
           main.cpp \
           newpassworddialog.cpp \
           orderitem.cpp \
           ordersmodel.cpp \
           orderstablecancelbutton.cpp \
           passworddialog.cpp \
           percentpicker.cpp \
           qtbitcointrader.cpp \
           ruleholder.cpp \
           rulesmodel.cpp \
           rulewidget.cpp \
           thisfeatureunderdevelopment.cpp \
           tradesitem.cpp \
           tradesmodel.cpp \
           translationdialog.cpp \
           translationline.cpp \
           updaterdialog.cpp \
           apptheme.cpp
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
