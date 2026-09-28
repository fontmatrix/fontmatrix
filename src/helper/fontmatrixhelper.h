/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FONTMATRIXHELPER_H
#define FONTMATRIXHELPER_H

#include <KAuth/ActionReply>

#include <QObject>
#include <QVariantMap>

/**
 * The root helper of the activation for all users, run by KAuth once polkit
 * has authorized the action. It does three things and nothing else: copies
 * font files into /usr/local/share/fonts/fontmatrix, removes copies from
 * there, and writes the rejects file of /etc/fonts/conf.d. Every path it is
 * given is checked: a copy is named by a file name only, its source must be
 * a regular file everybody may read (a system font is one), a file to hide
 * must exist.
 */
class FontmatrixHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    /// "sources": the files, "names": the file names of the copies; reply "copied", "failed": the names
    KAuth::ActionReply activate(const QVariantMap &args);
    /// "names": the file names of the copies to remove; reply "removed", "failed"
    KAuth::ActionReply deactivate(const QVariantMap &args);
    /// "files": the fonts to hide from everybody; none removes the rejects file
    KAuth::ActionReply rejects(const QVariantMap &args);
};

#endif
