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

#include "featuredexchangesdialog.h"
#include "exchangebutton.h"
#include "julyhttp.h"
#include "main.h"
#include "ui_featuredexchangesdialog.h"
#include <QDesktopServices>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <QtCore/qmath.h>

FeaturedExchangesDialog::FeaturedExchangesDialog() : QDialog(), exchangeNum(-1), ui(new Ui::FeaturedExchangesDialog)
{
    ui->setupUi(this);
    ui->okButton->setEnabled(false);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle("Qt Bitcoin Trader v" + baseValues.appVerStr + " - " + julyTr("FEATURED_EXCHANGES", "Featured Exchanges"));

    QSettings listSettings(":/Resources/Exchanges/List.ini", QSettings::IniFormat);
    allExchangesList = listSettings.childGroups();

    {
        QScopedPointer<JulyHttp> httpGet(new JulyHttp("qbtapi.centrabit.com", nullptr, this, true, false));
        connect(httpGet.data(), &JulyHttp::dataReceived, this, &FeaturedExchangesDialog::dataReceived);
        httpGet->secondTimer->stop();
        httpGet->noReconnect = true;
        httpGet->ignoreError = true;

        QElapsedTimer elapsedRequest;
        elapsedRequest.restart();

        httpGet->sendData(145, 0, "GET /?Object=General&Method=FeaturedExchanges", nullptr, nullptr, 0);
        httpGet->destroyClass = true;

        int counter = 0;

        while (cacheData.isEmpty() && counter++ < 30 && elapsedRequest.elapsed() < 5000)
        {
            QEventLoop loop;
            QTimer::singleShot(100, &loop, SLOT(quit()));
            loop.exec();
        }

        featuredExchangesList = QString(mainWindow.getMidData("Exchanges\":[", "]", &cacheData)).split(",");
        cacheData.clear();
    }

    for (int n = featuredExchangesList.size() - 1; n >= 0; n--)
    {
        if (featuredExchangesList.at(n).isEmpty())
            featuredExchangesList.removeAt(n);
    }

    if (!featuredExchangesList.contains("0"))
        featuredExchangesList.prepend("0");

    if (featuredExchangesList.isEmpty())
    {
        QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
        featuredExchangesList = settings.value("LastFeaturedExchanges", featuredExchangesList).toStringList();
    }
    else
    {
        QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
        settings.setValue("LastFeaturedExchanges", featuredExchangesList);
    }

    for (int n = featuredExchangesList.size() - 1; n >= 0; n--)
    {
        if (featuredExchangesList.at(n).isEmpty())
            featuredExchangesList.removeAt(n);
        else if (featuredExchangesList.at(n).length() < 2)
            featuredExchangesList[n].prepend("0");
    }

    if (featuredExchangesList.empty())
    {
        exchangeNum = -3;
        return;
    }

    removeNotValidExchanges();

    qint32 countCol = 3;

    if (featuredExchangesList.size() <= 4)
        countCol = 2;

    for (int i = 0; i < featuredExchangesList.size(); i++)
    {
        QString currentName = listSettings.value(featuredExchangesList.at(i) + "/Name").toString();
        QString currentLogo = fixLogo(listSettings.value(featuredExchangesList.at(i) + "/Logo").toString());
        QString currentURL = fixURL(listSettings.value(featuredExchangesList.at(i) + "/GetApiUrl").toString());

        if (currentName.isEmpty() || currentLogo.isEmpty() || currentURL.isEmpty())
            continue;

        auto* exchangeItem =
            new ExchangeButton(currentLogo, loadCurrencies(currentName), currentURL, featuredExchangesListIndex.at(i), this);

        if (countCol == 3 && i == featuredExchangesList.size() - 1 && (i - qFloor(i / countCol) * countCol) == 0)
            i++;

        ui->gridLayoutExchange->addWidget(exchangeItem, qFloor(i / countCol), i - qFloor(i / countCol) * countCol, Qt::AlignCenter);
    }

    julyTranslator.translateUi(this);

    QSize minSizeHint = minimumSizeHint();

    if (mainWindow.isValidSize(&minSizeHint))
        setFixedSize(minSizeHint);
}

FeaturedExchangesDialog::~FeaturedExchangesDialog()
{
}

void FeaturedExchangesDialog::dataReceived(const QByteArray& data, int /*unused*/, int /*unused*/)
{
    cacheData = data;
}

void FeaturedExchangesDialog::removeNotValidExchanges()
{
    for (int i = 0; i < featuredExchangesList.size(); i++)
    {
        bool isValid = false;

        for (int j = 0; j < allExchangesList.size(); j++)
        {
            if (featuredExchangesList.at(i) == allExchangesList.at(j))
            {
                featuredExchangesListIndex.insert(i, featuredExchangesList.at(i).toInt());
                isValid = true;
                break;
            }
        }

        if (!isValid)
        {
            featuredExchangesList.removeAt(i);
            i--;
        }
    }
}

QString FeaturedExchangesDialog::loadCurrencies(const QString& name)
{
    QString nameIni = name;
    nameIni.remove(" ");
    nameIni.remove("-");
    nameIni.remove(".");
    QSettings listSettings(":/Resources/Exchanges/" + nameIni + ".ini", QSettings::IniFormat);
    QStringList exchangesList = listSettings.childGroups();
    QString currencies = "";
    QString currencies12;
    QString currencies1;
    QString currencies2;

    for (int n = 0; n < exchangesList.size(); n++)
    {
        currencies12 = listSettings.value(exchangesList.at(n) + "/Symbol").toString();
        int posSplitter = currencies12.indexOf('/');

        if (posSplitter == -1)
        {
            currencies1 = currencies12.left(3);
            currencies2 = currencies12.right(3);
        }
        else
        {
            currencies1 = currencies12.left(posSplitter);
            currencies2 = currencies12.right(currencies12.size() - posSplitter - 1);
        }

        if (!currencies.contains(currencies1))
            currencies += currencies1 + ", ";

        if (!currencies.contains(currencies2))
            currencies += currencies2 + ", ";
    }

    currencies.chop(2);
    return currencies;
}

void FeaturedExchangesDialog::selectExchange(qint32 num)
{
    exchangeNum = num;
    ui->okButton->setEnabled(true);
}

QString FeaturedExchangesDialog::fixLogo(const QString& currentLogo)
{
    qint32 dotPosition = currentLogo.lastIndexOf(".");

    if (dotPosition >= 0)
    {
        QString currentLogoCopy = currentLogo;
        currentLogoCopy.insert(dotPosition, "_Big");
        return currentLogoCopy;
    }

    return currentLogo;
}

QString FeaturedExchangesDialog::fixURL(QString currentURL)
{
    quint32 slashNum = 0;

    for (qint32 i = 0; i < currentURL.length(); i++)
    {
        if (currentURL[i] == '/')
            slashNum++;

        if (slashNum > 2)
        {
            currentURL.chop(currentURL.length() - i);
            break;
        }
    }

    return currentURL;
}

void FeaturedExchangesDialog::on_okButton_clicked() const
{
    if (exchangeNum == 0)
    {
        QDesktopServices::openUrl(QUrl("https://qttrader.com/"));
        return;
    }

    //    accept();
}

void FeaturedExchangesDialog::on_otherExchangesButton_clicked()
{
    exchangeNum = -2;
    accept();
}
