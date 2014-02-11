// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "donatewebmoney.h"
#include <QClipboard>
#include "main.h"

DonateWebMoney::DonateWebMoney(QPushButton *button)
	: QMenu()
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);
	connect(this,SIGNAL(setCheckedButton(bool)),button,SLOT(setChecked(bool)));
	connect(this,SIGNAL(aboutToHide()),this,SLOT(aboutToHideWindow()));
	julyTranslator.translateUi(this);
	mainWindow.fixAllChildButtonsAndLabels(this);
	setFixedHeight(minimumSizeHint().height());
	setFixedWidth(width());
	QPalette pal=palette();
	pal.setColor(QPalette::Window, QColor(80,80,80));
	setPalette(pal);
}

DonateWebMoney::~DonateWebMoney()
{

}

void DonateWebMoney::mouseReleaseEvent(QMouseEvent *event)
{
	event->ignore();
}

void DonateWebMoney::aboutToHideWindow()
{
	emit setCheckedButton(false);
}

void DonateWebMoney::on_wmzCopy_clicked()
{
	QApplication::clipboard()->setText(ui.wmz->text());
}

void DonateWebMoney::on_wmrCopy_clicked()
{
	QApplication::clipboard()->setText(ui.wmr->text());
}

void DonateWebMoney::on_wmeCopy_clicked()
{
	QApplication::clipboard()->setText(ui.wme->text());
}

void DonateWebMoney::on_wmuCopy_clicked()
{
	QApplication::clipboard()->setText(ui.wmu->text());
}