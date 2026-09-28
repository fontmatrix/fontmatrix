/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// A dead simple font extractor from PDF docs

#ifndef FMPDFFONTEXTRACTOR
#define FMPDFFONTEXTRACTOR

#include <QMap>
#include <memory>

#include "fmfontextractorbase.h"

#include <podofo/podofo.h>

class FMPDFFontExtractor : public FMFontExtractorBase
{
public:
    FMPDFFontExtractor();
    ~FMPDFFontExtractor() override;

    bool loadFile(const QString &filePath) override;
    QStringList extensions() override;

    QStringList list() override;
    QString fontType(const QString &name) override;
    bool write(const QString &name, QIODevice *openedDevice) override;

private:
    bool cachedList;
    std::unique_ptr<PoDoFo::PdfMemDocument> document;
    QMap<QString, PoDoFo::PdfObject *> mfont;
    QMap<QString, QString> mType;
};

#endif // FMPDFFONTEXTRACTOR
