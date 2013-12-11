// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "donatebtc.h"
#include <QClipboard>
#include "main.h"

DonateBTC::DonateBTC(QPushButton *button, bool btc)
	: QMenu()
{
	ui.setupUi(this);
	isBtc=btc;
	if(btc)
	{
		ui.btcLabel11->setPixmap(QPixmap(":/Resources/CurrencySign/BTC.png"));
		ui.bitcoinAddress->setText("1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc");
	}
	else//ltc
	{
		ui.btcLabel11->setPixmap(QPixmap(":/Resources/CurrencySign/LTC.png"));
		ui.bitcoinAddress->setText("LPJZTHaHdMwm4kHV6WGNegmoWDSzC9rzYB");
	}
	setAttribute(Qt::WA_DeleteOnClose,true);
	connect(this,SIGNAL(setCheckedButton(bool)),button,SLOT(setChecked(bool)));
	connect(this,SIGNAL(aboutToHide()),this,SLOT(aboutToHideWindow()));
	julyTranslator.translateUi(this);
	mainWindow.fixAllChildButtonsAndLabels(this);
	setFixedHeight(minimumSizeHint().height());
	setFixedWidth(width());
}

DonateBTC::~DonateBTC()
{

}

void DonateBTC::aboutToHideWindow()
{
	emit setCheckedButton(false);
}

void DonateBTC::on_copyDonateButton_clicked()
{
	QApplication::clipboard()->setText(ui.bitcoinAddress->text());
}