lessThan(QT_VERSION, 5.5): {
error("Qt less than 5.5 is no longer supported. In order to compile Qt Bitcoin Trader you need update to Qt 5.5 and C++11");
}

QMAKE_MAC_SDK = macosx10.11
QMAKE_CFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-unused-function
QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-unused-function

TEMPLATE	= app
LANGUAGE	= C++
DEPENDPATH	+= .
INCLUDEPATH	+= .
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib


CONFIG	+= qt # release
CONFIG	+= c++11

 win32 { TARGET = ../Bin/QtBitcoinTrader }
!win32 { TARGET = QtBitcoinTrader }

QT += network script widgets
!win32 { QT += multimedia }

#exists("sapi.h"){ DEFINES += SAPI_ENABLED }
exists($$(WINDOWSSDKDIR)/Include/sapi.h){
  DEFINES += SAPI_ENABLED
  #win32 { !CONFIG(static) { LIBS += -lole32 } }
}

CONFIG(static) {
  greaterThan(QT_MAJOR_VERSION, 4): {
   QTPLUGIN.mediaservice=-
   QTPLUGIN.playlistformats=-
   QTPLUGIN.position=-
   QTPLUGIN.printsupport=-
   QTPLUGIN.bearer=-
   QTPLUGIN.accessible=-
   QTPLUGIN.sensors=-
   QTPLUGIN.sqldrivers=-
   QTPLUGIN.qmltooling=-
   QTPLUGIN.designer=-
   QTPLUGIN.iconengines=-
   QTPLUGIN.imageformats=-
   #win32 { DEFINES += STATIC_QT5_BUILD }
  }
}

win32 { LIBS += -lcrypt32 -leay32 -lssleay32 -luser32 -lgdi32 -ladvapi32 -lz -lws2_32 -lwinmm }
!win32 { LIBS += -lcrypto -lz }

mac{
LIBS += -framework CoreFoundation
LIBS += -framework ApplicationServices
}

#
# Headers
#
HEADERS += aboutdialog.h \
           addrulegroup.h \
           currencyinfo.h \
           currencypairitem.h \
           datafolderchusedialog.h \
           debugviewer.h \
           depthitem.h \
           depthmodel.h \
           exchange.h \
           exchange_bitfinex.h \
           exchange_bitstamp.h \
           exchange_btcchina.h \
           exchange_btce.h \
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
           rulesmodel.h \
           rulewidget.h \
           thisfeatureunderdevelopment.h \
           tradesitem.h \
           tradesmodel.h \
           translationdialog.h \
           translationline.h \
           updaterdialog.h \
           apptheme.h \
           logobutton.h \
           networkmenu.h \
           julybuttonmenu.h \
           scriptwidget.h \
           scriptobject.h \
           addscriptwindow.h \
           julylockfile.h \
           exchange_gocio.h \
           featuredexchangesdialog.h \
           allexchangesdialog.h \
           allexchangesmodel.h \
           exchangebutton.h \
           addruledialog.h \
           rulescriptparser.h \
           ruleholder.h \
           exchange_indacoin.h \
           julymath.h \
           exchange_bitcurex.h \
           exchange_bitmarket.h \
		   exchange_okcoin.h \
           platform/sound.h \
           platform/socket.h \
           config/config_manager.h \
           config/config_manager_dialog.h \
           utils/utils.h \
           dock/dock_host.h \
           charts/chartsview.h \
           charts/chartsmodel.h \
           settingsdialog.h \
           settingsgeneral.h \
           settingsnetworkproxy.h \
           settingsdialoglistelement.h \
           settingsdecimals.h \
    timesync.h \
    translationmessage.h \
    indicatorengine.h \
    news/newsview.h \
    news/newsmodel.h

FORMS += addrulegroup.ui \
         datafolderchusedialog.ui \
         debugviewer.ui \
         feecalculator.ui \
         newpassworddialog.ui \
         passworddialog.ui \
         percentpicker.ui \
         qtbitcointrader.ui \
         rulewidget.ui \
         thisfeatureunderdevelopment.ui \
         translationabout.ui \
         translationdialog.ui \
         updaterdialog.ui \
         logobutton.ui \
         networkmenu.ui \
         scriptwidget.ui \
         addscriptwindow.ui \
         featuredexchangesdialog.ui \
         allexchangesdialog.ui \
         exchangebutton.ui \
         addruledialog.ui \
         config/config_manager_dialog.ui \
         charts/chartsview.ui \
         settingsdialog.ui \
         settingsgeneral.ui \
         settingsnetworkproxy.ui \
         settingsdialoglistelement.ui \
         settingsdecimals.ui \
    translationmessage.ui \
    news/newsview.ui

SOURCES += aboutdialog.cpp \
           addrulegroup.cpp \
           currencypairitem.cpp \
           datafolderchusedialog.cpp \
           debugviewer.cpp \
           depthitem.cpp \
           depthmodel.cpp \
           exchange.cpp \
           exchange_bitfinex.cpp \
           exchange_bitstamp.cpp \
           exchange_btcchina.cpp \
           exchange_btce.cpp \
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
           rulesmodel.cpp \
           rulewidget.cpp \
           thisfeatureunderdevelopment.cpp \
           tradesitem.cpp \
           tradesmodel.cpp \
           translationdialog.cpp \
           translationline.cpp \
           updaterdialog.cpp \
           apptheme.cpp \
           logobutton.cpp \
           networkmenu.cpp \
           julybuttonmenu.cpp \
           scriptwidget.cpp \
           scriptobject.cpp \
           addscriptwindow.cpp \
           julylockfile.cpp \
           exchange_gocio.cpp \
           featuredexchangesdialog.cpp \
           allexchangesdialog.cpp \
           allexchangesmodel.cpp \
           exchangebutton.cpp \
           addruledialog.cpp \
           rulescriptparser.cpp \
           ruleholder.cpp \
           exchange_indacoin.cpp \
           exchange_bitcurex.cpp \
           exchange_bitmarket.cpp \
		   exchange_okcoin.cpp \
           platform/sound.cpp \
           platform/socket.cpp \
           config/config_manager.cpp \
           config/config_manager_dialog.cpp \
           utils/utils.cpp \
           dock/dock_host.cpp \
           charts/chartsview.cpp \
           charts/chartsmodel.cpp \
           settingsdialog.cpp \
           settingsgeneral.cpp \
           settingsnetworkproxy.cpp \
           settingsdialoglistelement.cpp \
           settingsdecimals.cpp \
    timesync.cpp \
    translationmessage.cpp \
    indicatorengine.cpp \
    news/newsview.cpp \
    news/newsmodel.cpp

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

macx:ICON = $${PWD}/QtBitcoinTrader.icns
macx:QMAKE_INFO_PLIST = $${PWD}/QtBitcoinTrader.plist
