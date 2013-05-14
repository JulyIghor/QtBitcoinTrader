//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#ifndef BITCOINTRADERUPDATER_H
#define BITCOINTRADERUPDATER_H

#include <QThread>
#include <QHttp>

class BitcoinTraderUpdater : public QThread
{
	Q_OBJECT

public:
	BitcoinTraderUpdater();

private:
	QByteArray md5Value;
	bool downloadingFile;
	QString clearData(QString data);
	QHttp *httpUpdate;
	void run();
	void quitApp();
public slots:
	void httpUpdateDone(bool);
	
};

#endif // BITCOINTRADERUPDATER_H
