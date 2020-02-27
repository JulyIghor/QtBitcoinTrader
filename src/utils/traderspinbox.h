#ifndef TRADERSPINBOX_H
#define TRADERSPINBOX_H

#include <QDoubleSpinBox>

class TraderSpinBox : public QDoubleSpinBox
{
public:
    TraderSpinBox(QWidget* parent);

private:
    QValidator::State validate(QString& input, int& pos) const override;
};

#endif // TRADERSPINBOX_H
