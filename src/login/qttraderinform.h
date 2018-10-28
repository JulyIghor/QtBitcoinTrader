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
    bool dontShowAgain();

private slots:
    void againToggled(bool checked);

private:
    QScopedPointer<QVBoxLayout> m_mainLayout;

    QScopedPointer<QHBoxLayout> m_titleLayout;
    QScopedPointer<QLabel>      m_logo;
    QScopedPointer<QLabel>      m_title;

    QScopedPointer<QLabel>      m_info;

    QScopedPointer<QHBoxLayout> m_buttonsLayout;
    QScopedPointer<QPushButton> m_registerButton;
    QScopedPointer<QCheckBox>   m_again;

    bool                        m_againIsChecked;
};

#endif
