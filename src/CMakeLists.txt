project(QtBicoinTrader)

#-------------------------------------------------------------
# cmake version (probably works fine with 2.x)
#-------------------------------------------------------------
cmake_minimum_required(VERSION 3.0)

#-------------------------------------------------------------
# C++11 required
#-------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)

#-------------------------------------------------------------
# Instruct CMake to run Qt moc automatically when needed.
#-------------------------------------------------------------
cmake_policy(SET CMP0071 NEW)
set(CMAKE_AUTOMOC ON)

#-------------------------------------------------------------
# Find the Qt libraries needed
#-------------------------------------------------------------
find_package(Qt5 COMPONENTS Core Widgets Network Script Multimedia REQUIRED)

#-------------------------------------------------------------
# set CPP files
#-------------------------------------------------------------
set(QT_BTC_SRCS
  aboutdialog.cpp
  apptheme.cpp
  currencypairitem.cpp
  datafolderchusedialog.cpp
  debugviewer.cpp
  depthitem.cpp
  depthmodel.cpp
  feecalculator.cpp
  historyitem.cpp
  historymodel.cpp
  indicatorengine.cpp
  iniengine.cpp
  julyaes256.cpp
  julyhttp.cpp
  julylightchanges.cpp
  julylockfile.cpp
  julyrsa.cpp
  julyscrolluponidle.cpp
  julyspinboxfix.cpp
  julyspinboxpicker.cpp
  julytranslator.cpp
  logobutton.cpp
  logthread.cpp
  main.cpp
  orderitem.cpp
  ordersmodel.cpp
  orderstablecancelbutton.cpp
  percentpicker.cpp
  qtbitcointrader.cpp
  thisfeatureunderdevelopment.cpp
  timesync.cpp
  tradesitem.cpp
  tradesmodel.cpp
  translationdialog.cpp
  translationline.cpp
  translationmessage.cpp
  updaterdialog.cpp
  #
  charts/chartsmodel.cpp
  charts/chartsview.cpp
  config/config_manager.cpp
  config/config_manager_dialog.cpp
  #
  dock/dock_host.cpp
  #
  exchange/exchange_bitfinex.cpp
  exchange/exchange_bitmarket.cpp
  exchange/exchange_bitstamp.cpp
  exchange/exchange_btcchina.cpp
  exchange/exchange.cpp
  exchange/exchange_gocio.cpp
  exchange/exchange_indacoin.cpp
  exchange/exchange_okcoin.cpp
  exchange/exchange_wex.cpp
  exchange/exchange_yobit.cpp
  #
  login/allexchangesdialog.cpp
  login/allexchangesmodel.cpp
  login/exchangebutton.cpp
  login/featuredexchangesdialog.cpp
  login/newpassworddialog.cpp
  login/passworddialog.cpp
  #
  menu/currencymenucell.cpp
  menu/currencymenu.cpp
  menu/networkmenu.cpp
  #
  news/newsmodel.cpp
  news/newsview.cpp
  #
  platform/socket.cpp
  platform/sound.cpp
  #
  script/addruledialog.cpp
  script/addrulegroup.cpp
  script/addscriptwindow.cpp
  script/ruleholder.cpp
  script/rulescriptparser.cpp
  script/rulesmodel.cpp
  script/rulewidget.cpp
  script/scriptobject.cpp
  script/scriptobjectthread.cpp
  script/scriptwidget.cpp
  #
  settings/settingsdecimals.cpp
  settings/settingsdialog.cpp
  settings/settingsdialoglistelement.cpp
  settings/settingsgeneral.cpp
  settings/settingsnetworkproxy.cpp
  #
  utils/currencysignloader.cpp
  utils/utils.cpp
)

#-------------------------------------------------------------
# set UI files
#-------------------------------------------------------------
set(QT_BTC_UI
  datafolderchusedialog.ui
  debugviewer.ui
  feecalculator.ui
  logobutton.ui
  percentpicker.ui
  qtbitcointrader.ui
  thisfeatureunderdevelopment.ui
  translationabout.ui
  translationdialog.ui
  translationmessage.ui
  updaterdialog.ui
  #
  charts/chartsview.ui
  #
  config/config_manager_dialog.ui
  #
  login/exchangebutton.ui
  login/featuredexchangesdialog.ui
  login/passworddialog.ui
  login/newpassworddialog.ui
  login/allexchangesdialog.ui
  #
  menu/currencymenucell.ui
  menu/currencymenu.ui
  menu/networkmenu.ui
  #
  news/newsview.ui
  #
  script/addscriptwindow.ui
  script/scriptwidget.ui
  script/rulewidget.ui
  script/addruledialog.ui
  script/addrulegroup.ui
  #
  settings/settingsnetworkproxy.ui
  settings/settingsdecimals.ui
  settings/settingsgeneral.ui
  settings/settingsdialoglistelement.ui
  settings/settingsdialog.ui
)

# Tell Qt to wrap the UI files
qt5_wrap_ui(QT_UI_SRCS ${QT_BTC_UI})

#-------------------------------------------------------------
# set Resource files
#-------------------------------------------------------------
qt5_add_resources(QT_BTC_RESOURCE_FILES 
  QtResource.qrc
)

#-------------------------------------------------------------
# Find includes in corresponding src/build directories
#-------------------------------------------------------------
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#-------------------------------------------------------------
# setup Qt5 include directories
#-------------------------------------------------------------
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS} 
    ${Qt5Script_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Multimedia_INCLUDE_DIRS}
)

#-------------------------------------------------------------
# Declare executable
#-------------------------------------------------------------
add_executable(QtBitcoinTrader
    ${QT_BTC_SRCS}
    ${QT_UI_SRCS}
    ${QT_BTC_RESOURCE_FILES}
)

#-------------------------------------------------------------
# Setup link libraries
#-------------------------------------------------------------
# Qt5 dependencies
target_link_libraries(QtBitcoinTrader Qt5::Widgets Qt5::Script Qt5::Network Qt5::Multimedia)

# Other libs needed
target_link_libraries(QtBitcoinTrader crypto ssl z)

# windows
# -lws2_32 -lole32 -lwinmm -lgdi32 -lcrypt32

