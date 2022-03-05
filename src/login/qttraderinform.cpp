//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2022 July Ighor <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "qttraderinform.h"
#include "main.h"
#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QTextEdit>
#include <QVBoxLayout>

QtTraderInform::QtTraderInform(QDialog* parent) :
    QDialog(parent),
    m_mainLayout(new QVBoxLayout()),
    m_titleLayout(new QHBoxLayout()),
    m_logo(new QLabel()),
    m_title(new QLabel(julyTr("NEW_EXCHANGE_QTTRADER", "HFT exchange\nQt Trader 2.0"))),
    m_info(new QLabel()),
    m_buttonsLayout(new QHBoxLayout()),
    m_registerButton(new QPushButton(julyTr("SIGN_UP_FOR_ACCESS", "Sign up\nfor Early Access"))),
    m_again(new QCheckBox(julyTr("DONT_SHOW_AGAIN", "Don't show again"))),
    m_againIsChecked(false)
{
    m_info->setTextFormat(Qt::TextFormat::RichText);
    m_info->setOpenExternalLinks(true);
    m_info->setText("Beta testing of the new excchange Qt Trader has started<br><br>"
                    "The first wave of testers were accepted<br><br>"
                    "Welcome to sign up for the second wave at <a href=\"qttrader.com\">qttrader.com</a>");
    QPixmap p("://Resources/QtTrader.png");

    m_logo->setFixedSize(200, 50);
    m_logo->setPixmap(p.scaled(m_logo->width(), m_logo->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_title->setStyleSheet("font-size:14px");
    m_title->setAlignment(Qt::AlignCenter);
    m_titleLayout->addWidget(m_logo.data());
    m_titleLayout->addWidget(m_title.data());
    m_titleLayout->setSpacing(10);

    m_info->setStyleSheet("background:white;text-align:center;padding:15px;font-size:20px;border:1px solid lightgrey");
    m_info->setAlignment(Qt::AlignCenter);

    m_registerButton->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_buttonsLayout->addWidget(m_again.data());
    m_buttonsLayout->addWidget(m_registerButton.data());
    m_buttonsLayout->setSpacing(10);

    m_mainLayout->addLayout(m_titleLayout.data());
    m_mainLayout->addWidget(m_info.data());
    m_mainLayout->addLayout(m_buttonsLayout.data());

    setLayout(m_mainLayout.data());
    setWindowTitle(julyTr("NEW_EXCHANGE", "HFT exchange"));
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    setFixedSize(minimumSizeHint());

    connect(m_registerButton.data(), &QPushButton::clicked, this, &QDialog::accept);
    connect(m_again.data(), &QCheckBox::toggled, this, &QtTraderInform::againToggled);
}

QtTraderInform::~QtTraderInform()
{
}

void QtTraderInform::againToggled(bool checked)
{
    m_againIsChecked = checked;
}

bool QtTraderInform::dontShowAgain() const
{
    return m_againIsChecked;
}
