#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QCoreApplication>
#include <QDir>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("About TableHistory");
    resize(380, 280); // Slightly increased height to accommodate the icon

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    // App Icon Label
    auto *iconLabel = new QLabel(this);
    iconLabel->setAlignment(Qt::AlignCenter);
    QString iconPath = QCoreApplication::applicationDirPath() + "/../Resources/icon.png";
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        iconLabel->setPixmap(pixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    auto *titleLabel = new QLabel("<h2>TableHistory</h2><h3>Version 1.0.0</h3>", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *descLabel = new QLabel("A free and open source C++ Qt 6 application<br>for managing historical records and Gantt timelines.", this);
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);

    // Copyright Notice
    auto *copyrightLabel = new QLabel("Copyright © 2026 Massimo Nardello.<br>All rights reserved under GPLv3 licence.", this);
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setStyleSheet("color: #555555; font-size: 10pt;");

    auto *closeBtn = new QPushButton("Close", this);
    closeBtn->setFixedWidth(100);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(iconLabel);
    layout->addSpacing(5);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(copyrightLabel);
    layout->addSpacing(10);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);
}
