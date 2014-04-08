/********************************************************************************
** Form generated from reading UI file 'TitleView.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TITLEVIEW_H
#define UI_TITLEVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_titleview
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QWidget *widget_display;
    QVBoxLayout *verticalLayout_2;

    void setupUi(QWidget *titleview)
    {
        if (titleview->objectName().isEmpty())
            titleview->setObjectName(QString::fromUtf8("titleview"));
        titleview->setWindowModality(Qt::NonModal);
        titleview->resize(333, 314);
        gridLayoutWidget = new QWidget(titleview);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(0, 0, 328, 301));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        widget_display = new QWidget(gridLayoutWidget);
        widget_display->setObjectName(QString::fromUtf8("widget_display"));

        verticalLayout->addWidget(widget_display);


        gridLayout->addLayout(verticalLayout, 1, 0, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setSizeConstraint(QLayout::SetDefaultConstraint);

        gridLayout->addLayout(verticalLayout_2, 0, 0, 1, 1);

        gridLayout->setRowStretch(1, 4);
        gridLayout->setRowMinimumHeight(1, 4);

        retranslateUi(titleview);

        QMetaObject::connectSlotsByName(titleview);
    } // setupUi

    void retranslateUi(QWidget *titleview)
    {
        Q_UNUSED(titleview);
    } // retranslateUi

};

namespace Ui {
    class titleview: public Ui_titleview {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TITLEVIEW_H
