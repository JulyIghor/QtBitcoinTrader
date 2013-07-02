// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef FEECALCULATOR_H
#define FEECALCULATOR_H

#include <QDialog>
#include "ui_feecalculator.h"

class FeeCalculator : public QDialog
{
	Q_OBJECT

public:
	FeeCalculator();
	~FeeCalculator();

private:
	bool buyPaidLocked;
	bool buyBtcReceivedLocked;
	bool buyBtcLocked;
	Ui::FeeCalculator ui;
public slots:
	void languageChanged();
private slots:
	void setStaysOnTop(bool);
	void setZeroProfitPrice();
	void profitLossChanged(double);
	void buyBtcChanged(double);
	void buyPriceChanged(double);
	void buyTotalPaidChanged(double);
	void buyBtcReceivedChanged(double);
	void sellPriceChanged(double);
	void sellAmountChanged(double);
	void sellFiatReceived(double);
	void feeChanged(double);

};

#endif // FEECALCULATOR_H
