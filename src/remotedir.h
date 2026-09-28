/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTEDIR_H
#define REMOTEDIR_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QUrl>

class QNetworkReply;

/**
 * Reads the font catalogues of remote directories.
 *
 * A remote directory is a web directory made by "Export Data": the font files,
 * a preview image per font and an index, fontmatrix.data, that names them with
 * their family, style, type and tags. From the index and the previews the
 * fonts can be listed, filtered and looked at without their files; a file is
 * downloaded when a font is opened or activated (FontItem::getFromNetwork()).
 *
 * run() fetches the index of every directory, then the previews, and emits
 * listIsReady() when all of it has arrived or failed. A directory that cannot
 * be read is reported and skipped.
 */
class RemoteDir : public QObject
{
    Q_OBJECT
public:
    struct FontInfo {
        QString file; ///< the URL of the font file
        QString family;
        QString variant;
        QString type;
        QString info; ///< the HTML block of the font's information page
        QStringList tags;
        QPixmap pix; ///< the preview, empty when the directory has none
        [[nodiscard]] QString dump() const;
    };

    /// the file of a directory that lists its fonts
    static QString indexFileName();

    explicit RemoteDir(const QStringList &dirs, QObject *parent = nullptr);
    ~RemoteDir() override;

    void run();
    [[nodiscard]] QList<FontInfo> rFonts() const
    {
        return m_fonts;
    }
    [[nodiscard]] bool isReady() const
    {
        return m_ready;
    }

Q_SIGNALS:
    void listIsReady();

private:
    void indexArrived(QNetworkReply *reply, const QUrl &base);
    void previewArrived(QNetworkReply *reply, int fontIndex);
    void requestDone();

    QStringList argDirs;
    QList<FontInfo> m_fonts;
    int m_pending = 0;
    bool m_ready = false;
};

#endif
