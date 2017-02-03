lessThan(QT_VERSION, 5.5): {
error("Qt less than 5.5 is no longer supported. In order to compile Qt Bitcoin Trader you need update to Qt 5.5 and C++11");
}

TEMPLATE	= app
LANGUAGE	= C++
DEPENDPATH	+= .
INCLUDEPATH	+= .

CONFIG	+= qt release c++11

 win32 { TARGET = ../Bin/QtBitcoinTrader }
!win32 { TARGET = QtBitcoinTrader }

QT += network script widgets
!win32 { QT += multimedia }

win32 {
    LIBS += -llibcrypto -llibssl -lz

    exists($$(WINDOWSSDKDIR)/Include/um/sapi.h){
        QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
        QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
        QMAKE_CFLAGS -= -Zc:strictStrings
        QMAKE_CXXFLAGS -= -Zc:strictStrings
        DEFINES += SAPI_ENABLED
    }

    checkFRAMEWORKDIR=$$(FRAMEWORKDIR)
    isEmpty(checkFRAMEWORKDIR) {
        QMAKE_CFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-unused-function
        QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-unused-function -Wno-reorder

        DEFINES += SAPI_ENABLED
        LIBS += -lsapi
    }
}

!win32 { LIBS += -lcrypto -lssl -lz }

mac {
    QMAKE_MAC_SDK = macosx10.12
    QMAKE_CFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-unused-function
    QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-unused-function

    LIBS += -dead_strip
    LIBS += -framework CoreFoundation
    LIBS += -framework ApplicationServices
}

CONFIG(static) {
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
    QTPLUGIN.geoservices=-
    QTPLUGIN.position=-
    QTPLUGIN.qmltooling=-
    QTPLUGIN.sensorgestures=-
}

#
# Headers
#
HEADERS += script/addrulegroup.h \
           script/rulesmodel.h \
           script/rulewidget.h \
           script/scriptwidget.h \
           script/scriptobject.h \
           script/addscriptwindow.h \
           script/addruledialog.h \
           script/rulescriptparser.h \
           script/ruleholder.h \
           script/scriptobjectthread.h \
           platform/sound.h \
           platform/socket.h \
           config/config_manager.h \
           config/config_manager_dialog.h \
           utils/utils.h \
           settings/settingsdialog.h \
           settings/settingsgeneral.h \
           settings/settingsnetworkproxy.h \
           settings/settingsdialoglistelement.h \
           settings/settingsdecimals.h \
           dock/dock_host.h \
           charts/chartsview.h \
           charts/chartsmodel.h \
           news/newsview.h \
           news/newsmodel.h \
           aboutdialog.h \
           currencyinfo.h \
           currencypairitem.h \
           datafolderchusedialog.h \
           debugviewer.h \
           depthitem.h \
           depthmodel.h \
           exchange/exchange.h \
           exchange/exchange_bitfinex.h \
           exchange/exchange_bitstamp.h \
           exchange/exchange_btcchina.h \
           exchange/exchange_btce.h \
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
           login/newpassworddialog.h \
           orderitem.h \
           ordersmodel.h \
           orderstablecancelbutton.h \
           login/passworddialog.h \
           percentpicker.h \
           qtbitcointrader.h \
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
           julylockfile.h \
           exchange/exchange_gocio.h \
           login/featuredexchangesdialog.h \
           login/allexchangesdialog.h \
           login/allexchangesmodel.h \
           login/exchangebutton.h \
           exchange/exchange_indacoin.h \
           julymath.h \
           exchange/exchange_bitcurex.h \
           exchange/exchange_bitmarket.h \
           exchange/exchange_okcoin.h \
           timesync.h \
           translationmessage.h \
           indicatorengine.h

FORMS += script/addrulegroup.ui \
         script/rulewidget.ui \
         script/scriptwidget.ui \
         script/addscriptwindow.ui \
         script/addruledialog.ui \
         config/config_manager_dialog.ui \
         charts/chartsview.ui \
         news/newsview.ui \
         settings/settingsdialog.ui \
         settings/settingsgeneral.ui \
         settings/settingsnetworkproxy.ui \
         settings/settingsdialoglistelement.ui \
         settings/settingsdecimals.ui \
         datafolderchusedialog.ui \
         debugviewer.ui \
         feecalculator.ui \
         login/newpassworddialog.ui \
         login/passworddialog.ui \
         percentpicker.ui \
         qtbitcointrader.ui \
         thisfeatureunderdevelopment.ui \
         translationabout.ui \
         translationdialog.ui \
         updaterdialog.ui \
         logobutton.ui \
         networkmenu.ui \
         login/featuredexchangesdialog.ui \
         login/allexchangesdialog.ui \
         login/exchangebutton.ui \
         translationmessage.ui

SOURCES += script/addrulegroup.cpp \ 
           script/rulesmodel.cpp \
           script/rulewidget.cpp \    
           script/scriptwidget.cpp \
           script/scriptobject.cpp \
           script/addscriptwindow.cpp \
           script/addruledialog.cpp \
           script/rulescriptparser.cpp \
           script/ruleholder.cpp \
           script/scriptobjectthread.cpp \
           platform/sound.cpp \
           platform/socket.cpp \
           config/config_manager.cpp \
           config/config_manager_dialog.cpp \
           utils/utils.cpp \
           dock/dock_host.cpp \
           charts/chartsview.cpp \
           charts/chartsmodel.cpp \
           news/newsview.cpp \
           news/newsmodel.cpp \
           settings/settingsdialog.cpp \
           settings/settingsgeneral.cpp \
           settings/settingsnetworkproxy.cpp \
           settings/settingsdialoglistelement.cpp \
           settings/settingsdecimals.cpp \
           aboutdialog.cpp \
           currencypairitem.cpp \
           datafolderchusedialog.cpp \
           debugviewer.cpp \
           depthitem.cpp \
           depthmodel.cpp \
           exchange/exchange.cpp \
           exchange/exchange_bitfinex.cpp \
           exchange/exchange_bitstamp.cpp \
           exchange/exchange_btcchina.cpp \
           exchange/exchange_btce.cpp \
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
           login/newpassworddialog.cpp \
           orderitem.cpp \
           ordersmodel.cpp \
           orderstablecancelbutton.cpp \
           login/passworddialog.cpp \
           percentpicker.cpp \
           qtbitcointrader.cpp \
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
           julylockfile.cpp \
           exchange/exchange_gocio.cpp \
           login/featuredexchangesdialog.cpp \
           login/allexchangesdialog.cpp \
           login/allexchangesmodel.cpp \
           login/exchangebutton.cpp \
           exchange/exchange_indacoin.cpp \
           exchange/exchange_bitcurex.cpp \
           exchange/exchange_bitmarket.cpp \
           exchange/exchange_okcoin.cpp \
           timesync.cpp \
           translationmessage.cpp \
           indicatorengine.cpp

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
