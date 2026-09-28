/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fontbookdialog.h"
#include "fmfontdb.h"
#include "fmpaths.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "typotek.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QDebug>
#include <QDomNodeList>
#include <QFileDialog>
#include <QGraphicsScene>

FontBookDialog::FontBookDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    isOk = false;
    m_isTemplate = false;
    // 	loadTemplateButton->setVisible(false);
    // 	templateLabel->setVisible(false);
    curTemplatePreview = nullptr;

    // 	fillSizeList();
    fillFontsList();
    // 	QString alorem ( "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nUt a sapien. Aliquam aliquet purus molestie dolor.\nInteger quis eros ut
    // erat posuere dictum. Curabitur dignissim.\nInteger orci. Fusce vulputate lacus at ipsum. \nQuisque in libero nec mi laoreet volutpat." ); 	QString
    // loremBig ( "LOREM IPSUM DOLOR" ); 	setSampleHeadline ( loremBig ); 	setSampleText ( alorem );

    // 	m_pageRect = QRectF(0,0,m_pageSize.width(), m_pageSize.height());
    // 	preScene = new QGraphicsScene(m_pageRect);
    // // 	preView->setScene(preScene);
    // 	preView->setRenderHint ( QPainter::Antialiasing, true );
    // 	preView->setBackgroundBrush(Qt::lightGray);

    templateScene = new QGraphicsScene();
    templateScene->setBackgroundBrush(Qt::lightGray);
    templatePreview->setScene(templateScene);

    fillTemplates();

    // 	slotPreview();

    connect(okButton, &QDialogButtonBox::accepted, this, &FontBookDialog::slotAccept);
    connect(okButton, &QDialogButtonBox::rejected, this, &FontBookDialog::slotCancel);
    connect(fileNameButton, &QPushButton::released, this, &FontBookDialog::slotFileDialog);
    // 	connect ( paperSizeCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPageSize ( int ) ) );
    // 	connect(this,SIGNAL(updateView()),this,SLOT(slotPreview()));

    // 	QList<QSpinBox*> spinList;
    // 	spinList << familySpinBox;
    // 	spinList << styleSpinBox;
    // 	spinList << sampleSpinBox;
    // 	spinList << familyFontSizeSpin;
    // 	spinList << styleFontSizeSpin;
    // 	spinList << headlineFontSizeSpin;
    // 	spinList << bodyFontSizeSpin;
    // 	for (auto* sp : spinList)
    // 	{
    // 		connect(sp,SIGNAL(valueChanged ( int  )),this,SIGNAL(updateView()));
    // 	}
    // 	connect (sampleTextEdit,SIGNAL(textChanged()),SIGNAL(updateView()));
    // 	connect (sampleHeadline,SIGNAL(textChanged( const QString&)),SIGNAL(updateView()));

    // 	connect(loadTemplateButton,SIGNAL(released()),this,SLOT(slotLoadTemplate()));

    connect(templatesList, &QListWidget::currentTextChanged, this, &FontBookDialog::slotPreviewTemplate);
}

FontBookDialog::~FontBookDialog() = default;

void FontBookDialog::slotAccept()
{
    isOk = true;
    close();
}

void FontBookDialog::slotCancel()
{
    isOk = false;
    close();
}

void FontBookDialog::slotFileDialog()
{
    QString theFile = QFileDialog::getSaveFileName(this, i18nc("@title:window", "Save fontBook"), QDir::homePath(), "Portable Document Format (*.pdf)");
    fileNameEdit->setText(theFile);
}

void FontBookDialog::fillFontsList()
{
    QList<FontItem *> localFontMap = FMFontDb::DB()->getFilteredFonts();
    for (auto *fit : std::as_const(localFontMap)) {
        selectedFontsList->addItem(fit->fancyName());
    }
}

QString FontBookDialog::getFileName()
{
    return fileNameEdit->text();
}

/**
 *	1 - browse to select a template file
 *	2 - load as a QDomDocument
 *	3 - search for "description" and "preview" elements
 *	4 - check validity of the doc (will be hard at the beginning)
 */
void FontBookDialog::slotLoadTemplate(const QString &theTemplate)
{
    // 	QString theTemplate = QFileDialog::getOpenFileName ( this, "Get template", QDir::homePath(), i18n("Templates (*.xml)"));
    qCDebug(FONTMATRIX_LOG) << "FontBookDialog::slotLoadTemplate(" << theTemplate << ") -> " << templatesMap[theTemplate];
    if (theTemplate.isEmpty())
        return;

    QFile file(templatesMap.value(theTemplate));
    QDomDocument doc("template");
    if (!file.open(QFile::ReadOnly)) {
        KMessageBox::error(this, i18nc("@info", "Cannot read %1.", file.fileName()));
        return;
    }
    if (!doc.setContent(&file)) {
        file.close();
        KMessageBox::error(this, i18nc("@info", "%1 is not a valid XML file.", file.fileName()));
        return;
    }
    file.close();

    m_template = doc;
    m_isTemplate = true;
}

void FontBookDialog::fillTemplates()
{
    QDir tDir(typotek::getInstance()->getTemplatesDir());
    QStringList filters;
    filters << "*.xml";
    tDir.setNameFilters(filters);
    QStringList pathList = tDir.entryList();
    for (int i = 0; i < pathList.count(); ++i) {
        QFile file(tDir.absoluteFilePath(pathList.at(i)));
        QDomDocument doc("template");
        if (!file.open(QFile::ReadOnly)) {
            KMessageBox::error(this, i18nc("@info", "Cannot read %1.", file.fileName()));
            return;
        }
        if (!doc.setContent(&file)) {
            file.close();
            KMessageBox::error(this, i18nc("@info", "%1 is not a valid XML file.", file.fileName()));
            return;
        }
        file.close();

        QString description;
        QDomNodeList descList = doc.elementsByTagName("name");
        if (descList.length()) {
            QDomNode node = descList.item(0);
            description = node.toElement().text();
        }
        QString preview;
        QDomNodeList prevList = doc.elementsByTagName("preview");
        if (descList.length()) {
            QDomNode node = prevList.item(0);
            preview = tDir.absoluteFilePath(node.toElement().text());
        }

        if (description.isEmpty())
            continue;

        templatesMap[description] = tDir.absoluteFilePath(pathList.at(i));

        if (!preview.isEmpty()) {
            templatesPreviewMap[description] = QPixmap(preview);
        }
    }
    // Here we insert default templates provided by Fontmatrix
    templatesMap["Default template"] = FMPaths::ResourcesDir() + "template_default.xml";
    templatesPreviewMap["Default template"] = QPixmap(FMPaths::ResourcesDir() + "template_default.png");
    templatesMap["Default template (oneliner)"] = FMPaths::ResourcesDir() + "template_oneline.xml";
    templatesPreviewMap["Default template (oneliner)"] = QPixmap(FMPaths::ResourcesDir() + "template_oneline.png");

    templatesList->addItems(templatesMap.keys());
}

void FontBookDialog::slotPreviewTemplate(const QString &key)
{
    qCDebug(FONTMATRIX_LOG) << "slotPreviewTemplate(" << key << ") -> " << templatesMap[key];
    if (templatesMap.contains(key)) {
        if (!templatesPreviewMap[key].isNull()) {
            if (curTemplatePreview)
                templateScene->removeItem(curTemplatePreview);
            delete curTemplatePreview;
            // ### memleak
            curTemplatePreview = templateScene->addPixmap(templatesPreviewMap[key]);
            // 			templatePreview->ensureVisible(curTemplatePreview,10,10);
        }

        slotLoadTemplate(key);
    }
}

#include "moc_fontbookdialog.cpp"
