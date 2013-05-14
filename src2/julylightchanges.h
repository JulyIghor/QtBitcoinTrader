//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#ifndef JULYLIGHTCHANGES_H
#define JULYLIGHTCHANGES_H

#include <QObject>
#include <QDoubleSpinBox>
#include <QTimer>

class JulyLightChanges : public QObject
{
	Q_OBJECT

public:
	JulyLightChanges(QDoubleSpinBox *parent, QString colorL="#FFAAAA", QString colorH="#AAFFAA");
	~JulyLightChanges();
private:
	double lastValue;
	QString colorL;
	QString colorH;
	QDoubleSpinBox *parentSheet;
	QTimer *changeTimer;
private slots:
	void changeTimerSlot();
public slots:
	void valueChanged(double);
	
};

#endif // JULYLIGHTCHANGES_H
