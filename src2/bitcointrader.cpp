//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include <QMessageBox>
#include <QHttp>
#include <QSettings>
#include "main.h"
#include "julylightchanges.h"
#include "addrulewindow.h"
#include <QClipboard>
#include <QProcess>
#include "tempwindow.h"
#include "feecalculator.h"
#include <QFile>
#include <QSysInfo>

#ifdef Q_OS_WIN
#include "qtwin.h"
#endif

BitcoinTrader::BitcoinTrader()
	: QDialog()
{
	ordersLogLoaded=false;
	appDir=QApplication::applicationDirPath()+"/";
#ifdef Q_OS_WIN
	QFile::remove(appDir+"BitcoinTrader.exe.bkp");
#endif
	authErrorOnce=false;
	showingMessage=false;
	lastLagState=false;
	floatFee=0.0;
	floatFeeDec=0.0;
	floatFeeInc=0.0;
	confirmBuySell=true;
	apiDownState=true;
	firstPriceLoad=true;
	sellLockBtcToSell=false;
	sellLockPricePerCoin=false;
	sellLockBtcPerOrder=false;
	sellLockAmountToReceive=false;

	buyLockTotalBtc=false;
	buyLockPricePerCoin=false;
	buyLockBtcPerOrder=false;
	buyLockTotalSpend=false;


	ui.setupUi(this);
	accountFeeChanged(ui.accountFee->value());
	setWindowTitle(windowTitle()+QString::number(appVerReal,'f',2));
	setAttribute(Qt::WA_QuitOnClose,true);

	setWindowFlags(Qt::Window);

#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())
	{
		QtWin::extendFrameIntoClientArea(this);
		setStyleSheet("QGroupBox {background: rgba(255,255,255,160); border: 1px solid gray;border-radius: 3px;margin-top: 7px;} QGroupBox:title {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent); border-radius: 2px; padding: 1 4px; top: -7; left: 7px;}");
	}
#endif

	setApiDown(false);

	btcChar=ui.btcLabel->text();

	ui.ordersTableFrame->setVisible(false);

	ui.ordersTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(2,QHeaderView::Stretch);
	ui.ordersTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(5,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(6,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->hideSection(6);

	ui.rulesNoMessage->setVisible(true);
	ui.rulesTable->setVisible(false);

	ui.rulesTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(4,QHeaderView::Stretch);

	QSettings settings(iniFileName,QSettings::IniFormat);
	ui.restSignLine->setText(restSign);
	ui.restKeyLine->setText(restKey);

#ifdef Q_OS_WIN
	useSSL=settings.value("OpenSSL",bool(QSysInfo::windowsVersion()>=0x0080)).toBool();
#else
	useSSL=settings.value("OpenSSL",true).toBool();
#endif

	settings.setValue("OpenSSL",useSSL);
	ui.sslCheck->setChecked(useSSL);

	ui.accountBTCBeep->setChecked(settings.value("AccountBTCBeep",false).toBool());
	ui.accountUSDBeep->setChecked(settings.value("AccountUSDBeep",false).toBool());
	ui.marketHighBeep->setChecked(settings.value("MarketHighBeep",false).toBool());
	ui.marketLowBeep->setChecked(settings.value("MmarketLowBeep",false).toBool());

	socketThreadAuth=new SocketThread(1);
	connect(socketThreadAuth,SIGNAL(dataReceived(QByteArray)),this,SLOT(dataReceivedAuth(QByteArray)));
	connect(socketThreadAuth,SIGNAL(apiDown()),this,SLOT(apiDownSlot()));

	ordersSelectionChanged();
	connect(ui.sslCheck,SIGNAL(toggled(bool)),this,SLOT(setSslEnabled(bool)));

	new JulyLightChanges(ui.marketVolume);
	new JulyLightChanges(ui.marketBuy);
	new JulyLightChanges(ui.marketSell);
	new JulyLightChanges(ui.marketBuy);
	new JulyLightChanges(ui.marketHigh);
	new JulyLightChanges(ui.marketLow);
	new JulyLightChanges(ui.accountBTC);
	new JulyLightChanges(ui.accountUSD);
	new JulyLightChanges(ui.marketLast);

	secondTimer=new QTimer;
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondTimer->setSingleShot(true);
	secondTimer->start(1000);
	resize(1280,800);

	httpUpdate=new QHttp("trader.uax.co",80,this);
	connect(httpUpdate,SIGNAL(done(bool)),this,SLOT(httpUpdateDone(bool)));
	updateCheckTimer=new QTimer(this);
	connect(updateCheckTimer,SIGNAL(timeout()),this,SLOT(checkUpdate()));
	checkUpdate();
	updateCheckTimer->start(3600000);
}

BitcoinTrader::~BitcoinTrader()
{
}

void BitcoinTrader::setApiDown(bool on)
{
	if(apiDownState==on)return;
	if(on)
	{
		ui.lagValue->setVisible(false);
		ui.apiDownLabel->setVisible(true);
	}
	else
	{
		ui.apiDownLabel->setVisible(false);
		ui.lagValue->setVisible(true);
	}
	apiDownState=on;
}

void BitcoinTrader::apiDownSlot()
{
	setApiDown(true);
}

void BitcoinTrader::setSslEnabled(bool on)
{
	useSSL=on;
	socketThreadAuth->reconnectApi();
	QSettings settings(iniFileName,QSettings::IniFormat);
	settings.setValue("OpenSSL",on);
}

void BitcoinTrader::checkUpdate()
{
	httpUpdate->get("/API.php?Thread=Call&Object=General&Method=CheckUpdate");
}

QString BitcoinTrader::clearData(QString data)
{
	while(data.count()&&(data.at(0)=='{'||data.at(0)=='['||data.at(0)=='\"'))data.remove(0,1);
	while(data.count()&&(data.at(data.length()-1)=='}'||data.at(data.length()-1)==']'||data.at(data.length()-1)=='\"'))data.remove(data.length()-1,1);
	return data;
}

void BitcoinTrader::httpUpdateDone(bool on)
{
	if(on)return;
	QString allData=httpUpdate->readAll();
	if(allData.length()<3)return;
	QString errorString;
	QString versionString;
	QStringList allDataList=allData.split(",");
	for(int n=0;n<allDataList.count();n++)
	{
		QStringList pairList=allDataList.at(n).split(":");
		if(pairList.count()==2)
		{
			QString name=clearData(pairList.first());
			QString value=clearData(pairList.last());
			if(name=="Error")errorString=value;
			if(name=="Version")versionString=value;
		}
	}
	if(errorString.isEmpty())
	{
		if(versionString.toDouble()>appVerReal)
		{
#ifdef QT_OS_WIN
			QProcess proc;
			proc.startDetached(QApplication::applicationFilePath()+" /checkupdate");
			proc.waitForStarted();
#else
			QMessageBox::information(this,windowTitle(),"Update released "+versionString+". Please download from https://sourceforge.net/projects/bitcointrader/");
#endif
		}
	}
}

void BitcoinTrader::saveSoundToggles()
{
	QSettings settings(iniFileName,QSettings::IniFormat);
	settings.setValue("AccountBTCBeep",ui.accountBTCBeep->isChecked());
	settings.setValue("AccountUSDBeep",ui.accountUSDBeep->isChecked());
	settings.setValue("MarketHighBeep",ui.marketHighBeep->isChecked());
	settings.setValue("MmarketLowBeep",ui.marketLowBeep->isChecked());
}

void BitcoinTrader::accountFeeChanged(double val)
{
	floatFee=val/100;
	floatFeeDec=1.0-floatFee;
	floatFeeInc=1.0+floatFee;
}

QByteArray BitcoinTrader::getMidData(QString a, QString b,QByteArray *data)
{
	QByteArray rez;
	if(b.isEmpty())b="\",";
	int startPos=data->indexOf(a,0);
	if(startPos>-1)
	{
	int endPos=data->indexOf(b,startPos+a.length());
	if(endPos>-1)rez=data->mid(startPos+a.length(),endPos-startPos-a.length());
	}
	return rez;
}


void BitcoinTrader::secondSlot()
{
	ui.lastUpdate->setValue((double)lastUpdate.msecsTo(QDateTime::currentDateTime())/1000);
	if(!ordersLogLoaded)updateLogTable();
	secondTimer->start(1000);
}

void BitcoinTrader::mtgoxLagChanged(double val)
{
	if(val>=1.0)
		ui.lagValue->setStyleSheet("background: #ffaaaa");
	else
		ui.lagValue->setStyleSheet("");
}

void BitcoinTrader::balanceChanged(double)
{
	updateLogTable();
}

bool BitcoinTrader::isValidKey()
{
	return !ui.restKeyLine->text().isEmpty()&&!ui.restSignLine->text().isEmpty();
}

void BitcoinTrader::dataReceivedAuth(QByteArray data)
{
	if(isLogEnabled)logThread->writeLog("Received: "+data);
	setApiDown(false);
	if(getMidData("{\"result\":\"","\",",&data)=="success")
	{
		QDateTime newDate=QDateTime::currentDateTime();
		ui.lastUpdate->setValue((double)lastUpdate.msecsTo(newDate)/1000);
		lastUpdate=newDate;

		if(data.size()>27)data.remove(0,28);
		if(data.size()&&data.at(data.size()-1)=='\"')data.remove(data.size()-1,1);

		if(data.startsWith("\"Login"))
		{
			QByteArray login=getMidData("Login\":\"","\",",&data);
			if(!login.isEmpty())ui.accountLoginLabel->setText(login);

			QByteArray btcBalance=getMidData("BTC\":{\"Balance\":{\"value\":\"","",&data);
			if(!btcBalance.isEmpty())ui.accountBTC->setValue(btcBalance.toDouble());

			QByteArray usdBalance=getMidData("USD\":{\"Balance\":{\"value\":\"","",&data);
			if(!usdBalance.isEmpty())ui.accountUSD->setValue(usdBalance.toDouble());

			QByteArray monthValue=getMidData("Monthly_Volume\":{\"value\":\"","\",",&data);
			if(!monthValue.isEmpty())ui.accountVolume->setValue(monthValue.toDouble());

			QByteArray tradeFee=getMidData("Trade_Fee\":","}",&data);
			if(!tradeFee.isEmpty())ui.accountFee->setValue(tradeFee.toDouble());
		}
		else
		if(data.startsWith("\"high"))
		{
			ui.marketHigh->setValue(getMidData("high\":{\"value\":\"","",&data).toDouble());
			ui.marketLow->setValue(getMidData("low\":{\"value\":\"","",&data).toDouble());
			ui.marketSell->setValue(getMidData("sell\":{\"value\":\"","",&data).toDouble());
			ui.marketLast->setValue(getMidData("last\":{\"value\":\"","",&data).toDouble());
			ui.marketBuy->setValue(getMidData("buy\":{\"value\":\"","",&data).toDouble());
			ui.marketVolume->setValue(getMidData("vol\":{\"value\":\"","",&data).toDouble());

			if(firstPriceLoad)
			{
				firstPriceLoad=false;
				sellTotalBtcToSellAllIn();
				sellPricePerCoinAsMarketPrice();

				buyBtcToBuyAllIn();
				buyPricePerCoinAsMarketPrice();
				
				sellBuyAsMarketPrice();

				updateLogTable();
			}
		}
		else
		if(data.startsWith("\"lag"))
		{
			ui.lagValue->setValue(getMidData("lag_secs\":",",\"",&data).toDouble());
		}
		else
		if(data=="]}")//Orders is empty
		{
			if(ui.ordersTable->rowCount())
			{
				if(isLogEnabled)logThread->writeLog("Order table cleared");
				oidMap.clear();
				ui.ordersTable->clearContents();
				ui.ordersTable->setRowCount(0);
				ui.ordersTableFrame->setVisible(false);
				ui.noOpenedOrdersLabel->setVisible(true);
				ui.noOpenedOrdersLabel->setVisible(true);
			}
		}
		else
		if(data.startsWith("{\"oid\":"))//Have orders
		{
			QByteArray currentOrder=getMidData("oid","\"actions\":",&data);
			QMap<QByteArray,bool> activeOrders;
			while(currentOrder.size())
			{
				QByteArray oid=getMidData("\":\"","\",\"",&currentOrder);
				QByteArray itemType=getMidData("\"type\":\"","\",\"",&currentOrder);
				QByteArray itemAmount=getMidData("\"amount\":{\"value\":\"","\",\"",&currentOrder);
				QByteArray itemPrice=getMidData("\"price\":{\"value\":\"","\",\"",&currentOrder);
				QByteArray itemStatus=getMidData("\"status\":\"","\",\"",&currentOrder);
				QByteArray itemDate=getMidData(",\"date\":",",",&currentOrder);

				QString oidData=itemDate+";"+itemType+";"+itemStatus+";"+itemAmount+";"+itemPrice;
				QString findedData=oidMap.value(oid,"");

				if(findedData!="")//Update
				{
					if(findedData!=oidData)
					{
						for(int n=0;n<ui.ordersTable->rowCount();n++)
						{
							if(ui.ordersTable->item(n,6)->text().toAscii()==oid)
							{
								if(ui.ordersTable->item(n,2)->text()!="canceled")
								{
									ui.ordersTable->item(n,2)->setText(itemStatus);
									ui.ordersTable->item(n,3)->setText(itemAmount);
									ui.ordersTable->item(n,5)->setText("$ "+QString::number(itemAmount.toDouble()*itemPrice.toDouble(),'f',5));
									setRowStateByText(n,itemStatus);
									oidMap[oid]=oidData;
								}
								break;
							}
						}
					}
				}
				else//Insert
				{
					insertIntoTable(oid,oidData);
					oidMap[oid]=oidData;
				}
				activeOrders[oid]=1;
				if(data.size()>currentOrder.size())data.remove(0,currentOrder.size());
				currentOrder=getMidData("oid","\"actions\"",&data);
			}

			for(int n=0;n<ui.ordersTable->rowCount();n++)
			{
				if(n<0)break;
				QByteArray curOid=ui.ordersTable->item(n,6)->text().toAscii();
				if(activeOrders.values(curOid).count()==0)
				{
					if(isLogEnabled)logThread->writeLog("Deleted: "+curOid);
					oidMap.remove(curOid);
					ui.ordersTable->removeRow(n--);

					ui.ordersTableFrame->setVisible(ui.ordersTable->rowCount());
					ui.noOpenedOrdersLabel->setVisible(!ui.ordersTable->isVisible());
				}
			}
		}
		else
		if(data.startsWith("\"oid\":"))
		{
			if(isLogEnabled)logThread->writeLog("Removed: "+data);

			QByteArray oid=getMidData("oid\":\"","\",\"",&data);
			if(!oid.isEmpty())
			for(int n=0;n<ui.ordersTable->rowCount();n++)
			{
				if(ui.ordersTable->item(n,6)->text().toAscii()==oid)
				{
					ui.ordersTable->item(n,2)->setText("canceled");
					setRowState(n,0);
					break;
				}
			}
		}
		else
		if(data.startsWith("\"oid\""))
		{
			if(isLogEnabled)logThread->writeLog("Canceled: "+data);
		}
		else
		if(data.right(2)=="\"}"&&data.size()>36&&data.size()<40)
		{
			if(isLogEnabled)logThread->writeLog("Added: "+data);
		}
		else
		if(data.startsWith("\"records\""))
		{
			ordersLogLoaded=true;
			QString newLog;
			QStringList dataList=QString(data).split("\"Index\"");
			for(int n=0;n<dataList.count();n++)
			{
				QByteArray curLog(dataList.at(n).toAscii());
				QByteArray logType=getMidData("\"Type\":\"","\",\"",&curLog);
				int logTypeInt=0;
				if(logType=="out"){logTypeInt=1;logType="<font color=\"red\">(Sold)</font>";}
				else
				if(logType=="in"){logTypeInt=2;logType="<font color=\"blue\">(Bought)</font>";}
				else 
				if(logType=="fee"){logTypeInt=3;logType.clear();}
				
				if(logTypeInt)
				{
					QByteArray logValue=getMidData("\"Value\":{\"value\":\"","\",\"",&curLog);
					/*int logValueDot=-1;
					if(logValueDot+9<logValue.size())logValueDot=logValue.indexOf('.');
					if(logValueDot==-1){logValue="Mt.Gox BUG";ordersLogLoaded=false;}
					else logValue=logValue.left(logValueDot+9);*/
					//QByteArray logBalance=getMidData("\"Balance\":{\"value\":\","\",\"",&curLog);
					QByteArray logDate=getMidData("\"Date\":",",\"",&curLog);
					QByteArray logText=getMidData(" at ","\",\"",&curLog);
					curLog=getMidData("\"Info\":\"","\",\"",&curLog);
	
			newLog.append("<font color=\"gray\">"+QDateTime::fromTime_t(logDate.toUInt()).toString("yyyy-MM-dd HH:mm:ss")+"</font>  <font color=\"#996515\">"+btcChar+logValue+"</font> at <font color=\"darkgreen\">"+logText+"</font> "+logType+"<br>");
				}
			}
			ui.logTextEdit->setHtml(newLog);
			if(isLogEnabled)logThread->writeLog("Log Table Updated");
		}
		else
		{
			if(data.size()>0)QMessageBox::warning(0,"Unknown Data",data);
			if(isLogEnabled)logThread->writeLog("Unknown Data: "+data);
		}
	}
	else
	{
		//delete socketThreadQuery;
		QString errorString=getMidData("error\":\"","\"",&data);
		
		if(errorString.isEmpty())return;

		if(errorString=="Order not found")
		{
			//QMessageBox::warning(0,"Mt.Gox Error",errorString);
		}
		else
		if(errorString.startsWith("Identification required"))
		{
			if(!showingMessage)
			{
				if(!showingMessage&&!authErrorOnce)
				{
				authErrorOnce=true;
				showingMessage=true;
				QMessageBox::warning(this,"Mt.Gox Error","Identification required to access private API.\nPlease enter valid API key and Secret.\n\nTo get new API Keys:\nGo http://mtgox.com => \"Security Center\"  => \"Advanced API Key Creation\"\nCheck \"Info\" and \"Trade\"\nPress Create Key\nCopy and paste RestKey and RestSign to program.");
				if(isLogEnabled)logThread->writeLog("Mt.Gox Error: Identification required to access private API\nPlease enter valid API key and Secret");
				showingMessage=false;
				}
			}
		}
		else
		if(errorString.startsWith("Invalid request"))
		{
			if(isLogEnabled)logThread->writeLog("Mt.Gox Error: Invalid request method for this API");
		}
		else
		if(errorString.isEmpty())
		{
			if(!showingMessage)
			{
			showingMessage=true;
			QMessageBox::warning(this,"Mt.Gox API",data+"\n\nPlease Restart Application");
			showingMessage=false;
			}

			if(isLogEnabled)logThread->writeLog("Mt.Gox Error: "+data);
		}
		else
		{
			if(!showingMessage)
			{
				showingMessage=true;
				QMessageBox::warning(this,"Mt.Gox API",errorString);
				showingMessage=false;
			}

		if(isLogEnabled)logThread->writeLog("Mt.Gox Error: "+errorString.toAscii());
		}
	}
}

void BitcoinTrader::insertIntoTable(QByteArray oid, QString data)
{
	QStringList dataList=data.split(";");
	if(dataList.count()!=5)return;
	int curRow=ui.ordersTable->rowCount();
	ui.ordersTable->setRowCount(curRow+1);
	ui.ordersTable->setRowHeight(curRow,30);
	ui.ordersTable->setItem(curRow,0,new QTableWidgetItem(QDateTime::fromTime_t(dataList.at(0).toUInt()).toString("yyyy-MM-dd HH:mm:ss")));ui.ordersTable->item(curRow,0)->setTextAlignment(Qt::AlignCenter);
	ui.ordersTable->setItem(curRow,1,new QTableWidgetItem(dataList.at(1)));ui.ordersTable->item(curRow,1)->setTextAlignment(Qt::AlignCenter);
	ui.ordersTable->setItem(curRow,2,new QTableWidgetItem(dataList.at(2)));ui.ordersTable->item(curRow,2)->setTextAlignment(Qt::AlignCenter);
	ui.ordersTable->setItem(curRow,3,new QTableWidgetItem(btcChar+" "+dataList.at(3)));ui.ordersTable->item(curRow,3)->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
	ui.ordersTable->setItem(curRow,4,new QTableWidgetItem("$ "+dataList.at(4)));ui.ordersTable->item(curRow,4)->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
	ui.ordersTable->setItem(curRow,5,new QTableWidgetItem("$ "+QString::number(dataList.at(3).toDouble()*dataList.at(4).toDouble(),'f',5)));ui.ordersTable->item(curRow,5)->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
	ui.ordersTable->setItem(curRow,6,new QTableWidgetItem(QString(oid)));ui.ordersTable->item(curRow,6)->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
	setRowStateByText(curRow,dataList.at(2).toAscii());
	ordersSelectionChanged();
	ui.ordersTableFrame->setVisible(true);
	ui.noOpenedOrdersLabel->setVisible(false);
}

void BitcoinTrader::setRowStateByText(int row, QByteArray text)
{
	if(text=="invalid")setRowState(row,4);
	else 
	if(text.contains("pending"))setRowState(row,1);
	else
	setRowState(row,2);
}

void BitcoinTrader::setRowState(int row, int state)
{
	QColor nColor(255,255,255);
	switch(state)
	{
		case 0: nColor=Qt::lightGray; break;
		case 1: nColor.setRgb(255,255,200); break;
		case 2: nColor.setRgb(200,255,200); break;
		case 3: nColor.setRgb(255,200,200); break;
		case 4: nColor.setRgb(200,200,255); break;
		default: break;
	}
	for(int n=0;n<6;n++)ui.ordersTable->item(row,n)->setBackgroundColor(nColor);
}

void BitcoinTrader::restKeyChanged(QString key)
{
	QSettings settings(iniFileName,QSettings::IniFormat);
	settings.setValue("RestKey",key);
	restKey=key.toAscii();
	validKeySign=isValidKey();
}

void BitcoinTrader::restSignChanged(QString key)
{
	QSettings settings(iniFileName,QSettings::IniFormat);
	settings.setValue("RestSign",key);
	restSign=QByteArray::fromBase64(key.toAscii());
	validKeySign=isValidKey();
}

void BitcoinTrader::ordersCancelAll()
{
	for(int n=0;n<ui.ordersTable->rowCount();n++)
		if(ui.ordersTable->item(n,2)->text()!="canceled")
			cancelOrderByOid(ui.ordersTable->item(n,6)->text().toAscii());
}

void BitcoinTrader::cancelOrderByOid(QByteArray oid)
{
	socketThreadAuth->sendToApi("BTCUSD/money/order/cancel","&oid="+oid);
	if(isLogEnabled)logThread->writeLog("BTCUSD/money/order/cancel?nonce=*&oid="+oid);
}

void BitcoinTrader::ordersCancelSelected()
{
	int curRow=ui.ordersTable->currentRow();
	if(curRow==-1||curRow>=ui.ordersTable->rowCount())return;
	QString oidToCancel=ui.ordersTable->item(curRow,6)->text();
	if(oidToCancel.length()>30)
		cancelOrderByOid(oidToCancel.toAscii());
}

void BitcoinTrader::calcButtonClicked()
{
	(new FeeCalculator)->show();
}

void BitcoinTrader::checkValidSellButtons()
{
	ui.sellBitcoinsButton->setEnabled(ui.sellBtcPerOrder->value()>=0.01&&ui.sellTotalBtc->value()>=0.01);
	ui.sellBuyButtonSellBuy->setEnabled(ui.sellBitcoinsButton->isEnabled()&&ui.buyBitcoinsButton->isEnabled());
}

void BitcoinTrader::sellPricePerCoinAsMarketPrice()
{
	ui.sellPricePerCoin->setValue(ui.marketBuy->value());
}

void BitcoinTrader::sellTotalBtcToSellAllIn()
{
	ui.sellTotalBtc->setValue(ui.accountBTC->value());
}

void BitcoinTrader::sellTotalBtcToSellHalfIn()
{
	ui.sellTotalBtc->setValue(ui.accountBTC->value()/2);
}

void BitcoinTrader::lastSoftLagChanged(double val)
{
	bool lastL=lastLagState;
	if(val>=2.0)
	{
		lastLagState=false;
		ui.lastUpdate->setStyleSheet("background: #ffaaaa");
	}
	else
	{
		ui.lastUpdate->setStyleSheet("");
		lastLagState=true;
	}
	if(lastL!=lastLagState)
	{
		checkValidSellButtons();
		checkValidBuyButtons();
		ui.ordersControls->setEnabled(lastLagState);
		ui.buyButtonBack->setEnabled(lastLagState);
		ui.sellButtonBack->setEnabled(lastLagState);
		ui.buySellButtonBack->setEnabled(lastLagState);
	}
}


void BitcoinTrader::sellTotalBtcToSellChanged(double val)
{
	if(sellLockBtcToSell)return;
	sellLockBtcToSell=true;

	sellLockAmountToReceive=true;
	ui.sellAmountToReceive->setValue(ui.sellPricePerCoin->value()*ui.sellTotalBtc->value()*floatFeeDec);

	sellLockAmountToReceive=false;

	sellLockBtcPerOrder=true;
	ui.sellBtcPerOrder->setValue(ui.sellTotalBtc->value()/ui.sellOrdersCount->value());
	sellLockBtcPerOrder=false;

	checkValidSellButtons();
	sellLockBtcToSell=false;
}

void BitcoinTrader::sellPricePerCoinInUsdChanged(double)
{
	if(!sellLockPricePerCoin)
	{
	sellLockPricePerCoin=true;
	sellTotalBtcToSellChanged(ui.sellTotalBtc->value());
	sellLockPricePerCoin=false;
	}
	ui.sellNextMaxBuyPrice->setValue(ui.sellPricePerCoin->value()*floatFeeDec*floatFeeDec);
	ui.sellNextMaxBuyStep->setValue(ui.sellPricePerCoin->value()-ui.sellNextMaxBuyPrice->value());
	checkValidSellButtons();
}

void BitcoinTrader::sellBtcPerOrderChanged(double val)
{
	if(sellLockBtcPerOrder)return;
	sellLockBtcPerOrder=true;
	ui.sellTotalBtc->setValue(val/ui.sellOrdersCount->value());
	sellLockBtcPerOrder=false;
	checkValidSellButtons();
}

void BitcoinTrader::sellOrdersCountChanged(int val)
{
	if(val<1)return;
	sellLockBtcPerOrder=true;
	ui.sellBtcPerOrder->setValue(ui.sellTotalBtc->value()/val);
	checkValidSellButtons();
	sellLockBtcPerOrder=false;
}

void BitcoinTrader::sellOrdersCountToDefault()
{
	ui.sellOrdersCount->setValue(1);
}

void BitcoinTrader::sellAmountToReceiveChanged(double val)
{
	if(sellLockAmountToReceive)return;
	sellLockAmountToReceive=true;

	sellLockBtcToSell=true;
	sellLockPricePerCoin=true;

	ui.sellTotalBtc->setValue(val/ui.sellPricePerCoin->value()/floatFeeDec);

	sellLockPricePerCoin=false;
	sellLockBtcToSell=false;

	sellLockBtcPerOrder=true;
	ui.sellBtcPerOrder->setValue(ui.sellTotalBtc->value()/ui.sellOrdersCount->value());
	sellLockBtcPerOrder=false;

	sellLockAmountToReceive=false;
	checkValidSellButtons();
}

void BitcoinTrader::sellBitcoinButton()
{
	checkValidSellButtons();
	if(ui.sellBitcoinsButton->isEnabled()==false)return;

	if(confirmBuySell)
	{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("Please confirm transaction");
	msgBox.setText("Are you sure to sell "+btcChar+" "+ui.sellTotalBtc->text()+" for $ "+ui.sellPricePerCoin->text()+" ?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	if(msgBox.exec()!=QMessageBox::Yes)return;
	}
	for(int n=0;n<ui.sellOrdersCount->value();n++)
	{
		QByteArray params="&type=ask&amount_int="+QByteArray::number(ui.sellBtcPerOrder->value()*100000000,'f',0)+"&price_int="+QByteArray::number(ui.sellPricePerCoin->value()*100000,'f',0);
		socketThreadAuth->sendToApi("BTCUSD/money/order/add",params);
		if(isLogEnabled)logThread->writeLog("BTCUSD/money/order/add?nonce=*"+params);
	}
}


void BitcoinTrader::buyTotalToSpendInUsdChanged(double val)
{
	if(buyLockTotalSpend)return;
	buyLockTotalSpend=true;

	buyLockTotalBtc=true;
	ui.buyTotalBtc->setValue(ui.buyTotalSpend->value()/ui.buyPricePerCoin->value());
	buyLockTotalBtc=false;

	buyLockBtcPerOrder=true;
	ui.buyBtcPerOrder->setValue(ui.buyTotalBtc->value()/ui.buyOrdersCount->value());
	buyLockBtcPerOrder=false;

	buyLockTotalSpend=false;
	checkValidBuyButtons();
}


void BitcoinTrader::buyBtcToBuyChanged(double val)
{
	ui.buyTotalBtcResult->setValue(val*floatFeeDec);
	if(buyLockTotalBtc)return;
	buyLockTotalBtc=true;

	buyLockTotalSpend=true;
	ui.buyTotalSpend->setValue(ui.buyTotalBtc->value()*ui.buyPricePerCoin->value());
	buyLockTotalSpend=false;

	buyLockBtcPerOrder=true;
	ui.buyBtcPerOrder->setValue(ui.buyTotalBtc->value()/ui.buyOrdersCount->value());
	buyLockBtcPerOrder=false;

	buyLockTotalBtc=false;
	checkValidBuyButtons();
}

void BitcoinTrader::buyPricePerCoinChanged(double)
{
	if(!buyLockPricePerCoin)
	{
	buyLockPricePerCoin=true;
	buyTotalToSpendInUsdChanged(ui.buyTotalSpend->value());
	buyLockPricePerCoin=false;
	}
	ui.buyNextInSellPrice->setValue(ui.buyPricePerCoin->value()*floatFeeInc*floatFeeInc);
	ui.buyNextMinBuyStep->setValue(ui.buyNextInSellPrice->value()-ui.buyPricePerCoin->value());
	checkValidBuyButtons();
}

void BitcoinTrader::checkValidBuyButtons()
{
	ui.buyBitcoinsButton->setEnabled(ui.buyTotalBtc->value()>=0.01&&ui.buyBtcPerOrder->value()>=0.01);
	ui.sellBuyButtonSellBuy->setEnabled(ui.sellBitcoinsButton->isEnabled()&&ui.buyBitcoinsButton->isEnabled());
}

void BitcoinTrader::buyOrdersCountChanged(int val)
{
	buyLockBtcPerOrder=true;
	ui.buyBtcPerOrder->setValue(ui.buyTotalBtc->value()/val);
	buyLockBtcPerOrder=false;
}

void BitcoinTrader::buyOrdersCountToDefault()
{
	ui.buyOrdersCount->setValue(1);
}

void BitcoinTrader::buyBtcPerOrderChanged(double val)
{
	if(buyLockBtcPerOrder)return;
	buyLockBtcPerOrder=true;
	ui.buyTotalBtc->setValue(ui.buyOrdersCount->value()*val);
	buyLockBtcPerOrder=false;
	checkValidBuyButtons();
}

void BitcoinTrader::buyBtcToBuyAllIn()
{
	ui.buyTotalSpend->setValue(ui.accountUSD->value());
}

void BitcoinTrader::buyBtcToBuyHalfIn()
{
	ui.buyTotalSpend->setValue(ui.accountUSD->value()/2);
}

void BitcoinTrader::buyPricePerCoinAsMarketPrice()
{
	ui.buyPricePerCoin->setValue(ui.marketSell->value());
}

void BitcoinTrader::ordersSelectionChanged()
{
	ui.ordersCancelAllButton->setEnabled(ui.ordersTable->rowCount());
	ui.ordersSelectNone->setEnabled(ui.ordersTable->selectedItems().count());
	ui.ordersCancelSelected->setEnabled(ui.ordersSelectNone->isEnabled());
}

void BitcoinTrader::closeEvent(QCloseEvent *event)
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(windowTitle());
	msgBox.setText("Are you sure to close Application?");//\nActive rules works only while application is running.");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	if(msgBox.exec()!=QMessageBox::Yes){event->ignore();return;}
	event->accept();
}

void BitcoinTrader::buyBitcoinsButton()
{
	checkValidBuyButtons();
	if(ui.buyBitcoinsButton->isEnabled()==false)return;
	if(confirmBuySell)
	{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("Please confirm transaction");
	msgBox.setText("Are you sure to buy "+btcChar+" "+ui.buyTotalBtc->text()+" for $ "+ui.buyPricePerCoin->text()+" ?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	if(msgBox.exec()!=QMessageBox::Yes)return;
	}

	for(int n=0;n<ui.buyOrdersCount->value();n++)
	{
		QByteArray params="&type=bid&amount_int="+QByteArray::number(ui.buyBtcPerOrder->value()*100000000,'f',0)+"&price_int="+QByteArray::number(ui.buyPricePerCoin->value()*100000,'f',0);
		socketThreadAuth->sendToApi("BTCUSD/money/order/add",params);
		if(isLogEnabled)logThread->writeLog("BTCUSD/money/order/add?nonce=*"+params);
	}
}

void BitcoinTrader::sellBuyAsMarketPrice()
{
	ui.sellBuyMidPrice->setValue(ui.marketLast->value());
}

void BitcoinTrader::updateLogTable()
{
	if(updateLogTime.elapsed()<1000)return;
	socketThreadAuth->sendToApi("money/wallet/history","&currency=BTC");
	if(isLogEnabled)logThread->writeLog("money/wallet/history?nonce=*&currency=BTC");
	updateLogTime.restart();
}

void BitcoinTrader::sellBuyApply()
{
	double delta2=ui.sellBuyDelta->value()/2;
	buyLockPricePerCoin=true;
	buyLockBtcPerOrder=true;
	ui.buyPricePerCoin->setValue(ui.sellBuyMidPrice->value()-delta2);
	ui.buyOrdersCount->setValue(ui.sellBuyOrdersCount->value()/2);
	buyLockBtcPerOrder=false;
	buyLockPricePerCoin=false;
	ui.buyBtcPerOrder->setValue(ui.sellBuyBtcPerOrder->value());

	sellLockPricePerCoin=true;
	sellLockBtcPerOrder=true;
	ui.sellPricePerCoin->setValue(ui.sellBuyMidPrice->value()+delta2);
	ui.sellOrdersCount->setValue(ui.sellBuyOrdersCount->value()-ui.buyOrdersCount->value());
	sellLockPricePerCoin=false;
	sellLockBtcPerOrder=false;
	ui.sellTotalBtc->setValue(ui.sellBuyBtcPerOrder->value()*ui.sellOrdersCount->value());

}

void BitcoinTrader::sellBuyButtonSellBuy()
{
	sellBuyApply();

	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("Please confirm transaction");
	msgBox.setText("Are you sure to buy "+btcChar+" "+ui.buyTotalBtc->text()+" for $ "+ui.buyPricePerCoin->text()+" ?\nAre you sure to sell "+btcChar+" "+ui.sellTotalBtc->text()+" for $ "+ui.sellPricePerCoin->text()+" ?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	if(msgBox.exec()!=QMessageBox::Yes)return;

	confirmBuySell=false;
	buyBitcoinsButton();
	sellBitcoinButton();
	confirmBuySell=true;
}

void BitcoinTrader::sellBuyMidPriceChanged(double val)
{
	ui.sellBuyDelta->setMinimum(val*floatFeeInc*floatFeeInc-val+0.05);
}

void BitcoinTrader::sellBuyDelta01()
{
	ui.sellBuyDelta->setValue(ui.sellBuyMidPrice->value()*floatFeeInc*floatFeeInc-ui.sellBuyMidPrice->value()+0.15);
}

void BitcoinTrader::sellBuyDelta02()
{
	ui.sellBuyDelta->setValue(ui.sellBuyMidPrice->value()*floatFeeInc*floatFeeInc-ui.sellBuyMidPrice->value()+0.25);
}

void BitcoinTrader::sellBuyDelta05()
{
	ui.sellBuyDelta->setValue(ui.sellBuyMidPrice->value()*floatFeeInc*floatFeeInc-ui.sellBuyMidPrice->value()+0.55);
}

void BitcoinTrader::copyDonateButton()
{
	QApplication::clipboard()->setText(ui.bitcoinAddress->text());
}

void BitcoinTrader::ruleAddButton()
{
	AddRuleWindow addRule;
	if(addRule.exec()!=QDialog::Accepted)return;

	int curRow=ui.rulesTable->rowCount();
	ui.rulesTable->setRowCount(curRow+1);
	ui.rulesTable->setRowHeight(curRow,30);
	ui.rulesTable->setItem(curRow,0,new QTableWidgetItem("If "+addRule.getIfString()+" Goes "+addRule.getGoesString()+" Than"));ui.rulesTable->item(curRow,0)->setTextAlignment(Qt::AlignCenter);
	ui.rulesTable->setItem(curRow,1,new QTableWidgetItem(addRule.getPriceString()));ui.rulesTable->item(curRow,1)->setTextAlignment(Qt::AlignCenter);
	ui.rulesTable->setItem(curRow,2,new QTableWidgetItem(addRule.getSellBuyString()));ui.rulesTable->item(curRow,2)->setTextAlignment(Qt::AlignCenter);
	ui.rulesTable->setItem(curRow,3,new QTableWidgetItem(addRule.getBitcoinsString()));ui.rulesTable->item(curRow,3)->setTextAlignment(Qt::AlignCenter);
	ui.rulesTable->setItem(curRow,4,new QTableWidgetItem("pending"));ui.rulesTable->item(curRow,4)->setTextAlignment(Qt::AlignCenter);

	ui.rulesNoMessage->setVisible(false);
	ui.rulesTable->setVisible(true);
}

void BitcoinTrader::ruleRemove()
{
	TempWindow(this).exec();
}

void BitcoinTrader::ruleRemoveAll()
{
	TempWindow(this).exec();
}

void BitcoinTrader::beep()
{
	QApplication::beep();
}

void BitcoinTrader::accountUSDChanged(double){if(ui.accountUSDBeep->isChecked())beep();}
void BitcoinTrader::accountBTCChanged(double){if(ui.accountBTCBeep->isChecked())beep();}
void BitcoinTrader::marketLowChanged(double) {if(ui.marketLowBeep->isChecked()) beep();}
void BitcoinTrader::marketHighChanged(double){if(ui.marketHighBeep->isChecked())beep();}