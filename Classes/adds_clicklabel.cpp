#include "adds_clicklabel.h"

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags)
    : QLabel(parent) {
}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent*) {
    emit clicked();
}

void ClickableLabel::mouseDoubleClickEvent(QMouseEvent*) {
    emit doubleClicked();
}
