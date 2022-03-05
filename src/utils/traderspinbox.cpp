#include "traderspinbox.h"

TraderSpinBox::TraderSpinBox(QWidget* parent) : QDoubleSpinBox(parent)
{
    installEventFilter(this);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMinimumWidth(100);
    setSingleStep(0.01);
}

QValidator::State TraderSpinBox::validate(QString& input, int& pos) const
{
    if (input.indexOf('.') == -1)
    {
        int posComma = input.indexOf(',');

        if (posComma >= 0 && input.lastIndexOf(',') == posComma)
            input[posComma] = '.';
    }

    return QDoubleSpinBox::validate(input, pos);
}
