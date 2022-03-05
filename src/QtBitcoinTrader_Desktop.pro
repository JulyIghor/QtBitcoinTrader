lessThan(QT_MAJOR_VERSION, 5) {
error("Qt 4 is no longer supported. In order to compile Qt Bitcoin Trader you need update to Qt5 http://qt.io/download-open-source/ or use pre-compiled binaries https://sourceforge.net/projects/bitcointrader/files/");
}

lessThan(QT_MAJOR_VERSION, 6) { lessThan(QT_MINOR_VERSION, 9) {
error("Qt $${QT_VERSION} is no longer supported. In order to compile Qt Bitcoin Trader you need update at least to Qt 5.9 http://qt.io/download-open-source/ or use pre-compiled binaries https://sourceforge.net/projects/bitcointrader/files/"); } }

TARGET = QtBitcoinTrader

CONFIG	+= qt c++11

TEMPLATE	= app
LANGUAGE	= C++
DEPENDPATH	+= .
INCLUDEPATH	+= .

QT += network widgets

lessThan(QT_MAJOR_VERSION, 6) {
QT += script texttospeech
} else {
QT += qml
}

unix:!macx { QT += multimedia }
macx   { QT += multimedia }

LIBS += -lssl -lcrypto -lz

        DEFINES += QTBUILDTARGETLINUX64
linux {
    contains(QMAKE_TARGET.arch, x86_64) {
        DEFINES += QTBUILDTARGETLINUX64
    }
}

win32 {
    LIBS += -lgdi32 -lws2_32 -lole32 -lwinmm

    contains(QMAKE_TARGET.arch, x86_64) {
        DEFINES += QTBUILDTARGETWIN64
    }

    DEFINES += SAPI_ENABLED

    checkFRAMEWORKDIR=$$(FRAMEWORKDIR)
    isEmpty(checkFRAMEWORKDIR) {
        LIBS += -lsapi
    }
}

macx {
    LIBS += -dead_strip
    LIBS += -framework CoreFoundation
    LIBS += -framework ApplicationServices
}

CONFIG(static) {
linux: QTPLUGIN.platforms+=qvnc qxcb
win32: QTPLUGIN.platforms=qwindows
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
HEADERS += $${PWD}/script/addrulegroup.h \
           $${PWD}/script/rulesmodel.h \
           $${PWD}/script/rulewidget.h \
           $${PWD}/script/scriptwidget.h \
           $${PWD}/script/scriptobject.h \
           $${PWD}/script/addscriptwindow.h \
           $${PWD}/script/addruledialog.h \
           $${PWD}/script/rulescriptparser.h \
           $${PWD}/script/ruleholder.h \
           $${PWD}/script/scriptobjectthread.h \
           $${PWD}/platform/sound.h \
           $${PWD}/platform/socket.h \
           $${PWD}/config/config_manager.h \
           $${PWD}/config/config_manager_dialog.h \
           $${PWD}/utils/utils.h \
           $${PWD}/settings/settingsdialog.h \
           $${PWD}/settings/settingsgeneral.h \
           $${PWD}/settings/settingsnetworkproxy.h \
           $${PWD}/settings/settingsdialoglistelement.h \
           $${PWD}/settings/settingsdecimals.h \
           $${PWD}/dock/dock_host.h \
           $${PWD}/charts/chartsview.h \
           $${PWD}/charts/chartsmodel.h \
           $${PWD}/news/newsview.h \
           $${PWD}/news/newsmodel.h \
           $${PWD}/aboutdialog.h \
           $${PWD}/currencyinfo.h \
           $${PWD}/currencypairitem.h \
           $${PWD}/datafolderchusedialog.h \
           $${PWD}/debugviewer.h \
           $${PWD}/depthitem.h \
           $${PWD}/depthmodel.h \
           $${PWD}/exchange/exchange.h \
           $${PWD}/exchange/exchange_bitfinex.h \
           $${PWD}/exchange/exchange_bitstamp.h \
           $${PWD}/exchange/exchange_indacoin.h \
           $${PWD}/exchange/exchange_yobit.h \
           $${PWD}/exchange/exchange_binance.h \
           $${PWD}/exchange/exchange_bittrex.h \
           $${PWD}/exchange/exchange_hitbtc.h \
           $${PWD}/exchange/exchange_poloniex.h \
           $${PWD}/feecalculator.h \
           $${PWD}/historyitem.h \
           $${PWD}/historymodel.h \
           $${PWD}/julyaes256.h \
           $${PWD}/julyhttp.h \
           $${PWD}/julylightchanges.h \
           $${PWD}/julyrsa.h \
           $${PWD}/julyscrolluponidle.h \
           $${PWD}/julyspinboxfix.h \
           $${PWD}/julyspinboxpicker.h \
           $${PWD}/julytranslator.h \
           $${PWD}/logthread.h \
           $${PWD}/main.h \
           $${PWD}/login/newpassworddialog.h \
           $${PWD}/orderitem.h \
           $${PWD}/ordersmodel.h \
           $${PWD}/orderstablecancelbutton.h \
           $${PWD}/login/passworddialog.h \
           $${PWD}/percentpicker.h \
           $${PWD}/qtbitcointrader.h \
           $${PWD}/thisfeatureunderdevelopment.h \
           $${PWD}/tradesitem.h \
           $${PWD}/tradesmodel.h \
           $${PWD}/translationdialog.h \
           $${PWD}/translationline.h \
           $${PWD}/updaterdialog.h \
           $${PWD}/apptheme.h \
           $${PWD}/logobutton.h \
           $${PWD}/menu/networkmenu.h \
           $${PWD}/julylockfile.h \
           $${PWD}/login/featuredexchangesdialog.h \
           $${PWD}/login/allexchangesdialog.h \
           $${PWD}/login/allexchangesmodel.h \
           $${PWD}/login/exchangebutton.h \
           $${PWD}/login/qttraderinform.h \
           $${PWD}/julymath.h \
           $${PWD}/timesync.h \
           $${PWD}/translationmessage.h \
           $${PWD}/indicatorengine.h \
           $${PWD}/menu/currencymenu.h \
           $${PWD}/menu/currencymenucell.h \
           $${PWD}/utils/currencysignloader.h \
           $${PWD}/iniengine.h \
           $${PWD}/utils/traderspinbox.h \
           $${PWD}/platform/procdestructor.h

FORMS += $${PWD}/script/addrulegroup.ui \
         $${PWD}/script/rulewidget.ui \
         $${PWD}/script/scriptwidget.ui \
         $${PWD}/script/addscriptwindow.ui \
         $${PWD}/script/addruledialog.ui \
         $${PWD}/config/config_manager_dialog.ui \
         $${PWD}/charts/chartsview.ui \
         $${PWD}/news/newsview.ui \
         $${PWD}/settings/settingsdialog.ui \
         $${PWD}/settings/settingsgeneral.ui \
         $${PWD}/settings/settingsnetworkproxy.ui \
         $${PWD}/settings/settingsdialoglistelement.ui \
         $${PWD}/settings/settingsdecimals.ui \
         $${PWD}/datafolderchusedialog.ui \
         $${PWD}/debugviewer.ui \
         $${PWD}/feecalculator.ui \
         $${PWD}/login/newpassworddialog.ui \
         $${PWD}/login/passworddialog.ui \
         $${PWD}/percentpicker.ui \
         $${PWD}/qtbitcointrader.ui \
         $${PWD}/thisfeatureunderdevelopment.ui \
         $${PWD}/translationabout.ui \
         $${PWD}/translationdialog.ui \
         $${PWD}/updaterdialog.ui \
         $${PWD}/logobutton.ui \
         $${PWD}/menu/networkmenu.ui \
         $${PWD}/login/featuredexchangesdialog.ui \
         $${PWD}/login/allexchangesdialog.ui \
         $${PWD}/login/exchangebutton.ui \
         $${PWD}/translationmessage.ui \
         $${PWD}/menu/currencymenu.ui \
         $${PWD}/menu/currencymenucell.ui

SOURCES +=$${PWD}/script/addrulegroup.cpp \
          $${PWD}/script/rulesmodel.cpp \
          $${PWD}/script/rulewidget.cpp \
          $${PWD}/script/scriptwidget.cpp \
          $${PWD}/script/scriptobject.cpp \
          $${PWD}/script/addscriptwindow.cpp \
          $${PWD}/script/addruledialog.cpp \
          $${PWD}/script/rulescriptparser.cpp \
          $${PWD}/script/ruleholder.cpp \
          $${PWD}/script/scriptobjectthread.cpp \
          $${PWD}/platform/sound.cpp \
          $${PWD}/platform/socket.cpp \
          $${PWD}/config/config_manager.cpp \
          $${PWD}/config/config_manager_dialog.cpp \
          $${PWD}/utils/utils.cpp \
          $${PWD}/dock/dock_host.cpp \
          $${PWD}/charts/chartsview.cpp \
          $${PWD}/charts/chartsmodel.cpp \
          $${PWD}/news/newsview.cpp \
          $${PWD}/news/newsmodel.cpp \
          $${PWD}/settings/settingsdialog.cpp \
          $${PWD}/settings/settingsgeneral.cpp \
          $${PWD}/settings/settingsnetworkproxy.cpp \
          $${PWD}/settings/settingsdialoglistelement.cpp \
          $${PWD}/settings/settingsdecimals.cpp \
          $${PWD}/aboutdialog.cpp \
          $${PWD}/currencypairitem.cpp \
          $${PWD}/datafolderchusedialog.cpp \
          $${PWD}/debugviewer.cpp \
          $${PWD}/depthitem.cpp \
          $${PWD}/depthmodel.cpp \
          $${PWD}/exchange/exchange.cpp \
          $${PWD}/exchange/exchange_bitfinex.cpp \
          $${PWD}/exchange/exchange_bitstamp.cpp \
          $${PWD}/exchange/exchange_indacoin.cpp \
          $${PWD}/exchange/exchange_yobit.cpp \
          $${PWD}/exchange/exchange_binance.cpp \
          $${PWD}/exchange/exchange_bittrex.cpp \
          $${PWD}/exchange/exchange_hitbtc.cpp \
          $${PWD}/exchange/exchange_poloniex.cpp \
          $${PWD}/feecalculator.cpp \
          $${PWD}/historyitem.cpp \
          $${PWD}/historymodel.cpp \
          $${PWD}/julyaes256.cpp \
          $${PWD}/julyhttp.cpp \
          $${PWD}/julylightchanges.cpp \
          $${PWD}/julyrsa.cpp \
          $${PWD}/julyscrolluponidle.cpp \
          $${PWD}/julyspinboxfix.cpp \
          $${PWD}/julyspinboxpicker.cpp \
          $${PWD}/julytranslator.cpp \
          $${PWD}/logthread.cpp \
          $${PWD}/main.cpp \
          $${PWD}/login/newpassworddialog.cpp \
          $${PWD}/orderitem.cpp \
          $${PWD}/ordersmodel.cpp \
          $${PWD}/orderstablecancelbutton.cpp \
          $${PWD}/login/passworddialog.cpp \
          $${PWD}/percentpicker.cpp \
          $${PWD}/qtbitcointrader.cpp \
          $${PWD}/thisfeatureunderdevelopment.cpp \
          $${PWD}/tradesitem.cpp \
          $${PWD}/tradesmodel.cpp \
          $${PWD}/translationdialog.cpp \
          $${PWD}/translationline.cpp \
          $${PWD}/updaterdialog.cpp \
          $${PWD}/apptheme.cpp \
          $${PWD}/logobutton.cpp \
          $${PWD}/menu/networkmenu.cpp \
          $${PWD}/julylockfile.cpp \
          $${PWD}/login/featuredexchangesdialog.cpp \
          $${PWD}/login/allexchangesdialog.cpp \
          $${PWD}/login/allexchangesmodel.cpp \
          $${PWD}/login/exchangebutton.cpp \
          $${PWD}/login/qttraderinform.cpp \
          $${PWD}/timesync.cpp \
          $${PWD}/translationmessage.cpp \
          $${PWD}/indicatorengine.cpp \
          $${PWD}/menu/currencymenu.cpp \
          $${PWD}/menu/currencymenucell.cpp \
          $${PWD}/utils/currencysignloader.cpp \
          $${PWD}/iniengine.cpp \
          $${PWD}/utils/traderspinbox.cpp \
          $${PWD}/platform/procdestructor.cpp

#
# Resources
# 
RESOURCES += $${PWD}/QtResource.qrc

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
win32:RC_FILE = $${PWD}/WinResource.rc

macx:ICON = $${PWD}/QtBitcoinTrader.icns
macx:QMAKE_INFO_PLIST = $${PWD}/QtBitcoinTrader.plist
