/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmshaper_own.h"
#include "fontmatrix_debug.h"

#include "fmconfig.h"
#include <QDebug>
#include <QDir>
#include <QFile>

FMOwnShaper::FMOwnShaper(QString s, QString lang)
{
    // We need to load at least a default rule
    Character noop;
    noop.GroupIndex = 1;
    Matches << MatchSequence();
    Matches.last().Properties << noop;
    Matches.last().Properties.last().isMatchedGroup = true;
    Replacements << ReplaceSequence();
    Replacements.last().Properties << noop;
    Replacements.last().Properties.last().GroupIndex = 1;

    loadRules(lang);
    fillIn(s);
}

FMOwnShaper::FMOwnShaper(QString lang)
{
    Character noop;
    noop.GroupIndex = 1;
    Matches << MatchSequence();
    Matches.last().Properties << noop;
    Matches.last().Properties.last().isMatchedGroup = true;
    Replacements << ReplaceSequence();
    Replacements.last().Properties << noop;
    Replacements.last().Properties.last().GroupIndex = 1;

    loadRules(lang);
}

FMOwnShaper::~FMOwnShaper() = default;

int FMOwnShaper::loadRules(QString lang)
{
    QString actualSDir;
    QString ShaperDir(FMConfig::value(QStringLiteral("Places/ShaperDataDir")).toString());
    if (ShaperDir.isEmpty()) {
        actualSDir = QLatin1String(":/shapers/");
    } else {
        actualSDir = ShaperDir + QDir::separator();
    }
    qCDebug(FONTMATRIX_LOG) << "SHAPER_FILES : " << actualSDir + lang + QLatin1String(".dict") << "; " << actualSDir + lang + QLatin1String(".match");
    QFile dictFile(actualSDir + lang + QLatin1String(".dict"));
    if (!dictFile.open(QIODevice::ReadOnly)) {
        qCWarning(FONTMATRIX_LOG) << "Failed to open " << dictFile.fileName();
        return 1;
    }
    QFile matchFile(actualSDir + lang + QLatin1String(".match"));
    if (!matchFile.open(QIODevice::ReadOnly)) {
        qCWarning(FONTMATRIX_LOG) << "Failed to open " << matchFile.fileName();
        return 1;
    }

    // lalala

    while (!dictFile.atEnd()) {
        QByteArray line = dictFile.readLine();
        if (line.startsWith('%')) // A bit of TeXish in it ;-)
            continue;

        QList<QByteArray> elems = line.split('|');
        if (!elems.isEmpty()) {
            bool ok;
            int unicode = elems.takeFirst().mid(0, 4).toInt(&ok, 16);
            if (!ok)
                qCDebug(FONTMATRIX_LOG) << "Oops";
            Dictionnary[unicode] = Character(unicode, elems);
        }
    }
    while (!matchFile.atEnd()) {
        QString line(CleanRule(QString::fromUtf8(matchFile.readLine())));

        if (line.startsWith(QLatin1Char('%')))
            continue;

        QList<QString> elems = line.split(QLatin1Char('|'));
        if (elems.size() == 2) {
            Matches.append(MatchSequence());
            Replacements.append(ReplaceSequence());

            Matches.last().SetMatch(elems[0].trimmed());
            Replacements.last().SetReplace(elems[1].trimmed());
        }
    }
    return 0;
}

void FMOwnShaper::fillIn(const QString &s)
{
    QStringList debug;
    In.clear();
    for (int i(0); i < s.size(); ++i) {
        if (Dictionnary.contains(s[i].unicode())) {
            In << Dictionnary[s[i].unicode()];
        } else {
            In << Character(s[i].unicode());
        }
        debug << QLatin1String("[") + QString::number(s[i].unicode(), 16) + QLatin1String("]");
    }
    qCDebug(FONTMATRIX_LOG) << debug.join(QStringLiteral(" "));
}

void FMOwnShaper::Op()
{
    /// Here is the beast :)
    /*
    It’s all about processing replacements

    To make it easy to read, we’ll first put matched chunks into a map
    then process each chunk in another loop.
    */

    // <index of the match sequence, matched list>
    QList<QPair<int, QList<Character>>> chunks;
    int idx(0);
    bool matched;
    while (idx < In.size()) {
        matched = false;
        // We begin at 1 since index 0 is NOOP
        for (int nm(1); nm < Matches.size(); ++nm) {
            int rc = Compare(idx, nm);
            if (rc > 0) {
                QList<Character> cl;
                QString debugString;
                for (int nc(0); nc < rc; ++nc) {
                    cl << In[idx + nc];
                    debugString += QLatin1String("[") + QString::number(In[idx + nc].unicode(), 16) + QLatin1String("]");
                }
                chunks.append(QPair<int, QList<Character>>(nm, cl));

                qCDebug(FONTMATRIX_LOG) << "MATCH : " << debugString;
                idx += rc;
                matched = true;
                break;
            }
        }
        if (!matched) {
            In[idx].isMatchedGroup = true;
            QList<Character> ul;
            ul << In[idx];
            chunks.append(QPair<int, QList<Character>>(0, ul));
            ++idx;
        }
    }

    // Now we apply replacements as defined in the rules file
    QList<QPair<int, QList<Character>>>::const_iterator chunkIt = chunks.constBegin();
    while (chunkIt != chunks.constEnd()) {
        Replace(chunkIt->first, chunkIt->second);
        ++chunkIt;
    }
}

/// Return 0 if not matched and number of consumed positions if it matched
int FMOwnShaper::Compare(int inIndex, int matchIndex)
{
    int matchLen = Matches.at(matchIndex).Properties.size();
    // We need to keep track of matched chars over "."
    QList<int> matchedGroup;
    if (matchLen > (In.size() - inIndex))
        return 0;
    for (int i(0); i < matchLen; ++i) {
        Character car = In.at(inIndex + i);
        Character mat = Matches.at(matchIndex).Properties[i];
        if (mat.isNull()) // We’ll just compare properties
        {
            if (mat.MatchAll) {
                for (const auto &prop : std::as_const(mat.CustomProperties)) {
                    if (!prop.isEmpty()) {
                        if (!car.CustomProperties.contains(prop))
                            return 0;
                    }
                }
                for (const auto &prop : std::as_const(car.CustomProperties)) {
                    if (!prop.isEmpty()) {
                        if (!mat.CustomProperties.contains(prop))
                            return 0;
                    }
                }
            } else {
                for (const auto &prop : std::as_const(mat.CustomProperties)) {
                    if (!prop.isEmpty()) {
                        if (!car.CustomProperties.contains(prop))
                            return 0;
                    }
                }
            }
            matchedGroup << inIndex + i;

        } else {
            if (mat.unicode() != car.unicode())
                return 0;
            else {
                if (mat.MatchAll) {
                    for (const auto &prop : std::as_const(mat.CustomProperties)) {
                        if (!car.CustomProperties.contains(prop))
                            return 0;
                    }
                    for (const auto &prop : std::as_const(car.CustomProperties)) {
                        if (!mat.CustomProperties.contains(prop))
                            return 0;
                    }
                } else {
                    for (const auto &prop : std::as_const(mat.CustomProperties)) {
                        if (!car.CustomProperties.contains(prop))
                            return 0;
                    }
                }
            }
        }
    }
    // It match
    for (const auto &idx : std::as_const(matchedGroup)) {
        In[idx].isMatchedGroup = true;
        // 		qDebug()<<"MATCHED" <<In[idx].unicode() << In[idx].DumpCustom();
    }
    return matchLen;
}

/// Apply replacement rule and append the result to Out
void FMOwnShaper::Replace(int repIndex, QList<Character> chunk)
{
    QList<Character> buffer;
    QMap<int, Character> matchedPos;
    // load matchedPos first
    int mIndex(0);
    for (const auto &car : chunk) {
        if (car.isMatchedGroup) {
            matchedPos[++mIndex] = car;
            // 			qDebug()<< "Matched "<<  QString::number(car.unicode(), 16 ) << " at pos "<< mIndex ;
        }
    }
    // Let replace :)
    for (const auto &rep : std::as_const(Replacements.at(repIndex).Properties)) {
        if (rep.isNull()) {
            Character tc(matchedPos[rep.GroupIndex].unicode(), rep.CustomProperties);
            buffer << tc;
        } else {
            buffer << rep;
        }
    }
    // Push in Out
    for (const auto &b : std::as_const(buffer)) {
        Out << b;
    }
}

void FMOwnShaper::DumpOut()
{
    // 	qDebug()<<"FMOwnShaper::DumpOut()";
    for (int i(0); i < Out.size(); ++i) {
        qCDebug(FONTMATRIX_LOG) << "Unicode(" << QString::number(Out[i].unicode(), 16) << ").[" << Out[i].DumpCustom() << "]";
    }
}

/// Character
Character::Character(int unicode, QList<QByteArray> tokens)
    : QChar(unicode)
    , MatchAll(false)
    , isMatchedGroup(false)
    , GroupIndex(0)
{
    for (int i(0); i < tokens.size(); ++i)
        AddProperty(QString::fromUtf8(tokens[i].trimmed()));
}

Character::Character(int unicode, QStringList tokens)
    : QChar(unicode)
    , MatchAll(false)
    , isMatchedGroup(false)
    , GroupIndex(0)
{
    for (int i(0); i < tokens.size(); ++i)
        AddProperty(tokens[i].trimmed());
}

QString Character::DumpCustom()
{
    QString ret;
    bool first = true;
    for (const auto &value : std::as_const(CustomProperties)) {
        if (first) {
            first = false;
            ret += value;
            continue;
        }

        ret += QLatin1String(" ; ");
        ret += value;
    }
    return ret;
}

/// Sequences

void MatchSequence::SetMatch(const QString &b)
{
    /*
     The byte array looks like :
        "U1111(prop1, prop2, prop3)U2222.(prop4)"
    And it has to be turned into :
        QList(
            Character(U+1111).CustomProperties["prop1", "prop2", "prop3"],
            Character(U+2222),
            Character(`\0`).CustomProperties["prop4"]
        )

    The null character will match only on properties. So dot its quite the same as in REGEX.
    */

    QString ref(b);
    // 	qDebug()<<"SetMatch("+ref+")";
    for (int idx(0); idx < ref.size(); ++idx) {
        QChar current(ref[idx]);
        if (current == QLatin1Char('U')) // a code point
        {
            bool ok;
            ++idx;
            int unicode = ref.mid(idx, 4).toInt(&ok, 16);
            if (!ok)
                qCDebug(FONTMATRIX_LOG) << "Oops";
            idx += 4;
            if (idx < ref.size() && ref[idx] == QLatin1Char('(')) // property list
            {
                QStringList pList;
                int countChars(0);
                while (idx + countChars < ref.size() && ref[idx + countChars] != QLatin1Char(')')) {
                    ++countChars;
                }
                QStringList pl(ref.mid(idx + 1, countChars - 1).split(QStringLiteral(";"), Qt::SkipEmptyParts));
                for (const auto &prop : std::as_const(pl)) {
                    pList << prop.trimmed();
                }
                idx += countChars;
                Properties << Character(unicode, pList);
            } else if (idx < ref.size() && ref[idx] == QLatin1Char('[')) // property list with exact match
            {
                QStringList pList;
                int countChars(0);
                while (idx + countChars < ref.size() && ref[idx + countChars] != QLatin1Char(']')) {
                    ++countChars;
                }
                QStringList pl(ref.mid(idx + 1, countChars - 1).split(QStringLiteral(";"), Qt::SkipEmptyParts));
                for (const auto &prop : std::as_const(pl)) {
                    pList << prop.trimmed();
                }
                idx += countChars;
                Properties << Character(unicode, pList);
                Properties.last().MatchAll = true;
            } else // if(ref[idx] != QLatin1Char('(') || ref[idx] != QLatin1Char('['))
            {
                --idx;
                Properties << Character(unicode);
            }

        } else if (current == QLatin1Char('.')) // a null char (can have properties)
        {
            int unicode = 0;
            ++idx;

            if (idx < ref.size() && ref[idx] == QLatin1Char('(')) // property list
            {
                QStringList pList;
                int countChars(0);
                while (idx + countChars < ref.size() && ref[idx + countChars] != QLatin1Char(')')) {
                    ++countChars;
                }
                QStringList pl(ref.mid(idx + 1, countChars - 1).split(QStringLiteral(";"), Qt::SkipEmptyParts));
                for (const auto &prop : std::as_const(pl)) {
                    pList << prop.trimmed();
                }
                idx += countChars;
                Properties << Character(unicode, pList);
            } else if (idx < ref.size() && ref[idx] == QLatin1Char('[')) // property list with exact match
            {
                QStringList pList;
                int countChars(0);
                while (idx + countChars < ref.size() && ref[idx + countChars] != QLatin1Char(']')) {
                    ++countChars;
                }
                QStringList pl(ref.mid(idx + 1, countChars - 1).split(QStringLiteral(";"), Qt::SkipEmptyParts));
                for (const auto &prop : std::as_const(pl)) {
                    pList << prop.trimmed();
                }
                idx += countChars;
                Properties << Character(unicode, pList);
                Properties.last().MatchAll = true;
            } else // if(ref[idx] != QLatin1Char('(')|| ref[idx] != QLatin1Char('['))
            {
                --idx;
                Properties << Character(unicode);
            }
        } else {
            // Error
            qCWarning(FONTMATRIX_LOG) << "ERROR match: current = " << current;
        }
    }
}

void ReplaceSequence::SetReplace(const QString &b)
{
    QString ref(b);
    // 	qDebug()<<"SetReplace("+ref+")";
    for (int idx(0); idx < ref.size(); ++idx) {
        QChar current(ref[idx]);
        if (current == QLatin1Char('U')) // a code point
        {
            bool ok;
            ++idx;
            int unicode = ref.mid(idx, 4).toInt(&ok, 16);
            if (!ok)
                qCDebug(FONTMATRIX_LOG) << "Oops";
            idx += 4;
            if (idx >= ref.size() || ref[idx] != QLatin1Char('(')) {
                --idx;
                Properties << Character(unicode);
            } else // property list
            {
                QStringList pList;
                int countChars(0);
                while (idx + countChars < ref.size() && ref[idx + countChars] != QLatin1Char(')')) {
                    ++countChars;
                }
                QStringList pl(ref.mid(idx + 1, countChars - 1).split(QStringLiteral(";"), Qt::SkipEmptyParts));
                for (const auto &prop : std::as_const(pl)) {
                    pList << prop.trimmed();
                }
                idx += countChars;
                Properties << Character(unicode, pList);
            }

        } else if (current == QLatin1Char('.')) // a null char (can have properties)
        {
            int unicode = 0;
            Properties << Character(unicode);
            ++idx;

            bool ok;
            int group = ref.mid(idx, 1).toInt(&ok, 10);
            Properties.last().GroupIndex = group;
            ++idx;

            if (idx >= ref.size() || ref[idx] != QLatin1Char('(')) {
                --idx;

            } else // property list
            {
                QStringList pList;
                int countChars(0);
                while (idx + countChars < ref.size() && ref[idx + countChars] != QLatin1Char(')')) {
                    ++countChars;
                }
                QStringList pl(ref.mid(idx + 1, countChars - 1).split(QStringLiteral(";"), Qt::SkipEmptyParts));
                for (const auto &prop : std::as_const(pl)) {
                    pList << prop.trimmed();
                }
                idx += countChars;
                Properties.last().CustomProperties = pList;
            }
        } else {
            // Error
            qCWarning(FONTMATRIX_LOG) << "ERROR replace: current = " << current;
        }
    }
}

QList<Character> FMOwnShaper::GetShaped()
{
    Out.clear();
    Op();
    return Out;
}

QString FMOwnShaper::CleanRule(QString rule)
{
    QString ret;
    int len(rule.size());
    for (int i(0); i < len; ++i) {
        if (!rule[i].isSpace())
            ret += rule[i];
    }
    return ret;
}

void Character::AddProperty(const QString &prop)
{
    if (!CustomProperties.contains(prop)) {
        CustomProperties << prop;
    }
}
