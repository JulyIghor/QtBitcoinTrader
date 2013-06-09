#include "exchangemtgox.h"
#include "main.h"

ExchangeMtGox::ExchangeMtGox(QObject *parent)
	: QThread(parent)
{
	http=0;
	moveToThread(this);
	start();
}

ExchangeMtGox::~ExchangeMtGox()
{
	if(http)delete http;
}

void ExchangeMtGox::run()
{
	http=new QHttp(this);
	connect(http,SIGNAL(done(bool)),this,SLOT(httpDone(bool)));
	exec();
}

void ExchangeMtGox::httpDone(bool error)
{

}