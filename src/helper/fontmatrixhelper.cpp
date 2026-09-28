/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fontmatrixhelper.h"
#include "fontmatrixsystemfonts.h"

#include <KAuth/HelperSupport>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>

#include <sys/stat.h>

namespace
{
/// a file name and nothing more: no directory, no dot entries
bool isPlainName(const QString &name)
{
    return !name.isEmpty() && name.size() <= 255 && !name.contains(QLatin1Char('/')) && name != QLatin1String(".") && name != QLatin1String("..");
}

/// a regular file that everybody may read: what a font for everybody has to be anyway
bool isReadableByAll(const QString &path)
{
    struct stat st;
    if (::stat(QFile::encodeName(path).constData(), &st) != 0)
        return false;
    return S_ISREG(st.st_mode) && (st.st_mode & S_IROTH);
}

/// fontconfig's cache of the directory, so that the first application does not scan it
void refreshCache()
{
    const QString fcCache(QStandardPaths::findExecutable(QStringLiteral("fc-cache")));
    if (fcCache.isEmpty())
        return;
    QProcess::execute(fcCache, {QStringLiteral("-f"), FontmatrixSystemFonts::Directory});
}

KAuth::ActionReply failure(const QString &description)
{
    KAuth::ActionReply reply(KAuth::ActionReply::HelperErrorReply());
    reply.setErrorDescription(description);
    return reply;
}
}

KAuth::ActionReply FontmatrixHelper::activate(const QVariantMap &args)
{
    const QStringList sources(args.value(QStringLiteral("sources")).toStringList());
    const QStringList names(args.value(QStringLiteral("names")).toStringList());
    if (sources.isEmpty() || sources.size() != names.size())
        return failure(QStringLiteral("activate: sources and names do not match"));

    QDir dir(FontmatrixSystemFonts::Directory);
    if (!dir.exists() && !dir.mkpath(dir.absolutePath()))
        return failure(QStringLiteral("cannot create ") + dir.absolutePath());

    QStringList copied;
    QStringList failed;
    for (int i = 0; i < sources.size(); ++i) {
        const QString source(QFileInfo(sources.at(i)).canonicalFilePath());
        const QString name(names.at(i));
        if (!isPlainName(name) || source.isEmpty() || !isReadableByAll(source)) {
            failed << name;
            continue;
        }
        const QString target(dir.absoluteFilePath(name));
        if (QFileInfo::exists(target) || QFile::copy(source, target)) {
            QFile::setPermissions(target, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
            copied << name;
        } else {
            failed << name;
        }
    }
    if (!copied.isEmpty())
        refreshCache();

    KAuth::ActionReply reply(KAuth::ActionReply::SuccessReply());
    reply.addData(QStringLiteral("copied"), copied);
    reply.addData(QStringLiteral("failed"), failed);
    return reply;
}

KAuth::ActionReply FontmatrixHelper::deactivate(const QVariantMap &args)
{
    const QStringList names(args.value(QStringLiteral("names")).toStringList());
    if (names.isEmpty())
        return failure(QStringLiteral("deactivate: nothing named"));

    const QDir dir(FontmatrixSystemFonts::Directory);
    QStringList removed;
    QStringList failed;
    for (const QString &name : names) {
        if (!isPlainName(name)) {
            failed << name;
            continue;
        }
        const QString target(dir.absoluteFilePath(name));
        if (!QFileInfo::exists(target) || QFile::remove(target))
            removed << name;
        else
            failed << name;
    }
    if (!removed.isEmpty())
        refreshCache();

    KAuth::ActionReply reply(KAuth::ActionReply::SuccessReply());
    reply.addData(QStringLiteral("removed"), removed);
    reply.addData(QStringLiteral("failed"), failed);
    return reply;
}

KAuth::ActionReply FontmatrixHelper::rejects(const QVariantMap &args)
{
    QStringList files;
    const QStringList given(args.value(QStringLiteral("files")).toStringList());
    for (const QString &file : given) {
        // a font of the system, not anything: an absolute path to a file that is there
        if (file.startsWith(QLatin1Char('/')) && QFileInfo(file).isFile())
            files << file;
    }
    files.sort();

    const QString path(FontmatrixSystemFonts::RejectsFile);
    if (files.isEmpty()) {
        if (QFileInfo::exists(path) && !QFile::remove(path))
            return failure(QStringLiteral("cannot remove ") + path);
        return KAuth::ActionReply::SuccessReply();
    }
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile out(path);
    const QByteArray content(FontmatrixSystemFonts::rejectsDocument(files));
    if (!out.open(QIODevice::WriteOnly) || out.write(content) != content.size() || !out.commit())
        return failure(QStringLiteral("cannot write ") + path + QStringLiteral(": ") + out.errorString());
    QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.fontmatrix", FontmatrixHelper)

#include "moc_fontmatrixhelper.cpp"
