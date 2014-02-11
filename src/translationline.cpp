// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "translationline.h"
#include <QTextDocument>
#include "main.h"

TranslationLine::TranslationLine(QWidget *parent)
	: QTextEdit(parent)
{
	fixingSize=false;
	setWordWrapMode(QTextOption::WrapAnywhere);
	setTabChangesFocus(true);
	setAcceptRichText(false);
    setMinimumWidth(100);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	connect(this,SIGNAL(textChanged()),this,SLOT(textChangedSlot()));
}

TranslationLine::~TranslationLine()
{

}

void TranslationLine::focusInEvent(QFocusEvent *e)
{
	QTextEdit::focusInEvent(e);
	selectAll();
}

void TranslationLine::focusOutEvent(QFocusEvent *e)
{
	QTextEdit::focusOutEvent(e);
	QTextCursor cursor=textCursor(); 
	cursor.clearSelection(); 
	setTextCursor(cursor); 
}

void TranslationLine::textChangedSlot()
{
	fixSize();
	if(isChanged())setStyleSheet("color: "+baseValues.appTheme.black.name());
	else setStyleSheet("color: "+baseValues.appTheme.red.name());
	emit lineTextChanged();
}

void TranslationLine::fixSize()
{
	if(fixingSize)return;
	fixingSize=true;
	int docHeight=document()->size().height();
	if(docHeight>0)setFixedHeight(docHeight);
	fixingSize=false;
}


void TranslationLine::setDefaultText(QString defText)
{
	if(defText!="yyyy-MM-dd HH:mm:ss"&&defText!=baseValues.exchangeName+":")defaultText=defText.replace("<br>","\n");
}

void TranslationLine::setItemText(QString text)
{
	if(text.isEmpty())text=defaultText;
	text.replace("<br>","\n");
	setPlainText(text);
	document()->adjustSize();
	fixSize();
}

void TranslationLine::resizeEvent(QResizeEvent *event)
{
	QTextEdit::resizeEvent(event);
	fixSize();
}

QString TranslationLine::getValidText()
{
	QString validText=toPlainText();
	validText.replace("\r","");
	validText.replace("\n","<br>");

	QString lastText;
	while(lastText!=validText&&!validText.isEmpty())
	{
		validText.replace("<br> ","<br>");
		lastText=validText;
	}

	lastText.clear();
	while(lastText!=validText&&!validText.isEmpty())
	{
		validText.replace(" <br>","<br>");
		lastText=validText;
	}

	return validText;
}
