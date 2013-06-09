#ifndef EXCHANGEMTGOX_H
#define EXCHANGEMTGOX_H

#include <QThread>
#include <QHttp>

class ExchangeMtGox : public QThread
{
	Q_OBJECT

public:
	ExchangeMtGox(QObject *parent);
	~ExchangeMtGox();

private:
	QHttp *http;
	void run();
private slots:
	void httpDone(bool);
};

#endif // EXCHANGEMTGOX_H
