/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmfontextract.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QFileDialog>
#include <QFileInfo>

#ifdef HAVE_PODOFO
#include "fmpdffontextractor.h"
#endif

FMFontExtract::FMFontExtract(QWidget *parent)
    : QDialog(parent)
    , lastPath(QDir::homePath())
    , lastDir(QDir::homePath())
{
    setupUi(this);
    currentExtractor = nullptr;
#ifdef HAVE_PODOFO
    auto pdfExtr(new FMPDFFontExtractor);
    for (const auto extensionsList = pdfExtr->extensions(); const auto &e : extensionsList) {
        extractors[e] = pdfExtr;
    }
#endif

    docPath->clear();

    connect(browsePDF, &QPushButton::clicked, this, &FMFontExtract::slotBrowseDoc);
    connect(browseDir, &QPushButton::clicked, this, &FMFontExtract::slotBrowseDir);
    connect(extractButton, &QPushButton::clicked, this, &FMFontExtract::slotExtract);
}

FMFontExtract::~FMFontExtract()
{
    QList<FMFontExtractorBase *> extP;
    for (const auto extractorsValues = extractors.values(); auto *b : extractorsValues) {
        if (!extP.contains(b))
            extP << b;
    }
    for (auto *b : std::as_const(extP)) {
        if (b)
            delete b;
    }
}

void FMFontExtract::loadDoc(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists()) {
        docPath->setText(i18nc("@info:status", "File does not exist:") + " " + fi.fileName());
        return;
    }

    QString suffix(fi.suffix());

    if (extractors.contains(suffix)) {
        currentExtractor = extractors.value(suffix);
        fontList->clear();
        if (currentExtractor->loadFile(path)) {
            for (const auto listRange = currentExtractor->list(); const auto &n : listRange) {
                fontList->addItem(n);
            }
        }
    } else {
        docPath->setText(i18nc("@info:status", "Format not handled."));
    }

    for (int i(0); i < fontList->count(); ++i) {
        fontList->item(i)->setCheckState(Qt::Unchecked);
    }
}

void FMFontExtract::slotBrowseDoc()
{
    // 	QString filters( "Portable Document Format (*.pdf *.PDF)" );
    QString path(QFileDialog::getOpenFileName(this, "Fontmatrix", lastPath));
    if (path.isEmpty())
        return;
    lastPath = path;
    docPath->setText(path);
    loadDoc(path);
}

void FMFontExtract::slotBrowseDir()
{
    QString dirpath(QFileDialog::getExistingDirectory(this, "Fontmatrix", lastDir));
    if (dirpath.isEmpty())
        return;
    lastDir = dirpath;
    outputDir->setText(dirpath);
}

void FMFontExtract::slotExtract()
{
    if (fontList->count() == 0)
        return;

    QStringList names;
    for (int i(0); i < fontList->count(); ++i) {
        if (fontList->item(i)->checkState() == Qt::Checked)
            names << fontList->item(i)->text();
    }
    QString odir(outputDir->text() + QDir::separator());

    QStringList failedExt;
    for (const auto &name : std::as_const(names)) {
        QString fnam(odir + name + "." + currentExtractor->fontType(name));
        if (QFile::exists(fnam))
            QFile::remove(fnam);
        QFile f(fnam);
        if (f.open(QIODevice::WriteOnly)) {
            if (!currentExtractor->write(name, &f))
                failedExt << name;
        }
    }
    if (!failedExt.isEmpty()) {
        KMessageBox::error(this, i18nc("@info", "Failed to extract:\n%1", failedExt.join("\n")));
    }
}

#include "moc_fmfontextract.cpp"
