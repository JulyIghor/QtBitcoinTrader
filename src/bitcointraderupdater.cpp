//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include "bitcointraderupdater.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QApplication>
#include "main.h"

BitcoinTraderUpdater::BitcoinTraderUpdater()
	: QThread()
{
	downloadingFile=false;
	moveToThread(this);
	start();
}


void BitcoinTraderUpdater::run()
{
	httpUpdate=new QHttp("trader.uax.co",80,this);
	connect(httpUpdate,SIGNAL(done(bool)),this,SLOT(httpUpdateDone(bool)));

	httpUpdate->get("/API.php?Thread=Call&Object=General&Method=GetUpdate");
	exec();
}

QString BitcoinTraderUpdater::clearData(QString data)
{
	while(data.count()&&(data.at(0)=='{'||data.at(0)=='['||data.at(0)=='\"'))data.remove(0,1);
	while(data.count()&&(data.at(data.length()-1)=='}'||data.at(data.length()-1)==']'||data.at(data.length()-1)=='\"'))data.remove(data.length()-1,1);
	return data;
}

void BitcoinTraderUpdater::quitApp()
{
	QCoreApplication::quit();
}

void BitcoinTraderUpdater::httpUpdateDone(bool on)
{
	if(on){quitApp();return;}
	if(downloadingFile)
	{
		QByteArray allData=httpUpdate->readAll();
		QByteArray md5File=QCryptographicHash::hash(allData,QCryptographicHash::Md5).toHex().toLower();
		if(md5Value==md5File)
		{
			QString curExe=QApplication::applicationFilePath();
			QString updExe=curExe+".upd";
			QString bkpExe=curExe+".bkp";
			QFile::remove(updExe);
			QFile::remove(bkpExe);
			if(!QFile::exists(updExe))
			{
				QFile writeFile(updExe);
				if(writeFile.open(QIODevice::WriteOnly))
				{
					writeFile.write(allData);
					writeFile.close();
					QFile validateExe(updExe);
					if(validateExe.open(QIODevice::ReadOnly))
					{
						bool isValidBkp=QCryptographicHash::hash(validateExe.readAll(),QCryptographicHash::Md5).toHex().toLower()==md5Value;
						validateExe.close();
						if(isValidBkp)
						{
							QFile::rename(curExe,bkpExe);
							QFile::rename(updExe,curExe);
							QFile::remove(updExe);
							QFile::remove(bkpExe);
						}
					}
				}
			}
		}
		quitApp();
		return;
	}
	QString allData=httpUpdate->readAll();
	if(allData.length()<3){quitApp();return;}
	QString errorString;
	QString versionString;

	QString link;
	QStringList allDataList=allData.split(",");
	for(int n=0;n<allDataList.count();n++)
	{
		QStringList pairList=allDataList.at(n).split(":");
		if(pairList.count()==3)pairList.removeAt(1);
		if(pairList.count()==2)
		{
			QString name=clearData(pairList.first());
			QString value=clearData(pairList.last());
			if(name=="Error")errorString=value;else
				if(name=="Version")versionString=value;else
					if(name=="Md5")md5Value=value.toLower().toAscii();else
						if(name=="Link")link=value.replace("\\","").replace("//","");
		}
	}
	if(errorString.isEmpty())
	{
		if(versionString.toFloat()>appVerReal)
		{
			if(link.length()<5||md5Value.isEmpty()){quitApp();return;}
			downloadingFile=true;
			QString domain=link.split("/").first();
			QString linkUrl=link.right(link.length()-domain.length());
			httpUpdate->setHost(domain);
			httpUpdate->get(linkUrl);
		}
	}
}