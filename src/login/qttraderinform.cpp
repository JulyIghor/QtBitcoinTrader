#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include "main.h"
#include "qttraderinform.h"

QtTraderInform::QtTraderInform(QDialog* parent)
    : QDialog(parent),
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
    connect(m_again.data(),          &QCheckBox::toggled,   this, &QtTraderInform::againToggled);
}

QtTraderInform::~QtTraderInform()
{

}

void QtTraderInform::againToggled(bool checked)
{
    m_againIsChecked = checked;
}

bool QtTraderInform::dontShowAgain()
{
    return m_againIsChecked;
}
