/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmportal.h"
#include "fontmatrix_debug.h"

#include <QFile>
#include <QGuiApplication>

#if defined(Q_OS_UNIX) && !defined(PLATFORM_APPLE)
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QVariantMap>
#include <QWidget>
#include <QWindow>

namespace
{
constexpr QLatin1String portalService("org.freedesktop.portal.Desktop");
constexpr QLatin1String portalPath("/org/freedesktop/portal/desktop");
constexpr QLatin1String openUriInterface("org.freedesktop.portal.OpenURI");

/**
 * The window the portal parents its dialog to. Only X11 windows have a handle
 * that can be written down like this; on Wayland one is asked for with the
 * xdg-foreign protocol, and without it the dialog simply opens unparented.
 */
QString parentWindowHandle(QWidget *parent)
{
    if (!parent || !parent->windowHandle())
        return QString();
    const QWindow *window = parent->window()->windowHandle();
    if (!window || QGuiApplication::platformName() != QLatin1String("xcb"))
        return QString();
    return QStringLiteral("x11:%1").arg(window->winId(), 0, 16);
}
}

bool FMPortal::isAvailable()
{
    static const bool available = [] {
        if (!QDBusConnection::sessionBus().isConnected())
            return false;
        QDBusMessage message(QDBusMessage::createMethodCall(portalService, portalPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("Get")));
        message << openUriInterface << QLatin1String("version");
        const QDBusMessage reply(QDBusConnection::sessionBus().call(message, QDBus::Block, 2000));
        return reply.type() == QDBusMessage::ReplyMessage;
    }();
    return available;
}

namespace
{
/// OpenURI.OpenFile with the file itself, which is what lets a sandboxed path be opened
bool openFile(const QString &path, bool ask, bool writable, QWidget *parent)
{
    if (!FMPortal::isAvailable())
        return false;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(FONTMATRIX_LOG) << "cannot open" << path << "to hand it to the desktop";
        return false;
    }
    QDBusMessage message(QDBusMessage::createMethodCall(portalService, portalPath, openUriInterface, QLatin1String("OpenFile")));
    QVariantMap options;
    options.insert(QStringLiteral("ask"), ask);
    options.insert(QStringLiteral("writable"), writable);
    message << parentWindowHandle(parent) << QVariant::fromValue(QDBusUnixFileDescriptor(file.handle())) << options;

    const QDBusMessage reply(QDBusConnection::sessionBus().call(message, QDBus::Block, 5000));
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(FONTMATRIX_LOG) << "the portal did not take" << path << ":" << reply.errorMessage();
        return false;
    }
    qCDebug(FONTMATRIX_LOG) << path << "handed to the desktop";
    return true;
}
}

bool FMPortal::openWith(const QString &path, QWidget *parent)
{
    // the chooser of the desktop, and the file writable, so that an editor can save it
    return openFile(path, true, true, parent);
}

bool FMPortal::openRead(const QString &path, QWidget *parent)
{
    return openFile(path, false, false, parent);
}

bool FMPortal::trash(const QString &path)
{
    if (!FMPortal::isAvailable())
        return false;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(FONTMATRIX_LOG) << "cannot open" << path << "to trash it";
        return false;
    }
    QDBusMessage message(QDBusMessage::createMethodCall(portalService, portalPath, QLatin1String("org.freedesktop.portal.Trash"), QLatin1String("TrashFile")));
    message << QVariant::fromValue(QDBusUnixFileDescriptor(file.handle()));
    const QDBusReply<uint> reply(QDBusConnection::sessionBus().call(message, QDBus::Block, 5000));
    // 1 is success, 0 failure
    if (!reply.isValid() || reply.value() != 1) {
        qCWarning(FONTMATRIX_LOG) << "the portal did not trash" << path << ":" << reply.error().message();
        return false;
    }
    return true;
}

#else // no D-Bus desktop: Windows and macOS have their own ways of opening a file

bool FMPortal::isAvailable()
{
    return false;
}

bool FMPortal::openWith(const QString &, QWidget *)
{
    return false;
}

bool FMPortal::openRead(const QString &, QWidget *)
{
    return false;
}

bool FMPortal::trash(const QString &)
{
    return false;
}

#endif
