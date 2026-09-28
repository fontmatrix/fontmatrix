/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMACTIVATE_H
#define FMACTIVATE_H

#include <QHash>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariantMap>

class FontItem;

class FMActivate : public QObject
{
    Q_OBJECT

    FMActivate();
    static FMActivate *instance;

    // no name here may be a macro of windows.h: wingdi.h has ERROR and SYSTEM_FONT
    enum Error {
        ALREADY_ACTIVE = 0,
        NO_UNLINK,
        ALREADY_UNACTIVE,
        MISSING_AFM,
        OTHER_ERROR,
        // Windows
        NO_COPY,
        NO_REGISTRY,
        NO_FONT_RESOURCE,
        LOCKED_FONT,
        UNSUPPORTED_FORMAT,
        // Linux, for all users
        NO_AUTHORIZATION
    };

    QHash<Error, QString> errorStrings;
    void setErrorStrings();

public:
    static FMActivate *getInstance();

    //		void activate(FontItem* fit , bool act );
    void activate(QList<FontItem *> fitList, bool act);
    QMap<QString, QString> errors();

#ifdef _WIN32
    /**
     * Brings the database in line with the per-user font folder and the
     * registry, once per start: a font whose copy or registry value has gone
     * (removed in Settings > Fonts) is flagged inactive, a registry value that
     * points to a file that has gone is deleted, and a copy that could not be
     * deleted at deactivation because it was in use is deleted now.
     */
    void reconcileUserFonts();
#elif !defined(PLATFORM_APPLE)
    /**
     * Once, from the versions that linked the activated fonts into a private
     * directory named in the user's fonts.conf and hid system fonts with
     * rejectfont entries there: the fonts flagged active get their copies,
     * the directory goes, and fonts.conf loses that directory and those
     * entries, which become the flags of the fonts and the rejects file.
     */
    void migrateActivated(const QString &oldDir);
    /**
     * Once per start: a font flagged active whose copy has gone is flagged
     * inactive, a file of the folder that belongs to no active font goes, and
     * the rejects file is brought in line with the flags of the system fonts.
     */
    void reconcileActivated();

    /**
     * Whether fonts can be activated for all users of the computer: built with
     * KAuth, not inside a Flatpak, and the polkit action of the helper installed.
     */
    static bool systemScopeAvailable();
    /// the preference: activate for all users (a copy in /usr/local/share/fonts/fontmatrix) rather than for oneself
    static bool systemScope();
    /**
     * Sets the preference and moves the hidden system fonts with it: the
     * rejects file of the new scope is written from the flags, the other one
     * removed. Copies stay where they are. False when the helper refused.
     */
    bool setSystemScope(bool system);
#endif

Q_SIGNALS:
    void activationEvent(const QStringList &);

private:
#if !defined(_WIN32) && !defined(PLATFORM_APPLE)
    /**
     * Writes the fontconfig file that hides the system fonts switched off:
     * every system font whose flag is off, as a rejectfont pattern on its
     * file. With none, the file goes. Written only when it would change.
     */
    void writeRejects();
    /// the system fonts whose flag is off
    [[nodiscard]] static QStringList hiddenSystemFonts();
    /// the copy of the font, in the user's folder or the system's; empty when there is none
    [[nodiscard]] static QString copyOf(FontItem *fit);
    /**
     * Runs an action of the root helper (activate, deactivate, rejects) with
     * the arguments, after polkit's authorization. False with the reason when
     * it was refused or failed.
     */
    static bool runHelper(const QString &action, const QVariantMap &args, QVariantMap *reply, QString *error);
    void writeSystemRejects(bool *ok);
#endif

    QMap<QString, QString> m_errors;
};

#endif
