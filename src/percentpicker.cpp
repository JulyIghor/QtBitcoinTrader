// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "percentpicker.h"

PercentPicker::PercentPicker(QPushButton *_button, QDoubleSpinBox *_spinBox, double _maxValue)
	: QMenu()
{
	maxValue=_maxValue;
	spinBox=_spinBox;
	button=_button;
	if(maxValue==0.0)maxValue=0.000000001;
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);
	setFixedSize(minimumSizeHint().width(),200);
	ui.verticalSlider->setValue(spinBox->value()*100.0/maxValue);
	ui.verticalSlider->setFocus();
}

PercentPicker::~PercentPicker()
{

}

void PercentPicker::mouseReleaseEvent(QMouseEvent *event)
{
	event->ignore();
}

void PercentPicker::on_percentTo1_clicked()
{
	if(ui.percentTo1->text()=="1%")
	{
		ui.verticalSlider->setValue(1);
		ui.percentTo1->setText("100%");
	}
	else
	if(ui.percentTo1->text()=="100%")
	{
		ui.verticalSlider->setValue(100);
		ui.percentTo1->setText("1%");
	}
}

void PercentPicker::on_verticalSlider_valueChanged(int val)
{
	if(val==1)ui.percentTo1->setText("100%");else
	if(val==100)ui.percentTo1->setText("1%");
	if(isVisible())spinBox->setValue((double)val*maxValue/100.0);
}