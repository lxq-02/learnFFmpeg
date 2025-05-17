#pragma once

#include <QtWidgets/QWidget>
#include "ui_TestRGB.h"

class TestRGB : public QWidget
{
    Q_OBJECT

public:
    TestRGB(QWidget *parent = nullptr);
    ~TestRGB();

    void paintEvent(QPaintEvent* ev) override;

private:
    Ui::TestRGBClass ui;
};
