/********************************************************************************
** Form generated from reading ui file 'fontaction.ui'
**
** Created: lun. oct. 15 16:59:04 2007
**      by: Qt User Interface Compiler version 4.3.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_FONTACTION_H
#define UI_FONTACTION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_FontAction
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QLabel *titleLabel;
    QSpacerItem *spacerItem;
    QCheckBox *activatedBox;
    QListWidget *tagsListWidget;
    QHBoxLayout *hboxLayout1;
    QLineEdit *newTagText;
    QPushButton *newTagButton;
    QSpacerItem *spacerItem1;
    QDialogButtonBox *buttonBox;

    void setupUi(QWidget *FontAction)
    {
    if (FontAction->objectName().isEmpty())
        FontAction->setObjectName(QString::fromUtf8("FontAction"));
    FontAction->resize(292, 523);
    gridLayout = new QGridLayout(FontAction);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    titleLabel = new QLabel(FontAction);
    titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
    QFont font;
    font.setPointSize(10);
    titleLabel->setFont(font);
    titleLabel->setFrameShape(QFrame::StyledPanel);
    titleLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    hboxLayout->addWidget(titleLabel);

    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    activatedBox = new QCheckBox(FontAction);
    activatedBox->setObjectName(QString::fromUtf8("activatedBox"));
    activatedBox->setLayoutDirection(Qt::RightToLeft);

    hboxLayout->addWidget(activatedBox);


    vboxLayout->addLayout(hboxLayout);

    tagsListWidget = new QListWidget(FontAction);
    tagsListWidget->setObjectName(QString::fromUtf8("tagsListWidget"));

    vboxLayout->addWidget(tagsListWidget);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    newTagText = new QLineEdit(FontAction);
    newTagText->setObjectName(QString::fromUtf8("newTagText"));

    hboxLayout1->addWidget(newTagText);

    newTagButton = new QPushButton(FontAction);
    newTagButton->setObjectName(QString::fromUtf8("newTagButton"));

    hboxLayout1->addWidget(newTagButton);

    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem1);


    vboxLayout->addLayout(hboxLayout1);

    buttonBox = new QDialogButtonBox(FontAction);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    vboxLayout->addWidget(buttonBox);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);

    QWidget::setTabOrder(activatedBox, tagsListWidget);
    QWidget::setTabOrder(tagsListWidget, newTagText);
    QWidget::setTabOrder(newTagText, newTagButton);
    QWidget::setTabOrder(newTagButton, buttonBox);

    retranslateUi(FontAction);

    QMetaObject::connectSlotsByName(FontAction);
    } // setupUi

    void retranslateUi(QWidget *FontAction)
    {
    FontAction->setWindowTitle(QApplication::translate("FontAction", "Form", 0, QApplication::UnicodeUTF8));
    titleLabel->setText(QApplication::translate("FontAction", "action", 0, QApplication::UnicodeUTF8));
    activatedBox->setToolTip(QApplication::translate("FontAction", "Activated Or Not", 0, QApplication::UnicodeUTF8));
    activatedBox->setText(QApplication::translate("FontAction", "Active", 0, QApplication::UnicodeUTF8));
    newTagButton->setText(QApplication::translate("FontAction", "Add Tag", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(FontAction);
    } // retranslateUi

};

namespace Ui {
    class FontAction: public Ui_FontAction {};
} // namespace Ui

#endif // UI_FONTACTION_H
