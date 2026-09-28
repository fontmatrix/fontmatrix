/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "remotedir.h"
#include "fontmatrix_debug.h"
#include "typotek.h"

#include <KLocalizedString>

#include <QDomDocument>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

QString RemoteDir::indexFileName()
{
    return QStringLiteral("fontmatrix.data");
}

RemoteDir::RemoteDir(const QStringList &dirs, QObject *parent)
    : QObject(parent)
    , argDirs(dirs)
{
    qCDebug(FONTMATRIX_LOG) << "RemoteDir" << dirs;
    if (argDirs.isEmpty())
        m_ready = true;
}

RemoteDir::~RemoteDir() = default;

void RemoteDir::run()
{
    if (argDirs.isEmpty()) {
        Q_EMIT listIsReady();
        return;
    }
    QNetworkAccessManager *net = typotek::getInstance()->network();
    for (const QString &dir : std::as_const(argDirs)) {
        QUrl base(dir);
        if (!base.isValid() || base.scheme().isEmpty()) {
            qCWarning(FONTMATRIX_LOG) << "Not a URL, no remote directory:" << dir;
            continue;
        }
        // the index and the files sit in the directory itself
        QString path(base.path());
        if (!path.endsWith(QLatin1Char('/')))
            path += QLatin1Char('/');
        base.setPath(path);
        QUrl index(base.resolved(QUrl(indexFileName())));
        typotek::getInstance()->showStatusMessage(i18nc("@info:status", "Downloading %1", index.toString()));
        ++m_pending;
        QNetworkReply *reply = net->get(QNetworkRequest(index));
        connect(reply, &QNetworkReply::finished, this, [this, reply, base]() {
            indexArrived(reply, base);
        });
    }
    if (m_pending == 0) {
        m_ready = true;
        Q_EMIT listIsReady();
    }
}

void RemoteDir::indexArrived(QNetworkReply *reply, const QUrl &base)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(FONTMATRIX_LOG) << "Cannot read the remote directory" << base.toString() << ":" << reply->errorString();
        requestDone();
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(reply->readAll())) {
        qCWarning(FONTMATRIX_LOG) << base.toString() << "has no readable" << indexFileName();
        requestDone();
        return;
    }
    QNetworkAccessManager *net = typotek::getInstance()->network();
    const QDomNodeList colList = doc.elementsByTagName(QStringLiteral("fontfile"));
    for (int i = 0; i < colList.length(); ++i) {
        const QDomElement col = colList.item(i).toElement();
        const QString basename(col.namedItem(QStringLiteral("file")).toElement().text());
        if (basename.isEmpty())
            continue;
        FontInfo fi;
        fi.family = col.attribute(QStringLiteral("family"));
        fi.variant = col.attribute(QStringLiteral("variant"));
        fi.type = col.attribute(QStringLiteral("type"));
        fi.file = base.resolved(QUrl(basename)).toString();
        fi.info = col.namedItem(QStringLiteral("info")).toElement().text();
        const QDomNodeList taglist = col.elementsByTagName(QStringLiteral("tag"));
        for (int ti = 0; ti < taglist.size(); ++ti) {
            const QString tag(taglist.at(ti).toElement().text());
            if (!tag.isEmpty() && !fi.tags.contains(tag))
                fi.tags << tag;
        }
        m_fonts << fi;

        // the preview is optional: a directory made before it was written has none
        const int fontIndex = m_fonts.size() - 1;
        ++m_pending;
        QNetworkReply *preview = net->get(QNetworkRequest(base.resolved(QUrl(basename + QStringLiteral(".png")))));
        connect(preview, &QNetworkReply::finished, this, [this, preview, fontIndex]() {
            previewArrived(preview, fontIndex);
        });
    }
    qCDebug(FONTMATRIX_LOG) << base.toString() << ":" << colList.length() << "fonts";
    requestDone();
}

void RemoteDir::previewArrived(QNetworkReply *reply, int fontIndex)
{
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError && fontIndex >= 0 && fontIndex < m_fonts.size()) {
        const QImage img(QImage::fromData(reply->readAll()));
        if (!img.isNull())
            m_fonts[fontIndex].pix = QPixmap::fromImage(img);
    }
    requestDone();
}

void RemoteDir::requestDone()
{
    if (--m_pending > 0)
        return;
    m_ready = true;
    typotek::getInstance()->showStatusMessage(
        i18ncp("@info:status", "%1 font description from the network", "%1 font descriptions from the network", m_fonts.size()));
    Q_EMIT listIsReady();
}

/// FontInfo **********************************************
QString RemoteDir::FontInfo::dump() const
{
    const QString sep(QStringLiteral(" | "));
    return file + sep + family + sep + variant + sep + type + sep + tags.join(sep);
}

#include "moc_remotedir.cpp"
