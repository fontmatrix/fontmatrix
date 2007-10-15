/********************************************************************************
** Form generated from reading ui file 'mainview.ui'
**
** Created: lun. oct. 15 16:59:04 2007
**      by: Qt User Interface Compiler version 4.3.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MAINVIEW_H
#define UI_MAINVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGraphicsView>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextBrowser>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_MainView
{
public:
    QGridLayout *gridLayout;
    QFrame *manageFrame;
    QGridLayout *gridLayout1;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout1;
    QLabel *label_2;
    QLabel *label_3;
    QVBoxLayout *vboxLayout2;
    QLineEdit *searchString;
    QComboBox *searchField;
    QVBoxLayout *vboxLayout3;
    QSpacerItem *spacerItem;
    QPushButton *searchButton;
    QHBoxLayout *hboxLayout1;
    QLabel *label;
    QComboBox *orderingCombo;
    QPushButton *editAllButton;
    QTreeWidget *fontTree;
    QTextBrowser *fontInfoText;
    QTextBrowser *glyphInfo;
    QFrame *viewFrame;
    QGridLayout *gridLayout2;
    QToolBox *toolBox;
    QWidget *pageText;
    QGridLayout *gridLayout3;
    QTextEdit *textEdit;
    QWidget *pageRender;
    QGridLayout *gridLayout4;
    QSpacerItem *spacerItem1;
    QSlider *renderZoom;
    QGraphicsView *loremView;
    QWidget *page;
    QGridLayout *gridLayout5;
    QSpacerItem *spacerItem2;
    QSlider *allZoom;
    QGraphicsView *abcView;

    void setupUi(QWidget *MainView)
    {
    if (MainView->objectName().isEmpty())
        MainView->setObjectName(QString::fromUtf8("MainView"));
    MainView->resize(891, 767);
    gridLayout = new QGridLayout(MainView);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    manageFrame = new QFrame(MainView);
    manageFrame->setObjectName(QString::fromUtf8("manageFrame"));
    manageFrame->setFrameShape(QFrame::StyledPanel);
    manageFrame->setFrameShadow(QFrame::Raised);
    gridLayout1 = new QGridLayout(manageFrame);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    label_2 = new QLabel(manageFrame);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    vboxLayout1->addWidget(label_2);

    label_3 = new QLabel(manageFrame);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    vboxLayout1->addWidget(label_3);


    hboxLayout->addLayout(vboxLayout1);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    searchString = new QLineEdit(manageFrame);
    searchString->setObjectName(QString::fromUtf8("searchString"));

    vboxLayout2->addWidget(searchString);

    searchField = new QComboBox(manageFrame);
    searchField->setObjectName(QString::fromUtf8("searchField"));

    vboxLayout2->addWidget(searchField);


    hboxLayout->addLayout(vboxLayout2);

    vboxLayout3 = new QVBoxLayout();
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    spacerItem = new QSpacerItem(61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    vboxLayout3->addItem(spacerItem);

    searchButton = new QPushButton(manageFrame);
    searchButton->setObjectName(QString::fromUtf8("searchButton"));

    vboxLayout3->addWidget(searchButton);


    hboxLayout->addLayout(vboxLayout3);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label = new QLabel(manageFrame);
    label->setObjectName(QString::fromUtf8("label"));

    hboxLayout1->addWidget(label);

    orderingCombo = new QComboBox(manageFrame);
    orderingCombo->setObjectName(QString::fromUtf8("orderingCombo"));

    hboxLayout1->addWidget(orderingCombo);


    hboxLayout->addLayout(hboxLayout1);


    vboxLayout->addLayout(hboxLayout);

    editAllButton = new QPushButton(manageFrame);
    editAllButton->setObjectName(QString::fromUtf8("editAllButton"));

    vboxLayout->addWidget(editAllButton);

    fontTree = new QTreeWidget(manageFrame);
    fontTree->setObjectName(QString::fromUtf8("fontTree"));
    fontTree->setColumnCount(2);

    vboxLayout->addWidget(fontTree);

    fontInfoText = new QTextBrowser(manageFrame);
    fontInfoText->setObjectName(QString::fromUtf8("fontInfoText"));

    vboxLayout->addWidget(fontInfoText);

    glyphInfo = new QTextBrowser(manageFrame);
    glyphInfo->setObjectName(QString::fromUtf8("glyphInfo"));

    vboxLayout->addWidget(glyphInfo);


    gridLayout1->addLayout(vboxLayout, 0, 0, 1, 1);


    gridLayout->addWidget(manageFrame, 0, 0, 1, 1);

    viewFrame = new QFrame(MainView);
    viewFrame->setObjectName(QString::fromUtf8("viewFrame"));
    viewFrame->setFrameShape(QFrame::StyledPanel);
    viewFrame->setFrameShadow(QFrame::Raised);
    gridLayout2 = new QGridLayout(viewFrame);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    toolBox = new QToolBox(viewFrame);
    toolBox->setObjectName(QString::fromUtf8("toolBox"));
    pageText = new QWidget();
    pageText->setObjectName(QString::fromUtf8("pageText"));
    pageText->setGeometry(QRect(0, 0, 112, 112));
    gridLayout3 = new QGridLayout(pageText);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    textEdit = new QTextEdit(pageText);
    textEdit->setObjectName(QString::fromUtf8("textEdit"));

    gridLayout3->addWidget(textEdit, 0, 0, 1, 1);

    toolBox->addItem(pageText, QApplication::translate("MainView", "input text", 0, QApplication::UnicodeUTF8));
    pageRender = new QWidget();
    pageRender->setObjectName(QString::fromUtf8("pageRender"));
    pageRender->setGeometry(QRect(0, 0, 407, 634));
    gridLayout4 = new QGridLayout(pageRender);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    spacerItem1 = new QSpacerItem(161, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout4->addItem(spacerItem1, 0, 0, 1, 1);

    renderZoom = new QSlider(pageRender);
    renderZoom->setObjectName(QString::fromUtf8("renderZoom"));
    renderZoom->setMaximum(1000);
    renderZoom->setValue(100);
    renderZoom->setTracking(false);
    renderZoom->setOrientation(Qt::Horizontal);

    gridLayout4->addWidget(renderZoom, 0, 1, 1, 1);

    loremView = new QGraphicsView(pageRender);
    loremView->setObjectName(QString::fromUtf8("loremView"));

    gridLayout4->addWidget(loremView, 1, 0, 1, 2);

    toolBox->addItem(pageRender, QApplication::translate("MainView", "Rendered text", 0, QApplication::UnicodeUTF8));
    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));
    page->setGeometry(QRect(0, 0, 407, 634));
    gridLayout5 = new QGridLayout(page);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout5->addItem(spacerItem2, 0, 0, 1, 1);

    allZoom = new QSlider(page);
    allZoom->setObjectName(QString::fromUtf8("allZoom"));
    allZoom->setMaximum(1000);
    allZoom->setValue(100);
    allZoom->setTracking(false);
    allZoom->setOrientation(Qt::Horizontal);

    gridLayout5->addWidget(allZoom, 0, 1, 1, 1);

    abcView = new QGraphicsView(page);
    abcView->setObjectName(QString::fromUtf8("abcView"));

    gridLayout5->addWidget(abcView, 1, 0, 1, 2);

    toolBox->addItem(page, QApplication::translate("MainView", "All glyphs", 0, QApplication::UnicodeUTF8));

    gridLayout2->addWidget(toolBox, 0, 0, 1, 1);


    gridLayout->addWidget(viewFrame, 0, 1, 1, 1);


    retranslateUi(MainView);

    toolBox->setCurrentIndex(1);


    QMetaObject::connectSlotsByName(MainView);
    } // setupUi

    void retranslateUi(QWidget *MainView)
    {
    MainView->setWindowTitle(QApplication::translate("MainView", "main view widget", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("MainView", "Search", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("MainView", "In", 0, QApplication::UnicodeUTF8));
    searchButton->setText(QApplication::translate("MainView", "Search", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("MainView", "Ordering", 0, QApplication::UnicodeUTF8));
    editAllButton->setText(QApplication::translate("MainView", "Edit All", 0, QApplication::UnicodeUTF8));
    fontTree->headerItem()->setText(0, QApplication::translate("MainView", "1", 0, QApplication::UnicodeUTF8));
    fontTree->headerItem()->setText(1, QApplication::translate("MainView", "2", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(pageText), QApplication::translate("MainView", "input text", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(pageRender), QApplication::translate("MainView", "Rendered text", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(page), QApplication::translate("MainView", "All glyphs", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MainView);
    } // retranslateUi

};

namespace Ui {
    class MainView: public Ui_MainView {};
} // namespace Ui

#endif // UI_MAINVIEW_H
