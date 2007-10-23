/********************************************************************************
** Form generated from reading ui file 'mainview.ui'
**
** Created: mar. oct. 23 23:19:23 2007
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
#include <QtGui/QToolBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_MainView
{
public:
    QGridLayout *gridLayout;
    QFrame *manageFrame;
    QVBoxLayout *vboxLayout;
    QGridLayout *gridLayout1;
    QVBoxLayout *vboxLayout1;
    QLabel *search;
    QLabel *in;
    QVBoxLayout *vboxLayout2;
    QLineEdit *searchString;
    QComboBox *searchField;
    QVBoxLayout *vboxLayout3;
    QPushButton *searchButton;
    QFrame *line;
    QLabel *FontInformations;
    QTextBrowser *fontInfoText;
    QLabel *FontList;
    QHBoxLayout *hboxLayout;
    QLabel *OrderBy;
    QLabel *Tags;
    QComboBox *tagsCombo;
    QSpacerItem *spacerItem;
    QLabel *Ordering;
    QComboBox *orderingCombo;
    QTreeWidget *fontTree;
    QFrame *line_2;
    QHBoxLayout *hboxLayout1;
    QPushButton *editAllButton;
    QSpacerItem *spacerItem1;
    QPushButton *activateAllButton;
    QPushButton *desactivateAllButton;
    QFrame *viewFrame;
    QGridLayout *gridLayout2;
    QToolBox *toolBox;
    QWidget *pageRender;
    QGridLayout *gridLayout3;
    QPushButton *textButton;
    QSpacerItem *spacerItem2;
    QLabel *label;
    QSlider *renderZoom;
    QGraphicsView *loremView;
    QWidget *page;
    QGridLayout *gridLayout4;
    QSpacerItem *spacerItem3;
    QSpacerItem *spacerItem4;
    QLabel *label_2;
    QSlider *allZoom;
    QGraphicsView *abcView;
    QTextBrowser *glyphInfo;
    QWidget *tagPage;

    void setupUi(QWidget *MainView)
    {
    if (MainView->objectName().isEmpty())
        MainView->setObjectName(QString::fromUtf8("MainView"));
    MainView->resize(963, 773);
    gridLayout = new QGridLayout(MainView);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    manageFrame = new QFrame(MainView);
    manageFrame->setObjectName(QString::fromUtf8("manageFrame"));
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(manageFrame->sizePolicy().hasHeightForWidth());
    manageFrame->setSizePolicy(sizePolicy);
    manageFrame->setMinimumSize(QSize(485, 400));
    manageFrame->setFrameShape(QFrame::StyledPanel);
    manageFrame->setFrameShadow(QFrame::Raised);
    vboxLayout = new QVBoxLayout(manageFrame);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    gridLayout1 = new QGridLayout();
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    search = new QLabel(manageFrame);
    search->setObjectName(QString::fromUtf8("search"));

    vboxLayout1->addWidget(search);

    in = new QLabel(manageFrame);
    in->setObjectName(QString::fromUtf8("in"));

    vboxLayout1->addWidget(in);


    gridLayout1->addLayout(vboxLayout1, 0, 0, 1, 1);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    searchString = new QLineEdit(manageFrame);
    searchString->setObjectName(QString::fromUtf8("searchString"));

    vboxLayout2->addWidget(searchString);

    searchField = new QComboBox(manageFrame);
    searchField->setObjectName(QString::fromUtf8("searchField"));
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(searchField->sizePolicy().hasHeightForWidth());
    searchField->setSizePolicy(sizePolicy1);

    vboxLayout2->addWidget(searchField);


    gridLayout1->addLayout(vboxLayout2, 0, 1, 1, 1);

    vboxLayout3 = new QVBoxLayout();
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    searchButton = new QPushButton(manageFrame);
    searchButton->setObjectName(QString::fromUtf8("searchButton"));
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(searchButton->sizePolicy().hasHeightForWidth());
    searchButton->setSizePolicy(sizePolicy2);
    searchButton->setMinimumSize(QSize(120, 0));
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    font.setWeight(75);
    searchButton->setFont(font);

    vboxLayout3->addWidget(searchButton);


    gridLayout1->addLayout(vboxLayout3, 0, 2, 1, 1);


    vboxLayout->addLayout(gridLayout1);

    line = new QFrame(manageFrame);
    line->setObjectName(QString::fromUtf8("line"));
    QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
    line->setSizePolicy(sizePolicy3);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    vboxLayout->addWidget(line);

    FontInformations = new QLabel(manageFrame);
    FontInformations->setObjectName(QString::fromUtf8("FontInformations"));
    QFont font1;
    font1.setPointSize(12);
    font1.setBold(false);
    font1.setWeight(50);
    font1.setKerning(true);
    FontInformations->setFont(font1);

    vboxLayout->addWidget(FontInformations);

    fontInfoText = new QTextBrowser(manageFrame);
    fontInfoText->setObjectName(QString::fromUtf8("fontInfoText"));
    QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(fontInfoText->sizePolicy().hasHeightForWidth());
    fontInfoText->setSizePolicy(sizePolicy4);

    vboxLayout->addWidget(fontInfoText);

    FontList = new QLabel(manageFrame);
    FontList->setObjectName(QString::fromUtf8("FontList"));
    FontList->setFont(font1);

    vboxLayout->addWidget(FontList);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    OrderBy = new QLabel(manageFrame);
    OrderBy->setObjectName(QString::fromUtf8("OrderBy"));
    QFont font2;
    font2.setPointSize(9);
    font2.setBold(false);
    font2.setWeight(50);
    OrderBy->setFont(font2);

    hboxLayout->addWidget(OrderBy);

    Tags = new QLabel(manageFrame);
    Tags->setObjectName(QString::fromUtf8("Tags"));

    hboxLayout->addWidget(Tags);

    tagsCombo = new QComboBox(manageFrame);
    tagsCombo->setObjectName(QString::fromUtf8("tagsCombo"));
    sizePolicy1.setHeightForWidth(tagsCombo->sizePolicy().hasHeightForWidth());
    tagsCombo->setSizePolicy(sizePolicy1);
    tagsCombo->setContextMenuPolicy(Qt::DefaultContextMenu);

    hboxLayout->addWidget(tagsCombo);

    spacerItem = new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    Ordering = new QLabel(manageFrame);
    Ordering->setObjectName(QString::fromUtf8("Ordering"));

    hboxLayout->addWidget(Ordering);

    orderingCombo = new QComboBox(manageFrame);
    orderingCombo->setObjectName(QString::fromUtf8("orderingCombo"));
    sizePolicy1.setHeightForWidth(orderingCombo->sizePolicy().hasHeightForWidth());
    orderingCombo->setSizePolicy(sizePolicy1);

    hboxLayout->addWidget(orderingCombo);


    vboxLayout->addLayout(hboxLayout);

    fontTree = new QTreeWidget(manageFrame);
    fontTree->setObjectName(QString::fromUtf8("fontTree"));
    QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(fontTree->sizePolicy().hasHeightForWidth());
    fontTree->setSizePolicy(sizePolicy5);
    QFont font3;
    font3.setPointSize(9);
    fontTree->setFont(font3);
    fontTree->viewport()->setProperty("cursor", QVariant(QCursor(Qt::PointingHandCursor)));
    fontTree->setAutoFillBackground(false);
    fontTree->setFrameShape(QFrame::StyledPanel);
    fontTree->setFrameShadow(QFrame::Sunken);
    fontTree->setLineWidth(1);
    fontTree->setTextElideMode(Qt::ElideRight);
    fontTree->setColumnCount(2);

    vboxLayout->addWidget(fontTree);

    line_2 = new QFrame(manageFrame);
    line_2->setObjectName(QString::fromUtf8("line_2"));
    sizePolicy3.setHeightForWidth(line_2->sizePolicy().hasHeightForWidth());
    line_2->setSizePolicy(sizePolicy3);
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);

    vboxLayout->addWidget(line_2);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    editAllButton = new QPushButton(manageFrame);
    editAllButton->setObjectName(QString::fromUtf8("editAllButton"));

    hboxLayout1->addWidget(editAllButton);

    spacerItem1 = new QSpacerItem(186, 26, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem1);

    activateAllButton = new QPushButton(manageFrame);
    activateAllButton->setObjectName(QString::fromUtf8("activateAllButton"));

    hboxLayout1->addWidget(activateAllButton);

    desactivateAllButton = new QPushButton(manageFrame);
    desactivateAllButton->setObjectName(QString::fromUtf8("desactivateAllButton"));
    desactivateAllButton->setAutoExclusive(false);

    hboxLayout1->addWidget(desactivateAllButton);


    vboxLayout->addLayout(hboxLayout1);


    gridLayout->addWidget(manageFrame, 0, 0, 1, 1);

    viewFrame = new QFrame(MainView);
    viewFrame->setObjectName(QString::fromUtf8("viewFrame"));
    QSizePolicy sizePolicy6(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(viewFrame->sizePolicy().hasHeightForWidth());
    viewFrame->setSizePolicy(sizePolicy6);
    viewFrame->setMinimumSize(QSize(350, 500));
    viewFrame->setFrameShape(QFrame::StyledPanel);
    viewFrame->setFrameShadow(QFrame::Raised);
    gridLayout2 = new QGridLayout(viewFrame);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    toolBox = new QToolBox(viewFrame);
    toolBox->setObjectName(QString::fromUtf8("toolBox"));
    pageRender = new QWidget();
    pageRender->setObjectName(QString::fromUtf8("pageRender"));
    pageRender->setGeometry(QRect(0, 0, 432, 640));
    gridLayout3 = new QGridLayout(pageRender);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    textButton = new QPushButton(pageRender);
    textButton->setObjectName(QString::fromUtf8("textButton"));

    gridLayout3->addWidget(textButton, 0, 0, 1, 1);

    spacerItem2 = new QSpacerItem(91, 27, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout3->addItem(spacerItem2, 0, 1, 1, 1);

    label = new QLabel(pageRender);
    label->setObjectName(QString::fromUtf8("label"));
    label->setPixmap(QPixmap(QString::fromUtf8(":/zoom.png")));
    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout3->addWidget(label, 0, 2, 1, 1);

    renderZoom = new QSlider(pageRender);
    renderZoom->setObjectName(QString::fromUtf8("renderZoom"));
    sizePolicy3.setHeightForWidth(renderZoom->sizePolicy().hasHeightForWidth());
    renderZoom->setSizePolicy(sizePolicy3);
    renderZoom->setMaximum(1000);
    renderZoom->setValue(100);
    renderZoom->setTracking(true);
    renderZoom->setOrientation(Qt::Horizontal);
    renderZoom->setTickPosition(QSlider::NoTicks);
    renderZoom->setTickInterval(0);

    gridLayout3->addWidget(renderZoom, 0, 3, 1, 1);

    loremView = new QGraphicsView(pageRender);
    loremView->setObjectName(QString::fromUtf8("loremView"));

    gridLayout3->addWidget(loremView, 1, 0, 1, 5);

    toolBox->addItem(pageRender, QApplication::translate("MainView", "Sample text", 0, QApplication::UnicodeUTF8));
    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));
    page->setGeometry(QRect(0, 0, 112, 238));
    gridLayout4 = new QGridLayout(page);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout4->addItem(spacerItem3, 0, 0, 1, 1);

    spacerItem4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout4->addItem(spacerItem4, 0, 1, 1, 1);

    label_2 = new QLabel(page);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setPixmap(QPixmap(QString::fromUtf8(":/zoom.png")));
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout4->addWidget(label_2, 0, 2, 1, 1);

    allZoom = new QSlider(page);
    allZoom->setObjectName(QString::fromUtf8("allZoom"));
    sizePolicy3.setHeightForWidth(allZoom->sizePolicy().hasHeightForWidth());
    allZoom->setSizePolicy(sizePolicy3);
    allZoom->setMaximum(1000);
    allZoom->setValue(100);
    allZoom->setTracking(false);
    allZoom->setOrientation(Qt::Horizontal);

    gridLayout4->addWidget(allZoom, 0, 3, 1, 1);

    abcView = new QGraphicsView(page);
    abcView->setObjectName(QString::fromUtf8("abcView"));

    gridLayout4->addWidget(abcView, 1, 0, 1, 4);

    glyphInfo = new QTextBrowser(page);
    glyphInfo->setObjectName(QString::fromUtf8("glyphInfo"));

    gridLayout4->addWidget(glyphInfo, 2, 0, 1, 4);

    toolBox->addItem(page, QApplication::translate("MainView", "All glyphs", 0, QApplication::UnicodeUTF8));
    tagPage = new QWidget();
    tagPage->setObjectName(QString::fromUtf8("tagPage"));
    tagPage->setGeometry(QRect(0, 0, 96, 26));
    toolBox->addItem(tagPage, QApplication::translate("MainView", "Tags", 0, QApplication::UnicodeUTF8));

    gridLayout2->addWidget(toolBox, 0, 0, 1, 1);


    gridLayout->addWidget(viewFrame, 0, 1, 1, 1);


    retranslateUi(MainView);

    toolBox->setCurrentIndex(0);


    QMetaObject::connectSlotsByName(MainView);
    } // setupUi

    void retranslateUi(QWidget *MainView)
    {
    MainView->setWindowTitle(QApplication::translate("MainView", "main view widget", 0, QApplication::UnicodeUTF8));
    search->setText(QApplication::translate("MainView", "Search", 0, QApplication::UnicodeUTF8));
    in->setText(QApplication::translate("MainView", "In", 0, QApplication::UnicodeUTF8));
    searchButton->setText(QApplication::translate("MainView", "Search", 0, QApplication::UnicodeUTF8));
    FontInformations->setText(QApplication::translate("MainView", "Font informations:", 0, QApplication::UnicodeUTF8));
    FontList->setText(QApplication::translate("MainView", "Font list:", 0, QApplication::UnicodeUTF8));
    OrderBy->setText(QString());
    Tags->setText(QApplication::translate("MainView", "Tags", 0, QApplication::UnicodeUTF8));
    Ordering->setText(QApplication::translate("MainView", "Ordering", 0, QApplication::UnicodeUTF8));
    fontTree->headerItem()->setText(0, QApplication::translate("MainView", "Names", 0, QApplication::UnicodeUTF8));
    fontTree->headerItem()->setText(1, QApplication::translate("MainView", "Files", 0, QApplication::UnicodeUTF8));
    editAllButton->setText(QApplication::translate("MainView", "Edit All", 0, QApplication::UnicodeUTF8));
    activateAllButton->setText(QApplication::translate("MainView", "Activate all", 0, QApplication::UnicodeUTF8));
    desactivateAllButton->setText(QApplication::translate("MainView", "Deactivate all", 0, QApplication::UnicodeUTF8));
    textButton->setText(QApplication::translate("MainView", "Sample text...", 0, QApplication::UnicodeUTF8));
    label->setText(QString());
    renderZoom->setToolTip(QApplication::translate("MainView", "zoom", "zoom", QApplication::UnicodeUTF8));
    renderZoom->setStatusTip(QApplication::translate("MainView", "zoom", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(pageRender), QApplication::translate("MainView", "Sample text", 0, QApplication::UnicodeUTF8));
    label_2->setText(QString());
    toolBox->setItemText(toolBox->indexOf(page), QApplication::translate("MainView", "All glyphs", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(tagPage), QApplication::translate("MainView", "Tags", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MainView);
    } // retranslateUi

};

namespace Ui {
    class MainView: public Ui_MainView {};
} // namespace Ui

#endif // UI_MAINVIEW_H
