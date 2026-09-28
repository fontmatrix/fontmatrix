/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmfontdb.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "typotek.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>

FMFontDb *FMFontDb::instance = nullptr;
FMFontDb::FMFontDb()
    : QSqlDatabase(typotek::getInstance()->getDatabaseDriver())
{
    fieldName[FontId] = QLatin1String("fontident");
    fieldName[Id] = QLatin1String("digitident");
    fieldName[Tags] = QLatin1String("tag");
    fieldName[Lang] = QLatin1String("lang");
    fieldName[InfoKey] = QLatin1String("infokey");
    fieldName[InfoValue] = QLatin1String("infovalue");
    fieldName[Type] = QLatin1String("type");
    fieldName[FsType] = QLatin1String("fstype");
    fieldName[Family] = QLatin1String("family");
    fieldName[Variant] = QLatin1String("variant");
    fieldName[Name] = QLatin1String("name");
    fieldName[Panose] = QLatin1String("panose");
    fieldName[Activation] = QLatin1String("activation");

    tableName[InternalId] = QLatin1String("fontmatrix_id");
    tableName[Data] = QLatin1String("fontmatrix_data");
    tableName[Tag] = QLatin1String("fontmatrix_tags");
    tableName[Info] = QLatin1String("fontmatrix_info");

    transactionDeep = 0;
    tagsCacheValid = false;

    priorList << QStringLiteral("Book") << QStringLiteral("Normal") << QStringLiteral("Regular") << QStringLiteral("Roman") << QStringLiteral("Plain")
              << QStringLiteral("Medium");
}

FMFontDb *FMFontDb::DB()
{
    // A QSqlDatabase connection belongs to the thread that opened it
    Q_ASSERT(!qApp || QThread::currentThread() == qApp->thread());
    if (!instance) {
        instance = new FMFontDb;
        instance->initFMDb();
        Q_ASSERT(instance);
    }
    return instance;
}

bool FMFontDb::execBound(QSqlQuery &query, const QString &statement, const QVariantList &values)
{
    // Identifiers are composed by the caller, every value is bound here.
    if (!query.prepare(statement)) {
        qCWarning(FONTMATRIX_LOG) << "Cannot prepare" << statement << query.lastError().text();
        if (transactionDeep > 0)
            transactionError << query.lastError();
        return false;
    }
    for (const auto &v : values)
        query.addBindValue(v);
    if (!query.exec()) {
        qCWarning(FONTMATRIX_LOG) << "Cannot execute" << statement << values << query.lastError().text();
        if (transactionDeep > 0)
            transactionError << query.lastError();
        return false;
    }
    return true;
}

void FMFontDb::invalidateTags(const QString &id)
{
    tagsCacheValid = false;
    if (id.isEmpty())
        rValueCache.clear();
    else
        rValueCache.remove(id);
}

void FMFontDb::initRecord(const QString &id)
{
    // 	qDebug()<<"initRecord"<<id;
    int nId(++internalCounter);

    QString qs1(QStringLiteral("INSERT INTO %1(%2,%3) VALUES(?,?)").arg(tableName.value(InternalId), fieldName.value(FontId), fieldName.value(Id)));

    QSqlQuery query(*this);
    // if not in internal id, no need to go further
    if (execBound(query, qs1, QVariantList() << id << nId)) {
        QString qs2(QStringLiteral("INSERT INTO %1(%2) VALUES(?)").arg(tableName.value(Data), fieldName.value(Id)));
        execBound(query, qs2, QVariantList() << nId);
    }

    cacheId[id] = nId;
    reverseCacheId[nId] = id;
}

void FMFontDb::setValue(const QString &id, Field field, QVariant value)
{
    // 	qDebug()<<"setValue"<<id<<fieldName.value ( field )<<value;
    int nId(getId(id));
    // 	transaction();
    if (field == Tags) {
        setTags(id, value.toStringList());
    } else {
        QString qs(QStringLiteral("UPDATE %1 SET %2=? WHERE %3=?").arg(tableName.value(Data), fieldName.value(field), fieldName.value(Id)));
        QSqlQuery query(*this);
        execBound(query, qs, QVariantList() << value << nId);
        rValueCache.remove(id);
    }
}

void FMFontDb::setValues(const QString &id, QList<Field> fields, QVariantList values)
{
    int nId(getId(id));
    // 	transaction();
    QSqlQuery query(*this);
    for (int i(0); i < fields.count(); ++i) {
        QString qs(QStringLiteral("UPDATE %1 SET %2=? WHERE %3=?").arg(tableName.value(Data), fieldName.value(fields[i]), fieldName.value(Id)));
        if (!execBound(query, qs, QVariantList() << values[i] << nId))
            break;
    }
    rValueCache.remove(id);
}

void FMFontDb::setInfoMap(const QString &id, const QMap<int, QMap<int, QString>> &info)
{
    // 	qDebug()<<"setInfoMap"<<id;
    // Here is the interesting part :-s)

    // id | lang | key | value
    int nId(getId(id));
    QVariantList idlist;
    QVariantList langlist;
    QVariantList keylist;
    QVariantList valuelist;
    QString qs ( QStringLiteral ( "INSERT INTO %1 VALUES(?,?,?,?)" )
	             .arg ( tableName.value ( Info ) )
	             /*.arg(fieldName.value ( Id ))
	             .arg(fieldName.value ( Lang ))
	             .arg(fieldName.value ( InfoKey ))
	             .arg(fieldName.value ( InfoValue ))*/ );
    QSqlQuery query(*this);
    query.prepare(qs);
    for (auto langIt(info.constBegin()); langIt != info.constEnd(); ++langIt) {
        const QMap<int, QString> &keys(langIt.value());
        for (auto keyIt(keys.constBegin()); keyIt != keys.constEnd(); ++keyIt) {
            idlist << nId;
            langlist << langIt.key();
            keylist << keyIt.key();
            valuelist << keyIt.value();
        }
    }
    query.addBindValue(idlist);
    query.addBindValue(langlist);
    query.addBindValue(keylist);
    query.addBindValue(valuelist);

    if (!query.execBatch()) {
        qCWarning(FONTMATRIX_LOG) << "Cannot execute" << qs << query.lastError().text();
        if (transactionDeep > 0)
            transactionError << query.lastError();
    }
}

QVariant FMFontDb::getValue(const QString &id, Field field, bool useCache)
{
    // 	qDebug() <<"getValue"<< fieldName.value ( field ) <<id;
    if (useCache) {
        if (rValueCache.contains(id)) {
            if (rValueCache.value(id).contains(field)) {
                // 			qDebug() <<"getCachedValue"<< fieldName.value ( field ) <<id <<rValueCache[id][field];
                return rValueCache[id][field];
            }
        } else
            rValueCache.clear();
    } else
        rValueCache.clear();

    if (field == Tags) {
        QStringList tl;
        QString qs(QStringLiteral("SELECT %1 FROM %2 WHERE %3=?").arg(fieldName.value(field), tableName.value(Tag), fieldName.value(Id)));
        QSqlQuery query(*this);
        if (execBound(query, qs, QVariantList() << getId(id))) {
            while (query.next()) {
                QString t(query.value(0).toString());
                if (!tl.contains(t))
                    tl << t;
            }
        }
        rValueCache[id][field] = tl;
        return tl;
    } else {
        QString qs(QStringLiteral("SELECT %1 FROM %2 WHERE %3=?").arg(fieldName.value(field), tableName.value(Data), fieldName.value(Id)));
        QSqlQuery query(*this);
        if (execBound(query, qs, QVariantList() << getId(id))) {
            if (query.first()) {
                rValueCache[id][field] = query.value(0);
                return query.value(0);
            }
        }
        return QVariant();
    }
}

QList<FontDBResult> FMFontDb::getValues(Field field, const QList<FontItem *> &fonts)
{
    QList<FontItem *> lF(fonts);
    if (fonts.isEmpty())
        lF = AllFonts();
    // TODO really filter out results according to the font list given in argument
    QList<FontDBResult> ret;
    QString qs(QStringLiteral("SELECT %1,%2 FROM %3").arg(fieldName.value(Id), fieldName.value(field), tableName.value(Data)));
    QSqlQuery query(*this);

    if (!execBound(query, qs))
        return ret;
    else {
        while (query.next()) {
            int id(query.value(0).toInt());
            if (id > 0) {
                FontDBResult fr = qMakePair(fontMap.value(id), query.value(1).toString());
                if (fr.first)
                    ret << fr;
            }
        }
        return ret;
    }
}

FontInfoMap FMFontDb::getInfoMap(const QString &id)
{
    // 	qDebug() <<"getInfoMap"<<id;
    FontInfoMap ret;
    if (id.isEmpty()) {
        qCDebug(FONTMATRIX_LOG) << "No such font is referenced";
        return ret;
    }
    if (temporaryFont.contains(id)) {
        qCDebug(FONTMATRIX_LOG) << "It is a temp Id";
        return temporaryFont.value(id)->moreInfo();
    }
    QString qs(QStringLiteral("SELECT * FROM %1 WHERE %2=?").arg(tableName.value(Info), fieldName.value(Id)));
    QSqlQuery query(*this);
    if (execBound(query, qs, QVariantList() << getId(id))) {
        int lIdx(query.record().indexOf(fieldName.value(Lang)));
        int kIdx(query.record().indexOf(fieldName.value(InfoKey)));
        int vIdx(query.record().indexOf(fieldName.value(InfoValue)));
        while (query.next()) {
            ret[query.value(lIdx).toInt()][query.value(kIdx).toInt()] = query.value(vIdx).toString();
        }
    } else {
        qCDebug(FONTMATRIX_LOG) << "Error executing query";
    }
    return ret;
}

QList<FontDBResult> FMFontDb::getInfo([[maybe_unused]] const QList<FontItem *> &fonts, InfoItem info, int codeLang)
{
    QList<FontDBResult> ret;
    // a name comes once per platform and language; a negative codeLang takes them all
    QString qs(QStringLiteral("SELECT %1,%2 FROM %3 WHERE (%4=?)")
                   .arg(fieldName.value(Id), fieldName.value(InfoValue), tableName.value(Info), fieldName.value(InfoKey)));
    QVariantList values;
    values << int(info);
    if (codeLang >= 0) {
        qs += QStringLiteral(" AND (%1=?)").arg(fieldName.value(Lang));
        values << codeLang;
    }
    QSqlQuery query(*this);
    if (!execBound(query, qs, values))
        return ret;
    else {
        while (query.next()) {
            int id(query.value(0).toInt());
            if (id > 0) {
                FontDBResult fr = qMakePair(fontMap.value(id), query.value(1).toString());
                if (fr.first)
                    ret << fr;
            }
        }
        return ret;
    }
}

void FMFontDb::addTag(const QString &id, const QString &t)
{
    int nId(getId(id));
    QString ts(QStringLiteral("INSERT INTO %1(%2,%3) VALUES(?,?)").arg(tableName.value(Tag), fieldName.value(Id), fieldName.value(Tags)));
    QSqlQuery query(*this);
    execBound(query, ts, QVariantList() << nId << t);
    invalidateTags(id);
}

void FMFontDb::addTag(const QStringList &idlist, const QString &t)
{
    QVariantList nidlist;
    QVariantList taglist;
    QString qs(QStringLiteral("INSERT INTO %1(%2,%3) VALUES(?,?)").arg(tableName.value(Tag), fieldName.value(Id), fieldName.value(Tags)));
    QSqlQuery query(*this);
    query.prepare(qs);
    for (const auto &id : idlist) {
        nidlist << getId(id);
        taglist << t;
    }
    invalidateTags();
    query.addBindValue(nidlist);
    query.addBindValue(taglist);

    if (!query.execBatch()) {
        qCWarning(FONTMATRIX_LOG) << "Cannot execute" << qs << query.lastError().text();
        if (transactionDeep > 0)
            transactionError << query.lastError();
    }
}

void FMFontDb::removeTag(const QString &id, const QString &t)
{
    int nId(getId(id));
    QString qs(QStringLiteral("DELETE FROM %1 WHERE (%2=?) AND (%3=?)").arg(tableName.value(Tag), fieldName.value(Id), fieldName.value(Tags)));
    QSqlQuery query(*this);
    execBound(query, qs, QVariantList() << nId << t);
    invalidateTags(id);
}

void FMFontDb::setTags(const QString &id, const QStringList &tl)
{
    int nId(getId(id));
    QString qs(QStringLiteral("DELETE FROM %1 WHERE %2=?").arg(tableName.value(Tag), fieldName.value(Id)));
    QSqlQuery query(*this);
    execBound(query, qs, QVariantList() << nId);
    invalidateTags(id);
    // 	TransactionBegin();
    for (const auto &t : tl) {
        addTag(id, t);
    }
    // 	TransactionEnd();
}

QStringList FMFontDb::getTags()
{
    // 	qDebug() <<"getTags";
    if (tagsCacheValid)
        return tagsCache;

    tagsCache.clear();
    QString qs(QStringLiteral("SELECT %1 FROM %2").arg(fieldName.value(Tags), tableName.value(Tag)));
    QSqlQuery query(*this);
    if (execBound(query, qs)) {
        while (query.next())
            tagsCache << query.value(0).toString();
        tagsCache.removeDuplicates();
        tagsCache.sort();
        tagsCacheValid = true;
    }
    return tagsCache;
}

void FMFontDb::announceTagsChanged()
{
    invalidateTags();
    Q_EMIT tagsChanged();
}

void FMFontDb::addTagToDB(const QString &t)
{
    // 	qDebug() << "addtag"<< t;
    QString vs(QStringLiteral("INSERT INTO %1(%2,%3) VALUES(?,?)").arg(tableName.value(Tag), fieldName.value(Id), fieldName.value(Tags)));
    QSqlQuery query(*this);
    execBound(query, vs, QVariantList() << 0 << t);
    invalidateTags();
    Q_EMIT tagsChanged();
}

void FMFontDb::removeTagFromDB(const QString &t)
{
    QString qs(QStringLiteral("DELETE FROM %1 WHERE %2=?").arg(tableName.value(Tag), fieldName.value(Tags)));
    QSqlQuery query(*this);
    execBound(query, qs, QVariantList() << t);
    invalidateTags();
    Q_EMIT tagsChanged();
}

void FMFontDb::editTag(const QString &tOld, const QString &tNew)
{
    QString qs(QStringLiteral("UPDATE %1 SET %2=? WHERE %2=?").arg(tableName.value(Tag), fieldName.value(Tags)));
    QSqlQuery query(*this);
    execBound(query, qs, QVariantList() << tNew << tOld);
    invalidateTags();
    Q_EMIT tagsChanged();
}

void FMFontDb::initFMDb()
{
    // 	qDebug() <<"initFMDb";

    setHostName(typotek::getInstance()->getDatabaseHostname());
    setDatabaseName(typotek::getInstance()->getDatabaseDbName());
    setUserName(typotek::getInstance()->getDatabaseUser());
    setPassword(typotek::getInstance()->getDatabasePassword());
    if (!open()) {
        qCWarning(FONTMATRIX_LOG) << "Connection to" << hostName() << "::" << databaseName() << "failed miserably!";
        qCWarning(FONTMATRIX_LOG) << "====================================================================";
        qCWarning(FONTMATRIX_LOG) << lastError();
        qCWarning(FONTMATRIX_LOG) << "====================================================================";
        return;
    } else
        qCDebug(FONTMATRIX_LOG) << "Connection to" << hostName() << "::" << databaseName() << "SUCCESS!";

    QStringList tl(tables(QSql::Tables));
    bool allIsAlreadyHere(true);
    for (const auto tableNameValues = tableName.values(); const auto &tn : tableNameValues) {
        if (!tl.contains(tn)) {
            allIsAlreadyHere = false;
            break;
        }
    }

    if (!allIsAlreadyHere) {
        // We want to create the tables then!
        QString fId(QStringLiteral("%1 CHAR(255) ").arg(fieldName.value(FontId)));
        QString fNumId(QStringLiteral("%1 INTEGER").arg(fieldName.value(Id)));

        QString fFamily(QStringLiteral("%1 CHAR(255) ").arg(fieldName.value(Family)));
        QString fVariant(QStringLiteral("%1 CHAR(255) ").arg(fieldName.value(Variant)));
        QString fName(QStringLiteral("%1 CHAR(255) ").arg(fieldName.value(Name)));
        QString fType(QStringLiteral("%1 CHAR(32) ").arg(fieldName.value(Type)));
        QString fPanose(QStringLiteral("%1 CHAR(32) ").arg(fieldName.value(Panose)));
        QString fFsType(QStringLiteral("%1 INTEGER ").arg(fieldName.value(FsType)));
        QString fActivation(QStringLiteral("%1 INTEGER ").arg(fieldName.value(Activation)));

        QString fLang(QStringLiteral("%1 INTEGER ").arg(fieldName.value(Lang)));
        QString fInfoKey(QStringLiteral("%1 INTEGER ").arg(fieldName.value(InfoKey)));
        QString fInfoValue(QStringLiteral("%1 TEXT").arg(fieldName.value(InfoValue)));

        QString fTags(QStringLiteral("%1 CHAR(255) ").arg(fieldName.value(Tags)));

        QString cData(QStringLiteral("CREATE TABLE %1 (%2,%3,%4,%5,%6,%7,%8,%9)")
                          .arg(tableName.value(Data), fNumId, fFamily, fVariant, fName, fType, fPanose, fFsType, fActivation));
        QString iData(QStringLiteral("CREATE INDEX iData ON %1(%2)").arg(tableName.value(Data), fieldName.value(Id)));

        QString cInfo(QStringLiteral("CREATE TABLE %1 (%2,%3,%4,%5)").arg(tableName.value(Info), fNumId, fLang, fInfoKey, fInfoValue));
        QString iInfo(QStringLiteral("CREATE INDEX iInfo ON %1(%2)").arg(tableName.value(Info), fieldName.value(Id)));

        QString cTag(QStringLiteral("CREATE TABLE %1 (%2,%3)").arg(tableName.value(Tag), fNumId, fTags));
        QString iTag(QStringLiteral("CREATE INDEX iTag ON %1(%2)").arg(tableName.value(Tag), fieldName.value(Id)));

        QString cId(QStringLiteral("CREATE TABLE %1 (%2,%3)").arg(tableName.value(InternalId), fId, fNumId));
        QString iId(QStringLiteral("CREATE INDEX iId ON %1(%2)").arg(tableName.value(InternalId), fieldName.value(FontId)));

        QSqlQuery query(*this);
        if (!query.exec(cData))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << cData << "\n---------------------------------\n" << query.lastError().databaseText();
        if (!query.exec(iData))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << iData << "\n---------------------------------\n" << query.lastError().databaseText();
        if (!query.exec(cInfo))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << cInfo << "\n---------------------------------\n" << query.lastError().databaseText();
        if (!query.exec(iInfo))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << iInfo << "\n---------------------------------\n" << query.lastError().databaseText();
        if (!query.exec(cTag))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << cTag << "\n---------------------------------\n" << query.lastError().databaseText();
        if (!query.exec(iTag))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << iTag << "\n---------------------------------\n" << query.lastError().databaseText();
        if (!query.exec(cId))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << cId << "\n---------------------------------\n" << query.lastError().databaseText();
        if (!query.exec(iId))
            qCWarning(FONTMATRIX_LOG) << "ERROR:" << iId << "\n---------------------------------\n" << query.lastError().databaseText();

        internalCounter = 0;
    } else {
        internalCounter = 0;
        QSqlQuery query(*this);
        if (query.exec(QStringLiteral("SELECT MAX(%1) FROM %2").arg(fieldName.value(Id), tableName.value(InternalId)))) {
            if (query.first()) {
                internalCounter = query.value(0).toInt();
            }
        }
        bool rq(false);
        /// build ID caches
        QString qs1(QStringLiteral("SELECT * FROM %1 "));
        rq = query.exec(qs1.arg(tableName.value(InternalId)));
        if (!rq) {
            qCWarning(FONTMATRIX_LOG) << query.lastQuery();
            qCWarning(FONTMATRIX_LOG) << lastError();
            return;
        } else {
            int idxdigit(query.record().indexOf(fieldName.value(Id)));
            int idxfont(query.record().indexOf(fieldName.value(FontId)));
            while (query.next()) {
                cacheId[query.value(idxfont).toString()] = query.value(idxdigit).toInt();
                reverseCacheId[query.value(idxdigit).toInt()] = query.value(idxfont).toString();
            }
        }

        /// build memory font database
        //  which is ment to be a lightweight version of the
        // file DB and act as a proxy for most of requests.
        QString qs2(QStringLiteral("SELECT %1,%2,%3,%4,%5 FROM %6"));
        rq = query.exec(qs2.arg(fieldName.value(Id),
                                fieldName.value(Family),
                                fieldName.value(Variant),
                                fieldName.value(Type),
                                fieldName.value(Activation),
                                tableName.value(Data)));
        if (!rq) {
            qCWarning(FONTMATRIX_LOG) << query.lastQuery();
            qCWarning(FONTMATRIX_LOG) << lastError();
            return;
        } else {
            int anId(0);
            QString path;
            bool act(false);
            while (query.next()) {
                anId = query.value(0).toInt();
                path = reverseCacheId[anId];
                act = (query.value(4).toInt() == 0) ? false : true;
                if ((path.isEmpty()) || (anId == 0))
                    continue;
                // 				qDebug()<<anId<<++counter<<path;
                if (fontMap.contains(anId))
                    continue;
                else
                    fontMap[anId] = new FontItem(path, query.value(1).toString(), query.value(2).toString(), query.value(3).toString(), act);
            }
        }
    }
}

int FMFontDb::getId(const QString &fontid)
{
    return cacheId.value(fontid);
}

FontItem *FMFontDb::Font(const QString &id, bool noTemporary)
{
    // 	qDebug() <<"Font"<<id;
    FontItem *fitem(nullptr);
    if (id.isEmpty())
        return fitem;
    if (temporaryFont.contains(id)) {
        if (noTemporary) {
            delete temporaryFont.value(id);
            temporaryFont.remove(id);
        } else
            return temporaryFont.value(id);
    }
    int fid(getId(id));
    if (fid > 0) {
        if (fontMap.contains(fid)) {
            fitem = fontMap.value(fid);
            fitem->updateItem();
        } else
            qCWarning(FONTMATRIX_LOG) << "ERROR fetching font item" << id;
    } else {
        // 		qDebug() <<"New font"<< id;
        fitem = AddFont(new FontItem(id));
    }
    return fitem;
}

bool FMFontDb::Knows(const QString &id)
{
    return !id.isEmpty() && getId(id) > 0;
}

FontItem *FMFontDb::AddFont(FontItem *fitem)
{
    if (!fitem)
        return nullptr;
    const QString id(fitem->path());
    if (fitem->isValid()) {
        fitem->dumpIntoDB();
        const int fid(getId(id));
        if (fid > 0) {
            fontMap[fid] = fitem;
            return fitem;
        }
    }
    delete fitem;
    qCWarning(FONTMATRIX_LOG) << "ERROR creating font item" << id;
    return nullptr;
}

QList<FontItem *> FMFontDb::AllFonts()
{
    // 	if(!fontMap.isEmpty())
    return fontMap.values();
}

QStringList FMFontDb::AllFontNames()
{
    return cacheId.keys();
}

QList<FontItem *> FMFontDb::FamilySet(const QString &family)
{
    QList<FontItem *> ret;
    for (const auto fontMapValues = fontMap.values(); auto *f : fontMapValues) {
        if (f->family() == family)
            ret << f;
    }
    return ret;
}

void FMFontDb::TransactionBegin()
{
    if (transactionDeep > 0)
        ++transactionDeep;
    else {
        if (transaction()) {
            transactionError.clear();
            ++transactionDeep;
        } else
            qCWarning(FONTMATRIX_LOG) << "Cannot BEGIN transaction";
        // 		qDebug() <<"TransactionBegin";
    }
}

bool FMFontDb::TransactionEnd()
{
    // 	qDebug() <<"TransactionEnd"<< (transactionDeep - 1);

    if (transactionDeep <= 0) {
        // TransactionBegin() failed, or was not called
        transactionDeep = 0;
        return false;
    }
    --transactionDeep;
    if (transactionDeep > 0)
        return true;

    // What went through is kept. The statements of one font do not depend on
    // those of another, and the fonts sit in the caches already: a rollback
    // would take them out of the file only, until the next start. Each failure
    // has been reported where it happened.
    const bool clean(transactionError.isEmpty());
    if (!clean)
        qCWarning(FONTMATRIX_LOG) << transactionError.count() << "statements of this transaction failed";
    transactionError.clear();
    if (!commit()) {
        qCWarning(FONTMATRIX_LOG) << "Cannot COMMIT" << lastError().text();
        return false;
    }
    return clean;
}

int FMFontDb::FontCount()
{
    QString qs(QStringLiteral("SELECT COUNT(%1) FROM %2 "));
    QSqlQuery query(*this);
    if (query.exec(qs.arg(fieldName.value(Id), tableName.value(InternalId)))) {
        if (query.first()) {
            return query.value(0).toInt();
        }
    } else
        qCWarning(FONTMATRIX_LOG) << query.lastError();
    return 0;
}

QList<FontItem *> FMFontDb::Fonts(const QVariant &pattern, Field field)
{
    if ((field == Family) || (field == Type) || (field == Variant) || (field == Name)
        /*|| ( field == Panose ) */)
        return Fonts(QStringLiteral("%1=?").arg(fieldName.value(field)), QVariantList() << pattern.toString(), Data);
    else if (field == Tags)
        return Fonts(QStringLiteral("%1=?").arg(fieldName.value(field)), QVariantList() << pattern.toString(), Tag);
    else if (field == Activation || (field == FsType))
        return Fonts(QStringLiteral("%1=?").arg(fieldName.value(field)), QVariantList() << pattern.toInt(), Data);
    else
        return QList<FontItem *>();
}

QList<FontItem *> FMFontDb::Fonts(const QVariant &pattern, InfoItem info, int codeLang)
{
    // A name comes once per platform and language: 0 is Macintosh English,
    // 1033 Windows US English, and a third of the fonts only have the second.
    // A negative codeLang looks in all of them.
    QString qs(QStringLiteral("(%1=?) AND (%2 LIKE ?)").arg(fieldName.value(InfoKey), fieldName.value(InfoValue)));
    QVariantList values;
    values << int(info) << QString(QLatin1Char('%') + pattern.toString() + QLatin1Char('%'));
    if (codeLang >= 0) {
        qs += QStringLiteral(" AND (%1=?)").arg(fieldName.value(Lang));
        values << codeLang;
    }
    return Fonts(qs, values, Info);
}

QList<FontItem *> FMFontDb::Fonts(const QString &whereString, const QVariantList &values, Table table)
{
    //	QList< FontItem * > ret;
    QString qs(QStringLiteral("SELECT %1 FROM %2 WHERE ").arg(fieldName.value(Id), tableName.value(table)) + whereString);
    QSqlQuery query(*this);
    if (!execBound(query, qs, values))
        return QList<FontItem *>();
    else {
        //		if(!familyOnly)
        {
            QMap<int, FontItem *> reg;
            while (query.next()) {
                int id(query.value(0).toInt());
                if (id > 0) {
                    if (!fontMap.value(id))
                        qCWarning(FONTMATRIX_LOG) << "ERROR : DB contains references to id" << id << "which is not in fontmap";
                    else {
                        reg[id] = fontMap.value(id);
                    }
                }
            }
            return reg.values();
        }
    }
}

bool FMFontDb::Remove(const QString &id)
{
    // TODO  implement remove
    // 	return true;
    // 	qDebug()<<"Remove"<<id;
    int nId(getId(id));

    fontMap.remove(nId);
    reverseCacheId.remove(nId);
    cacheId.remove(id);

    invalidateTags(id);

    // Every table is tried even after a failure, so that a font never
    // survives in one of them only.
    bool res(true);
    QSqlQuery query(*this);
    for (const Table t : {Tag, Info, Data, InternalId}) {
        QString qs(QStringLiteral("DELETE FROM %1 WHERE %2=?").arg(tableName.value(t), fieldName.value(Id)));
        if (!execBound(query, qs, QVariantList() << nId))
            res = false;
    }

    return res;
}

// FontItem * FMFontDb::FirstFont()
// {
// }
//
// FontItem * FMFontDb::NextFont()
// {
// }

bool FMFontDb::insertTemporaryFont(const QString &path)
{
    // basic check
    // 	qDebug() <<"TFont"<<path;
    if (path.isEmpty())
        return false;

    QFileInfo fi(path);
    QString absPath(fi.absoluteFilePath());

    if (temporaryFont.contains(path)) {
        return true;
    }

    // check if we have it yet
    int fid(getId(absPath));
    if (fid > 0) {
        if (fontMap.contains(fid)) {
            return true;
        }
    }

    // Build an item
    auto item(new FontItem(absPath));
    if (!item->isValid()) {
        delete item;
        return false;
    }
    temporaryFont[absPath] = item;
    // 	item->lock();
    return true;
}

void FMFontDb::clearFilteredFonts()
{
    currentFonts.clear();
    currentFamiliesCache.clear();
}

QList<FontItem *> FMFontDb::getFilteredFonts(bool familyOnly)
{
    if (!familyOnly)
        return currentFonts;

    if (currentFamiliesCache.isEmpty() && (!currentFonts.isEmpty())) {
        QMap<QString, QList<FontItem *>> pools;
        for (auto *it : std::as_const(currentFonts)) {
            if (pools.contains(it->family()))
                pools[it->family()].append(it);
            else {
                QList<FontItem *> tl;
                tl.append(it);
                pools[it->family()] = tl;
            }
        }

        for (const auto poolsKeys = pools.keys(); const auto &k : poolsKeys) {
            FontItem *sel = pools[k].first();
            for (const auto range = pools[k]; auto *it : range) {
                if (priorList.contains(it->variant(), Qt::CaseInsensitive)) {
                    sel = it;
                    break;
                }
            }
            currentFamiliesCache << sel;
        }
    }
    return currentFamiliesCache;
}

void FMFontDb::setFilterdFonts(const QList<FontItem *> &flist)
{
    currentFonts.clear();
    currentFamiliesCache.clear();
    currentFonts = flist;
}

int FMFontDb::countFilteredFonts() const
{
    return currentFonts.count();
}

void FMFontDb::insertFilteredFont(FontItem *item)
{
    if ((item != nullptr) && (!currentFonts.contains(item))) {
        currentFonts.append(item);
        currentFamiliesCache.clear();
    }
}

void FMFontDb::removeFilteredFont(FontItem *item)
{
    if (item != nullptr) {
        currentFonts.removeAll(item);
        currentFamiliesCache.clear();
    }
}

bool FMFontDb::isFiltered(FontItem *item) const
{
    return currentFonts.contains(item);
}

void FMFontDb::filterAllFonts()
{
    currentFonts.clear();
    currentFamiliesCache.clear();
    currentFonts = fontMap.values();
}

#include "moc_fmfontdb.cpp"
