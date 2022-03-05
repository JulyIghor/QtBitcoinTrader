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

#ifndef QTTRADERINFORM_H
#define QTTRADERINFORM_H

#include <QDialog>
#include <QScopedPointer>

class QPushButton;
class QTextEdit;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QCheckBox;

class QtTraderInform : public QDialog
{
    Q_OBJECT

public:
    explicit QtTraderInform(QDialog* parent = nullptr);
    ~QtTraderInform();
    bool dontShowAgain() const;

private slots:
    void againToggled(bool checked);

private:
    QScopedPointer<QVBoxLayout> m_mainLayout;

    QScopedPointer<QHBoxLayout> m_titleLayout;
    QScopedPointer<QLabel> m_logo;
    QScopedPointer<QLabel> m_title;

    QScopedPointer<QLabel> m_info;

    QScopedPointer<QHBoxLayout> m_buttonsLayout;
    QScopedPointer<QPushButton> m_registerButton;
    QScopedPointer<QCheckBox> m_again;

    bool m_againIsChecked;
};

#endif
