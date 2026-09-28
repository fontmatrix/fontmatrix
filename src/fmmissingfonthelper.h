/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMMISSINGFONTHELPER_H
#define FMMISSINGFONTHELPER_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
  Let's try to nicely handle missing font files.
  At first, just popup the "check database" dialog.
*/
class FMMissingFontHelper : public QObject
{
    FMMissingFontHelper() = default;

public:
    explicit FMMissingFontHelper(const QString &ff);
    explicit FMMissingFontHelper(const QStringList &ff);
};

#endif // FMMISSINGFONTHELPER_H
