/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmactivate.h"
#include "fontmatrix_debug.h"

#include "fmfontdb.h"
#include "fontitem.h"
#include "typotek.h"

#include <KLocalizedString>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QFile>
#include <QTextStream>

FMActivate *FMActivate::instance = nullptr;

FMActivate::FMActivate()
{
    setErrorStrings();
}

void FMActivate::setErrorStrings()
{
    //: The Font asked for activation is already activated
    errorStrings[ALREADY_ACTIVE] = i18nc("@info activation error", "Font already activated");
    //: The copy of the font in the user's font folder could not be deleted
    errorStrings[NO_UNLINK] = i18nc("@info activation error", "Unable to remove the font from the user font folder");
    //: The Font asked for de-activation is already de-activated
    errorStrings[ALREADY_UNACTIVE] = i18nc("@info activation error", "Font already de-activated");
    //: A postcript font (pfb) without its metrics file (afm)
    errorStrings[MISSING_AFM] = i18nc("@info activation error", "Cannot copy the metrics file");
    //: A generic error in activation or deactivation process
    errorStrings[OTHER_ERROR] = i18nc("activation", "Error");
    //: The font file could not be copied into the user's font folder
    errorStrings[NO_COPY] = i18nc("@info activation error", "Unable to copy the font file into the user font folder");
    //: Windows: the font could not be written to or removed from the user's font registry key
    errorStrings[NO_REGISTRY] = i18nc("@info activation error", "Unable to register the font for this user");
    //: Windows: AddFontResource failed, applications will see the font at the next logon only
    errorStrings[NO_FONT_RESOURCE] = i18nc("@info activation error", "The font is installed but could not be loaded for running applications");
    //: Windows: the font is installed for all users or by the user in Settings, Fontmatrix cannot change it
    errorStrings[LOCKED_FONT] = i18nc("@info activation error", "The font is installed by the system and cannot be changed here");
    //: Windows: only TrueType and OpenType fonts can be installed per user
    errorStrings[UNSUPPORTED_FORMAT] = i18nc("@info activation error", "Only TrueType and OpenType fonts can be activated for this user");
    //: Linux: polkit refused, or the user cancelled the password dialog, the activation for all users
    errorStrings[NO_AUTHORIZATION] = i18nc("@info activation error", "Not authorized to change the fonts of all users");
}

FMActivate *FMActivate::getInstance()
{
    if (!instance) {
        instance = new FMActivate;
        Q_ASSERT(instance);
    }
    return instance;
}

#ifdef PLATFORM_APPLE

void FMActivate::activate(QList<FontItem *> fitList, bool act)
{
    // TODO insert error messages.
    QHash<FontItem *, bool> stack;
    typotek *T(typotek::getInstance());
    for (auto *fit : fitList) {
        qCDebug(FONTMATRIX_LOG) << "Activation of " << fit->path() << act;
        if (act) // Activation
        {
            if (!T->isSysFont(fit)) {
                if (!fit->isActivated()) {
                    fit->setActivated(true);

                    // 				QFileInfo fofi ( fit->path() );

                    if (!QFile::copy(fit->localPath(), T->getManagedDir() + "/" + fit->activationName())) {
                        qCWarning(FONTMATRIX_LOG) << "unable to copy " << fit->path();
                    } else {
                        // Success
                        stack[fit] = true;
                        qCDebug(FONTMATRIX_LOG) << fit->path() << " copied";
                        if (!fit->afm().isEmpty()) {
                            // 						QFileInfo afm ( fit->afm() );
                            if (!QFile::copy(fit->afm(), T->getManagedDir() + "/" + fit->activationAFMName())) {
                                qCWarning(FONTMATRIX_LOG) << "unable to copy " << fit->afm();
                            } else {
                                qCDebug(FONTMATRIX_LOG) << fit->afm() << "copied";
                            }
                        } else {
                            qCDebug(FONTMATRIX_LOG) << "There is no AFM file attached to " << fit->path();
                        }
                    }
                } else {
                    qCDebug(FONTMATRIX_LOG) << "\tYet activated";
                }

            } else {
                qCDebug(FONTMATRIX_LOG) << "\tIs Locked";
            }

        } else // Deactivation
        {
            if (!T->isSysFont(fit)) {
                if (fit->isActivated()) {
                    fit->setActivated(false);
                    // 				QFileInfo fofi ( fit->path() );
                    if (!QFile::remove(T->getManagedDir() + "/" + fit->activationName())) {
                        qCWarning(FONTMATRIX_LOG) << "unable to unlink " << fit->name();
                    } else {
                        // Success
                        stack[fit] = false;
                        if (!fit->afm().isEmpty()) {
                            // 						QFileInfo afm ( fit->afm() );
                            if (!QFile::remove(T->getManagedDir() + "/" + fit->activationAFMName())) {
                                qCWarning(FONTMATRIX_LOG) << "unable to unlink " << fit->afm();
                            }
                        }
                        // 					typo->adaptator()->private_signal ( 0, fofi.fileName() );
                    }
                }

            } else {
                qCDebug(FONTMATRIX_LOG) << "\tIs Locked";
            }
        }
    }

    QStringList aList;
    FMFontDb::DB()->TransactionBegin();
    for (auto it(stack.constBegin()); it != stack.constEnd(); ++it) {
        it.key()->setActivated(it.value());
        aList << it.key()->path();
    }
    FMFontDb::DB()->TransactionEnd();

    Q_EMIT activationEvent(aList);
}

#elif defined(_WIN32)

/*
    A font is activated for the current user the way Settings > Fonts installs
    one: a copy in %LOCALAPPDATA%\Microsoft\Windows\Fonts (managedDir), a REG_SZ
    value under HKCU\Software\Microsoft\Windows NT\CurrentVersion\Fonts whose
    name is the full font name plus "(TrueType)" or "(OpenType)" and whose data
    is the path of the copy, which Windows loads at every logon, and
    AddFontResource() plus WM_FONTCHANGE so that running applications see it now.
    Deactivation takes the three away in the opposite order. Nothing needs
    administrator rights.
*/

// clang-format off: windows.h has to come first
#define NOMINMAX
#include <windows.h>
// clang-format on

#include "fmfreetypelib.h"

#include <QByteArrayView>
#include <QDir>
#include <QFileInfo>
#include <QMultiHash>
#include <QStringDecoder>
#include <QThread>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

#include <string>
#include <vector>

namespace
{
constexpr wchar_t userFontsKeyPath[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

/// the path as the Win32 API and the registry take it
std::wstring nativePath(const QString &path)
{
    return QDir::toNativeSeparators(QDir::cleanPath(path)).toStdWString();
}

/// paths are compared without their case and with either separator
bool samePath(const QString &a, const QString &b)
{
    return QDir::cleanPath(QDir::fromNativeSeparators(a)).compare(QDir::cleanPath(QDir::fromNativeSeparators(b)), Qt::CaseInsensitive) == 0;
}

bool isInside(const QString &path, const QString &dir)
{
    const QString p(QDir::cleanPath(QDir::fromNativeSeparators(path)));
    const QString d(QDir::cleanPath(QDir::fromNativeSeparators(dir)) + QLatin1Char('/'));
    return p.startsWith(d, Qt::CaseInsensitive);
}

/**
 * The full name (name ID 4) of every face of the file, the English one when
 * there is one. The file is read into memory: FT_New_Face() wants an 8-bit
 * path, which cannot name every folder on Windows.
 */
QStringList fullNames(const QString &path)
{
    QStringList names;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return names;
    const QByteArray data(f.readAll());
    FT_Library lib = FMFreetypeLib::lib(QThread::currentThread());
    FT_Long faceCount = 1;
    for (FT_Long i = 0; i < faceCount; ++i) {
        FT_Face face = nullptr;
        if (FT_New_Memory_Face(lib, reinterpret_cast<const FT_Byte *>(data.constData()), static_cast<FT_Long>(data.size()), i, &face))
            break;
        faceCount = face->num_faces;
        QString english;
        QString other;
        const FT_UInt count = FT_Get_Sfnt_Name_Count(face);
        for (FT_UInt n = 0; n < count && english.isEmpty(); ++n) {
            FT_SfntName sn;
            if (FT_Get_Sfnt_Name(face, n, &sn) != 0 || sn.name_id != TT_NAME_ID_FULL_NAME || sn.string_len == 0)
                continue;
            const QByteArrayView bytes(reinterpret_cast<const char *>(sn.string), sn.string_len);
            QString value;
            if (sn.platform_id == TT_PLATFORM_MICROSOFT) {
                // every Microsoft name record is UTF-16BE
                value = QString(QStringDecoder(QStringDecoder::Utf16BE)(bytes)).trimmed();
            } else if (sn.platform_id == TT_PLATFORM_MACINTOSH && sn.encoding_id == TT_MAC_ID_ROMAN) {
                value = QString::fromLatin1(bytes).trimmed();
            }
            if (value.isEmpty())
                continue;
            if (sn.platform_id == TT_PLATFORM_MICROSOFT && sn.language_id == TT_MS_LANGID_ENGLISH_UNITED_STATES)
                english = value;
            else if (other.isEmpty())
                other = value;
        }
        const QString name(english.isEmpty() ? other : english);
        if (!name.isEmpty() && !names.contains(name))
            names << name;
        FT_Done_Face(face);
    }
    return names;
}

/**
 * The name of the registry value, as Settings > Fonts writes it: the full font
 * name, the names of a collection joined by " & ", and the format in
 * parentheses. Empty for a format Windows does not install per user.
 */
QString registryValueName(FontItem *fit)
{
    QString suffix;
    if (fit->type() == QLatin1String("TrueType"))
        suffix = QStringLiteral(" (TrueType)");
    else if (fit->type() == QLatin1String("CFF"))
        suffix = QStringLiteral(" (OpenType)");
    else
        return QString();
    QStringList names(fullNames(fit->localPath()));
    if (names.isEmpty())
        names << fit->fancyName().trimmed();
    return names.join(QLatin1String(" & ")) + suffix;
}

/// The per-user font key, opened for the lifetime of the object.
class UserFontsKey
{
public:
    explicit UserFontsKey(bool write)
    {
        const REGSAM access = KEY_QUERY_VALUE | (write ? KEY_SET_VALUE : 0);
        LSTATUS status;
        if (write) {
            // the key does not exist until the first per-user font is installed
            status = RegCreateKeyExW(HKEY_CURRENT_USER, userFontsKeyPath, 0, nullptr, REG_OPTION_NON_VOLATILE, access, nullptr, &m_key, nullptr);
        } else {
            status = RegOpenKeyExW(HKEY_CURRENT_USER, userFontsKeyPath, 0, access, &m_key);
        }
        if (status != ERROR_SUCCESS) {
            qCWarning(FONTMATRIX_LOG) << "Cannot open the user fonts registry key, error" << status;
            m_key = nullptr;
        }
    }
    ~UserFontsKey()
    {
        if (m_key)
            RegCloseKey(m_key);
    }
    Q_DISABLE_COPY(UserFontsKey)

    [[nodiscard]] bool isOpen() const
    {
        return m_key != nullptr;
    }

    /// value name -> font path, for every string value of the key
    [[nodiscard]] QMap<QString, QString> values() const
    {
        QMap<QString, QString> ret;
        if (!m_key)
            return ret;
        DWORD maxNameChars = 0;
        DWORD maxDataBytes = 0;
        if (RegQueryInfoKeyW(m_key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &maxNameChars, &maxDataBytes, nullptr, nullptr)
            != ERROR_SUCCESS)
            return ret;
        std::vector<wchar_t> name(maxNameChars + 1);
        std::vector<BYTE> data(maxDataBytes + sizeof(wchar_t));
        for (DWORD i = 0;; ++i) {
            DWORD nameChars = static_cast<DWORD>(name.size());
            DWORD dataBytes = static_cast<DWORD>(data.size());
            DWORD type = 0;
            const LSTATUS status = RegEnumValueW(m_key, i, name.data(), &nameChars, nullptr, &type, data.data(), &dataBytes);
            if (status == ERROR_NO_MORE_ITEMS)
                break;
            if (status != ERROR_SUCCESS || type != REG_SZ)
                continue;
            QString path(QString::fromWCharArray(reinterpret_cast<const wchar_t *>(data.data()), static_cast<int>(dataBytes / sizeof(wchar_t))));
            while (path.endsWith(QChar(0)))
                path.chop(1);
            ret.insert(QString::fromWCharArray(name.data(), static_cast<int>(nameChars)), path);
        }
        return ret;
    }

    bool set(const QString &name, const QString &path)
    {
        const std::wstring n(name.toStdWString());
        const std::wstring d(nativePath(path));
        const LSTATUS status =
            RegSetValueExW(m_key, n.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE *>(d.c_str()), static_cast<DWORD>((d.size() + 1) * sizeof(wchar_t)));
        if (status != ERROR_SUCCESS)
            qCWarning(FONTMATRIX_LOG) << "Cannot write the registry value" << name << ", error" << status;
        return status == ERROR_SUCCESS;
    }

    bool remove(const QString &name)
    {
        const LSTATUS status = RegDeleteValueW(m_key, name.toStdWString().c_str());
        if (status != ERROR_SUCCESS)
            qCWarning(FONTMATRIX_LOG) << "Cannot delete the registry value" << name << ", error" << status;
        return status == ERROR_SUCCESS;
    }

private:
    HKEY m_key = nullptr;
};

/**
 * Writes the value that makes Windows load the copy at logon. When another
 * font already holds the name, the copy's file name is added to it, so that
 * the other font keeps its value.
 */
bool registerUserFont(const QString &path, const QString &valueName)
{
    UserFontsKey key(true);
    if (!key.isOpen())
        return false;
    QString name(valueName);
    const QMap<QString, QString> values(key.values());
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        if (it.key().compare(valueName, Qt::CaseInsensitive) == 0 && !samePath(it.value(), path)) {
            name.insert(name.lastIndexOf(QLatin1String(" (")), QStringLiteral(" (%1)").arg(QFileInfo(path).fileName()));
            break;
        }
    }
    return key.set(name, path);
}

/// Deletes every value that points to the copy. Nothing to delete is a success.
bool unregisterUserFont(const QString &path)
{
    UserFontsKey key(true);
    if (!key.isOpen())
        return false;
    bool ok = true;
    const QMap<QString, QString> values(key.values());
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        if (samePath(it.value(), path))
            ok = key.remove(it.key()) && ok;
    }
    return ok;
}

/// Tells every window that the font table changed. Notify, not send: a hung
/// application must not hang Fontmatrix.
void broadcastFontChange()
{
    SendNotifyMessageW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
}
} // namespace

void FMActivate::activate(QList<FontItem *> fitList, bool act)
{
    QHash<FontItem *, bool> stack;
    typotek *T(typotek::getInstance());
    const QString managed(T->getManagedDir());
    bool fontTableChanged = false;
    for (auto *fit : fitList) {
        if (act) // Activation
        {
            if (fit->isRemote() && !fit->isCached()) {
                // the file comes first; this font is activated on its own when it is here
                connect(
                    fit,
                    &FontItem::downloadFinished,
                    this,
                    [this, fit](bool ok) {
                        if (ok)
                            activate(QList<FontItem *>() << fit, true);
                    },
                    Qt::SingleShotConnection);
                fit->getFromNetwork();
                continue;
            }
            if (T->isSysFont(fit) || isInside(fit->localPath(), managed)) {
                // installed for everybody, or by the user in Settings > Fonts
                m_errors[fit->path()] = errorStrings[LOCKED_FONT];
                continue;
            }
            if (fit->isActivated()) {
                m_errors[fit->path()] = errorStrings[ALREADY_ACTIVE];
                continue;
            }
            const QString valueName(registryValueName(fit));
            if (valueName.isEmpty()) {
                m_errors[fit->path()] = errorStrings[UNSUPPORTED_FORMAT];
                continue;
            }
            const QString copy(managed + QLatin1Char('/') + fit->activationName());
            // a copy may be left from a deactivation while the font was in use
            if (!QFileInfo::exists(copy) && !QFile::copy(fit->localPath(), copy)) {
                qCWarning(FONTMATRIX_LOG) << "unable to copy" << fit->localPath() << "to" << copy;
                m_errors[fit->path()] = errorStrings[NO_COPY];
                continue;
            }
            if (!registerUserFont(copy, valueName)) {
                m_errors[fit->path()] = errorStrings[NO_REGISTRY];
                QFile::remove(copy);
                continue;
            }
            stack[fit] = true;
            fontTableChanged = true;
            qCDebug(FONTMATRIX_LOG) << fit->path() << "installed as" << valueName;
            // Windows loads the copy at the next logon anyway
            if (AddFontResourceW(nativePath(copy).c_str()) == 0) {
                qCWarning(FONTMATRIX_LOG) << "AddFontResource failed for" << copy;
                m_errors[fit->path()] = errorStrings[NO_FONT_RESOURCE];
            }
        } else // Deactivation
        {
            if (T->isSysFont(fit) || isInside(fit->localPath(), managed)) {
                m_errors[fit->path()] = errorStrings[LOCKED_FONT];
                continue;
            }
            if (!fit->isActivated()) {
                m_errors[fit->path()] = errorStrings[ALREADY_UNACTIVE];
                continue;
            }
            const QString copy(managed + QLatin1Char('/') + fit->activationName());
            // the value first: without it the font is gone at the next logon whatever happens to the file
            if (!unregisterUserFont(copy)) {
                m_errors[fit->path()] = errorStrings[NO_REGISTRY];
                continue;
            }
            stack[fit] = false;
            fontTableChanged = true;
            RemoveFontResourceW(nativePath(copy).c_str());
            // an application that uses the font keeps the file open; reconcileUserFonts() deletes it at the next start
            if (QFileInfo::exists(copy) && !QFile::remove(copy)) {
                qCWarning(FONTMATRIX_LOG) << "unable to delete" << copy;
                m_errors[fit->path()] = errorStrings[NO_UNLINK];
            }
        }
    }

    QStringList aList;
    FMFontDb::DB()->TransactionBegin();
    for (auto it(stack.constBegin()); it != stack.constEnd(); ++it) {
        it.key()->setActivated(it.value());
        aList << it.key()->path();
    }
    FMFontDb::DB()->TransactionEnd();

    if (fontTableChanged)
        broadcastFontChange();
    Q_EMIT activationEvent(aList);
}

void FMActivate::reconcileUserFonts()
{
    typotek *T(typotek::getInstance());
    const QString managed(T->getManagedDir());
    UserFontsKey key(true);
    if (!key.isOpen())
        return;
    QMap<QString, QString> values(key.values());

    // values of Fontmatrix's copies whose file has gone
    for (auto it = values.begin(); it != values.end();) {
        if (isInside(it.value(), managed) && !QFileInfo::exists(QDir::fromNativeSeparators(it.value()))) {
            qCDebug(FONTMATRIX_LOG) << "registry value without a file:" << it.key();
            key.remove(it.key());
            it = values.erase(it);
        } else {
            ++it;
        }
    }
    const auto isRegistered = [&values](const QString &path) {
        for (const QString &registered : std::as_const(values)) {
            if (samePath(registered, path))
                return true;
        }
        return false;
    };

    // the flag of the database against the folder and the registry
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    QMultiHash<QString, FontItem *> byFileName;
    QHash<FontItem *, bool> stack;
    for (FontItem *fit : fonts) {
        if (T->isSysFont(fit))
            continue;
        const QString local(fit->localPath());
        if (isInside(local, managed)) {
            // installed by the user in Settings > Fonts: active as long as it is registered
            const bool registered = isRegistered(local);
            if (registered != fit->isActivated())
                stack[fit] = registered;
            continue;
        }
        byFileName.insert(QFileInfo(local).fileName(), fit);
        if (!fit->isActivated())
            continue;
        const QString copy(managed + QLatin1Char('/') + fit->activationName());
        if (!QFileInfo::exists(copy) || !isRegistered(copy)) {
            qCDebug(FONTMATRIX_LOG) << fit->path() << "is not installed any more";
            stack[fit] = false;
        }
    }
    if (!stack.isEmpty()) {
        FMFontDb::DB()->TransactionBegin();
        for (auto it(stack.constBegin()); it != stack.constEnd(); ++it)
            it.key()->setActivated(it.value());
        FMFontDb::DB()->TransactionEnd();
    }

    // copies that were in use when their font was deactivated: "<size>-<file name>" of an inactive font
    const QFileInfoList files(QDir(managed).entryInfoList(QDir::Files));
    for (const QFileInfo &fi : files) {
        if (isRegistered(fi.absoluteFilePath()))
            continue;
        const QString name(fi.fileName());
        const qsizetype dash = name.indexOf(QLatin1Char('-'));
        if (dash <= 0)
            continue;
        bool numeric = false;
        const qint64 size = name.first(dash).toLongLong(&numeric);
        if (!numeric)
            continue;
        const QList<FontItem *> candidates(byFileName.values(name.sliced(dash + 1)));
        for (FontItem *fit : candidates) {
            if (fit->isActivated() || QFileInfo(fit->localPath()).size() != size)
                continue;
            if (QFile::remove(fi.absoluteFilePath()))
                qCDebug(FONTMATRIX_LOG) << "deleted the copy left behind:" << fi.absoluteFilePath();
            else
                qCWarning(FONTMATRIX_LOG) << "still unable to delete" << fi.absoluteFilePath();
            break;
        }
    }
}

#else // Linux and the other Unices

/*
    A font is activated by a copy in ~/.local/share/fonts/fontmatrix, the user
    font directory every fontconfig scans by itself, which Flatpak hands to
    every sandbox and GNOME watches: nothing is configured anywhere. A system
    font is deactivated by hiding it, a rejectfont pattern in a fontconfig
    file of Fontmatrix's own in ~/.config/fontconfig/conf.d, written from the
    flags of the system fonts. The user's fonts.conf is not touched.
*/

#include "fmconfig.h"
#include "fmpaths.h"
#include "helper/fontmatrixsystemfonts.h"

#ifdef HAVE_KAUTH
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#endif

#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QSet>

#include <unistd.h>

namespace
{
/// a copy of the file: a hard link when the file system allows one, the bytes otherwise
bool copyFont(const QString &from, const QString &to)
{
    if (QFileInfo::exists(to))
        return true;
    if (::link(QFile::encodeName(from).constData(), QFile::encodeName(to).constData()) == 0)
        return true;
    return QFile::copy(from, to);
}

const QString systemScopeKey(QStringLiteral("Activation/SystemWide"));
}

bool FMActivate::systemScopeAvailable()
{
#ifdef HAVE_KAUTH
    // asked of polkit once: the action is there when the helper's policy is installed on the host
    static const bool available = [] {
        if (QFileInfo::exists(QStringLiteral("/.flatpak-info")))
            return false;
        KAuth::Action action(FontmatrixSystemFonts::HelperId + QStringLiteral(".activate"));
        action.setHelperId(FontmatrixSystemFonts::HelperId);
        const KAuth::Action::AuthStatus status = action.status();
        return status != KAuth::Action::InvalidStatus && status != KAuth::Action::ErrorStatus;
    }();
    return available;
#else
    return false;
#endif
}

bool FMActivate::systemScope()
{
    return systemScopeAvailable() && FMConfig::value(systemScopeKey, false).toBool();
}

bool FMActivate::runHelper([[maybe_unused]] const QString &action, [[maybe_unused]] const QVariantMap &args, QVariantMap *reply, QString *error)
{
#ifdef HAVE_KAUTH
    KAuth::Action kaction(FontmatrixSystemFonts::HelperId + QLatin1Char('.') + action);
    kaction.setHelperId(FontmatrixSystemFonts::HelperId);
    kaction.setArguments(args);
    KAuth::ExecuteJob *job = kaction.execute();
    const bool ok = job->exec();
    if (ok) {
        if (reply)
            *reply = job->data();
    } else if (error) {
        *error = job->errorString();
    }
    qCDebug(FONTMATRIX_LOG) << "helper" << action << (ok ? "done" : "failed") << (ok ? QString() : job->errorString());
    job->deleteLater();
    return ok;
#else
    if (reply)
        reply->clear();
    if (error)
        *error = QStringLiteral("built without KAuth");
    return false;
#endif
}

QString FMActivate::copyOf(FontItem *fit)
{
    const QString name(fit->activationName());
    if (name.isEmpty())
        return QString();
    const QString user(typotek::getInstance()->getManagedDir() + QLatin1Char('/') + name);
    if (QFileInfo::exists(user))
        return user;
    const QString system(FontmatrixSystemFonts::Directory + QLatin1Char('/') + name);
    if (QFileInfo::exists(system))
        return system;
    return QString();
}

QStringList FMActivate::hiddenSystemFonts()
{
    typotek *T(typotek::getInstance());
    QStringList hidden;
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts) {
        if (T->isSysFont(fit) && !fit->isActivated())
            hidden << fit->path();
    }
    hidden.sort();
    return hidden;
}

void FMActivate::writeSystemRejects(bool *ok)
{
    QVariantMap args;
    args.insert(QStringLiteral("files"), hiddenSystemFonts());
    QString error;
    const bool done = runHelper(QStringLiteral("rejects"), args, nullptr, &error);
    if (!done)
        qCWarning(FONTMATRIX_LOG) << "the rejects of the system were not written:" << error;
    if (ok)
        *ok = done;
}

bool FMActivate::setSystemScope(bool system)
{
    if (system == FMConfig::value(systemScopeKey, false).toBool())
        return true;
    bool ok = true;
    if (system) {
        // the hidden fonts hide for everybody now, the user's file has nothing more to say
        writeSystemRejects(&ok);
        if (!ok)
            return false;
        FMConfig::setValue(systemScopeKey, true);
        const QString userFile(FMPaths::FontconfigRejectsFile());
        if (QFileInfo::exists(userFile))
            QFile::remove(userFile);
    } else {
        QVariantMap args;
        args.insert(QStringLiteral("files"), QStringList());
        QString error;
        if (!runHelper(QStringLiteral("rejects"), args, nullptr, &error)) {
            qCWarning(FONTMATRIX_LOG) << "the rejects of the system were not removed:" << error;
            return false;
        }
        FMConfig::setValue(systemScopeKey, false);
        writeRejects();
    }
    return true;
}

void FMActivate::activate(QList<FontItem *> fitList, bool act)
{
    QHash<FontItem *, bool> stack;
    typotek *T(typotek::getInstance());
    const QString managed(T->getManagedDir());
    const bool forEverybody = systemScope();
    // what goes through the root helper, in one call and one password
    QList<FontItem *> systemCopies;
    QList<FontItem *> systemRemovals;
    bool rejectsChanged = false;
    for (auto *fit : fitList) {
        if (act) // Activation
        {
            if (fit->isRemote() && !fit->isCached()) {
                // the file comes first; this font is activated on its own when it is here
                connect(
                    fit,
                    &FontItem::downloadFinished,
                    this,
                    [this, fit](bool ok) {
                        if (ok)
                            activate(QList<FontItem *>() << fit, true);
                    },
                    Qt::SingleShotConnection);
                fit->getFromNetwork();
                continue;
            }
            if (fit->isActivated()) {
                m_errors[fit->path()] = errorStrings[ALREADY_ACTIVE];
                continue;
            }
            if (T->isSysFont(fit)) {
                // hidden until now: shown again once the rejects file is written
                stack[fit] = true;
                rejectsChanged = true;
                continue;
            }
            if (forEverybody) {
                systemCopies << fit;
                continue;
            }
            const QString copy(managed + QLatin1Char('/') + fit->activationName());
            if (!copyFont(fit->localPath(), copy)) {
                qCWarning(FONTMATRIX_LOG) << "unable to copy" << fit->localPath() << "to" << copy;
                m_errors[fit->path()] = errorStrings[NO_COPY];
                continue;
            }
            stack[fit] = true;
            qCDebug(FONTMATRIX_LOG) << fit->path() << "copied to" << copy;
            if (!fit->afm().isEmpty() && !copyFont(fit->afm(), managed + QLatin1Char('/') + fit->activationAFMName())) {
                qCWarning(FONTMATRIX_LOG) << "unable to copy" << fit->afm();
                m_errors[fit->path()] = errorStrings[MISSING_AFM];
            }
        } else // Deactivation
        {
            if (!fit->isActivated()) {
                m_errors[fit->path()] = errorStrings[ALREADY_UNACTIVE];
                continue;
            }
            if (T->isSysFont(fit)) {
                stack[fit] = false;
                rejectsChanged = true;
                continue;
            }
            const QString copy(managed + QLatin1Char('/') + fit->activationName());
            if (!QFileInfo::exists(copy) && QFileInfo::exists(FontmatrixSystemFonts::Directory + QLatin1Char('/') + fit->activationName())) {
                // activated for everybody: only the helper removes it
                systemRemovals << fit;
                continue;
            }
            if (QFileInfo::exists(copy) && !QFile::remove(copy)) {
                qCWarning(FONTMATRIX_LOG) << "unable to remove" << copy;
                m_errors[fit->path()] = errorStrings[NO_UNLINK];
                continue;
            }
            stack[fit] = false;
            if (!fit->afm().isEmpty()) {
                const QString afm(managed + QLatin1Char('/') + fit->activationAFMName());
                if (QFileInfo::exists(afm) && !QFile::remove(afm)) {
                    qCWarning(FONTMATRIX_LOG) << "unable to remove" << afm;
                    m_errors[fit->afm()] = errorStrings[NO_UNLINK];
                }
            }
        }
    }

    if (!systemCopies.isEmpty()) {
        QVariantMap args;
        QStringList sources;
        QStringList names;
        for (FontItem *fit : std::as_const(systemCopies)) {
            sources << fit->localPath();
            names << fit->activationName();
        }
        args.insert(QStringLiteral("sources"), sources);
        args.insert(QStringLiteral("names"), names);
        QVariantMap reply;
        QString error;
        const bool done = runHelper(QStringLiteral("activate"), args, &reply, &error);
        const QStringList copied(reply.value(QStringLiteral("copied")).toStringList());
        for (FontItem *fit : std::as_const(systemCopies)) {
            if (done && copied.contains(fit->activationName()))
                stack[fit] = true;
            else
                m_errors[fit->path()] = errorStrings[done ? NO_COPY : NO_AUTHORIZATION];
        }
    }
    if (!systemRemovals.isEmpty()) {
        QVariantMap args;
        QStringList names;
        for (FontItem *fit : std::as_const(systemRemovals))
            names << fit->activationName();
        args.insert(QStringLiteral("names"), names);
        QVariantMap reply;
        QString error;
        const bool done = runHelper(QStringLiteral("deactivate"), args, &reply, &error);
        const QStringList removed(reply.value(QStringLiteral("removed")).toStringList());
        for (FontItem *fit : std::as_const(systemRemovals)) {
            if (done && removed.contains(fit->activationName()))
                stack[fit] = false;
            else
                m_errors[fit->path()] = errorStrings[done ? NO_UNLINK : NO_AUTHORIZATION];
        }
    }

    QStringList aList;
    FMFontDb::DB()->TransactionBegin();
    for (auto it(stack.constBegin()); it != stack.constEnd(); ++it) {
        it.key()->setActivated(it.value());
        aList << it.key()->path();
    }
    FMFontDb::DB()->TransactionEnd();

    if (rejectsChanged) {
        if (forEverybody) {
            bool ok = true;
            writeSystemRejects(&ok);
            if (!ok) {
                // the flags went where the file did not: back
                FMFontDb::DB()->TransactionBegin();
                for (auto it(stack.constBegin()); it != stack.constEnd(); ++it) {
                    if (T->isSysFont(it.key())) {
                        it.key()->setActivated(!it.value());
                        m_errors[it.key()->path()] = errorStrings[NO_AUTHORIZATION];
                    }
                }
                FMFontDb::DB()->TransactionEnd();
            }
        } else {
            writeRejects();
        }
    }
    Q_EMIT activationEvent(aList);
}

void FMActivate::writeRejects()
{
    const QStringList hidden(hiddenSystemFonts());
    const QString path(FMPaths::FontconfigRejectsFile());
    if (hidden.isEmpty()) {
        if (QFileInfo::exists(path) && !QFile::remove(path))
            qCWarning(FONTMATRIX_LOG) << "cannot remove" << path;
        return;
    }
    const QByteArray content(FontmatrixSystemFonts::rejectsDocument(hidden));

    // fontconfig reads its configuration again when a file changes: only when it does
    QFile old(path);
    if (old.open(QIODevice::ReadOnly) && old.readAll() == content)
        return;
    old.close();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size() || !file.commit())
        qCWarning(FONTMATRIX_LOG) << "cannot write" << path << ":" << file.errorString();
    else
        qCDebug(FONTMATRIX_LOG) << hidden.size() << "system fonts hidden in" << path;
}

void FMActivate::migrateActivated(const QString &oldDir)
{
    typotek *T(typotek::getInstance());
    FMFontDb *db(FMFontDb::DB());
    const QString managed(T->getManagedDir());

    // the copies, before the links go
    const QList<FontItem *> fonts(db->AllFonts());
    for (FontItem *fit : fonts) {
        if (!fit->isActivated() || T->isSysFont(fit) || fit->activationName().isEmpty())
            continue;
        if (!copyFont(fit->localPath(), managed + QLatin1Char('/') + fit->activationName()))
            qCWarning(FONTMATRIX_LOG) << "migration: unable to copy" << fit->localPath();
        else if (!fit->afm().isEmpty())
            copyFont(fit->afm(), managed + QLatin1Char('/') + fit->activationAFMName());
    }
    QDir old(oldDir);
    if (old.exists() && !old.removeRecursively())
        qCWarning(FONTMATRIX_LOG) << "migration: cannot remove" << oldDir;
    else
        qCDebug(FONTMATRIX_LOG) << "migration: the links of" << oldDir << "are copies in" << managed;

    // fonts.conf: the directory entries and the rejectfont globs of the versions before, one last edit
    QFile fcfile(FMPaths::FontconfigUserFile());
    if (!fcfile.open(QIODevice::ReadOnly))
        return;
    QDomDocument fc;
    if (!fc.setContent(&fcfile)) {
        fcfile.close();
        return;
    }
    fcfile.close();
    bool changed = false;
    QList<QDomElement> gone;
    const QDomNodeList dirs = fc.elementsByTagName(QStringLiteral("dir"));
    for (int i = 0; i < dirs.count(); ++i) {
        const QDomElement dir = dirs.at(i).toElement();
        const QString text(dir.text().trimmed());
        if (text.endsWith(QLatin1String("/Activated"))
            && (text.contains(QLatin1String("/Fontmatrix/")) || text.contains(QLatin1String("/.Fontmatrix/"))
                || text.contains(QLatin1String("/Undertype/fontmatrix/"))))
            gone << dir;
    }
    QStringList nowHidden;
    const QDomNodeList globs = fc.elementsByTagName(QStringLiteral("glob"));
    for (int i = 0; i < globs.count(); ++i) {
        const QDomElement glob = globs.at(i).toElement();
        if (glob.parentNode().nodeName() != QLatin1String("rejectfont"))
            continue;
        const QString text(glob.text().trimmed());
        // a glob on the very file of a system font is one of ours
        if (T->isSysFont(db->Knows(text) ? db->Font(text) : nullptr)) {
            nowHidden << text;
            gone << glob;
        }
    }
    for (QDomElement &element : gone) {
        QDomNode parent = element.parentNode();
        parent.removeChild(element);
        changed = true;
        // a rejectfont or selectfont left with nothing in it goes too
        while (!parent.isNull() && parent.isElement() && !parent.hasChildNodes()
               && (parent.nodeName() == QLatin1String("rejectfont") || parent.nodeName() == QLatin1String("selectfont"))) {
            QDomNode above = parent.parentNode();
            above.removeChild(parent);
            parent = above;
        }
    }
    if (!nowHidden.isEmpty()) {
        db->TransactionBegin();
        for (const QString &path : std::as_const(nowHidden))
            db->Font(path)->setActivated(false);
        db->TransactionEnd();
    }
    if (changed) {
        QSaveFile out(fcfile.fileName());
        if (out.open(QIODevice::WriteOnly)) {
            QTextStream ts(&out);
            fc.save(ts, 4);
            ts.flush();
            if (!out.commit())
                qCWarning(FONTMATRIX_LOG) << "migration: cannot write" << out.fileName();
            else
                qCDebug(FONTMATRIX_LOG) << "migration:" << gone.size() << "entries of" << out.fileName() << "removed";
        }
    }
}

void FMActivate::reconcileActivated()
{
    typotek *T(typotek::getInstance());
    const QString managed(T->getManagedDir());
    QSet<QString> belongs;
    QHash<FontItem *, bool> stack;
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts) {
        if (T->isSysFont(fit) || !fit->isActivated())
            continue;
        const QString name(fit->activationName());
        if (name.isEmpty() || copyOf(fit).isEmpty()) {
            qCDebug(FONTMATRIX_LOG) << fit->path() << "has no copy any more";
            stack[fit] = false;
            continue;
        }
        belongs << name;
        if (!fit->afm().isEmpty())
            belongs << fit->activationAFMName();
    }
    if (!stack.isEmpty()) {
        FMFontDb::DB()->TransactionBegin();
        for (auto it(stack.constBegin()); it != stack.constEnd(); ++it)
            it.key()->setActivated(it.value());
        FMFontDb::DB()->TransactionEnd();
    }

    // the folder is Fontmatrix's alone: what belongs to no active font is a leftover
    const QFileInfoList files(QDir(managed).entryInfoList(QDir::Files | QDir::System | QDir::Hidden));
    for (const QFileInfo &fi : files) {
        if (!belongs.contains(fi.fileName())) {
            if (QFile::remove(fi.absoluteFilePath()))
                qCDebug(FONTMATRIX_LOG) << "removed the leftover" << fi.absoluteFilePath();
        }
    }

    // the rejects of everybody are written when a font is switched: not at every start, not for a password
    if (!systemScope())
        writeRejects();
}

#endif

QMap<QString, QString> FMActivate::errors()
{
    QMap<QString, QString> ret(m_errors);
    m_errors.clear();
    return ret;
}

#include "moc_fmactivate.cpp"
