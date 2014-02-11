// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "thisfeatureunderdevelopment.h"
#include "main.h"
#include "donatepanel.h"

ThisFeatureUnderDevelopment::ThisFeatureUnderDevelopment(QWidget *parent)
	: QDialog(parent)
{
	if(parent==0)parent=this;
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);
	ui.donateGroupBoxBottom->layout()->addWidget(new DonatePanel(parent));
	themeChanged();
	connect(baseValues.mainWindow_,SIGNAL(themeChanged()),this,SLOT(themeChanged()));
}

ThisFeatureUnderDevelopment::~ThisFeatureUnderDevelopment()
{

}

void ThisFeatureUnderDevelopment::themeChanged()
{
	ui.labelNotAvailable->setStyleSheet("background: "+baseValues.appTheme.white.name()+"; border-radius: 8px; border: 1px solid "+baseValues.appTheme.gray.name());
}