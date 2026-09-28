/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmhyphenator.h"

#include <QDebug>
#include <QFile>
#include <QStringDecoder>
#include <QStringEncoder>
#include <QStringList>

#include <cstdlib>

FMHyphenator::FMHyphenator()
{
    dict = nullptr;
    textEncoder = nullptr;
    textDecoder = nullptr;
}

bool FMHyphenator::loadDict(const QString &dictPath, int leftMin, int rightMin)
{
    /* load the hyphenation dictionary */
    if (dict) {
        if (dictPath == currentDictPath)
            return true;
        hnj_hyphen_free(dict);
        dict = nullptr;
        currentDictPath.clear();
    }

    if ((dict = hnj_hyphen_load(dictPath.toLocal8Bit().constData())) == nullptr) {
        qDebug() << "Unable to load dict file:" << dictPath;
        return false;
    }
    currentDictPath = dictPath;

    // the first line of the dictionary names its encoding; the words go in as that
    delete textEncoder;
    textEncoder = nullptr;
    delete textDecoder;
    textDecoder = nullptr;
    QFile df(dictPath);
    if (df.open(QIODevice::ReadOnly)) {
        QByteArray codecName = df.readLine().trimmed();
        textEncoder = new QStringEncoder(codecName.constData());
        textDecoder = new QStringDecoder(codecName.constData());
        if (!textEncoder->isValid() || !textDecoder->isValid()) {
            delete textEncoder;
            textEncoder = nullptr;
            delete textDecoder;
            textDecoder = nullptr;
        }
    }
    dict->lhmin = leftMin;
    dict->rhmin = rightMin;
    return true;
}

void FMHyphenator::unload()
{
    if (dict)
        hnj_hyphen_free(dict);
    dict = nullptr;
    currentDictPath.clear();
}

FMHyphenator::~FMHyphenator()
{
    if (dict)
        hnj_hyphen_free(dict);
    delete textEncoder;
    delete textDecoder;
}

/**
 *
 * @param word
 * @return
 */
HyphList FMHyphenator::hyphenate(const QString &word) const
{
    HyphList ret;
    if (!dict)
        return ret;

    // Only the letters go to the library: the punctuation around the word ("sollen."
    // or "(schnell") is put back on the pairs, and a break position is counted in the
    // whole word, which is what the layout looks up. A dot inside the word is libhyphen's
    // pattern boundary; such a word is left unhyphenated.
    int head(0);
    while (head < word.size() && !word.at(head).isLetter())
        ++head;
    int tail(word.size());
    while (tail > head && !word.at(tail - 1).isLetter())
        --tail;
    const QString prefix(word.left(head));
    const QString suffix(word.mid(tail));
    const QString ref(word.mid(head, tail - head));
    if (ref.size() < 2 || ref.contains(QLatin1Char('.')))
        return ret;
    QByteArray hw(textEncoder ? textEncoder->encode(ref.toLower()) : ref.toLower().toLocal8Bit());
    QByteArray ht(hw.size() + 5, '0');

    char **rep = nullptr;
    int *pos = nullptr;
    int *cut = nullptr;
    if (hnj_hyphen_hyphenate2(dict, hw.data(), hw.size(), ht.data(), nullptr, &rep, &pos, &cut)) {
        qDebug() << "Hyphenate(" << word << ") failed";
        return ret;
    }

    // ht, rep, pos and cut are indexed by character (libhyphen skips the UTF-8
    // continuation bytes itself). A break after character i is "ht[i] & 1"; a
    // non-standard one replaces cut[i] characters from i - pos[i] + 1 with rep[i], and
    // the break is the "=" inside rep[i]  -  see single_hyphenations() in libhyphen's example.c
    for (int i(0); i + 1 < ref.size(); ++i) {
        if (!(ht[i] & 1))
            continue;
        if (rep && rep[i]) {
            const QString r(textDecoder ? textDecoder->decode(QByteArray(rep[i])) : QString::fromLocal8Bit(rep[i]));
            const int at(i - (pos ? pos[i] : 0) + 1);
            const int len(cut ? cut[i] : 0);
            ret[head + i] =
                QPair<QString, QString>(prefix + ref.left(at) + r.section(QLatin1Char('='), 0, 0), r.section(QLatin1Char('='), 1) + ref.mid(at + len) + suffix);
        } else
            ret[head + i] = QPair<QString, QString>(prefix + ref.left(i + 1), ref.mid(i + 1) + suffix);
    }

    // allocated by hnj_hyphen_hyphenate2() when the word has a non-standard hyphenation,
    // one entry per character (see example.c of libhyphen)
    if (rep) {
        for (int i(0); i < hw.size(); ++i)
            free(rep[i]);
        free(rep);
    }
    free(pos);
    free(cut);

    return ret;
}
