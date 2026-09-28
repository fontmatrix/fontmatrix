/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMFONTDB_H
#define FMFONTDB_H

#include "fmsharestruct.h"

#include <QMap>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QVariant>

/**
 * An effort to migrate font storage to proper database.
 * We expect to avoid the growing memory print when running
 * and speedup starting of the program.
 *
 *
 */

class FontItem;
class QSqlQuery;

using FontDBResult = QPair<FontItem *, QString>;

class FMFontDb : public QObject, public QSqlDatabase
{
    Q_OBJECT
    // No surprise, we want it to be a singleton
    static FMFontDb *instance;
    FMFontDb();
    ~FMFontDb() override = default;
    enum Table {
        InternalId = 0,
        Data = 10,
        Tag,
        Info,
        TOs2
    };

public:
    enum Field {
        FontId = 0,
        Id = 1,
        Activation = 2,
        Type = 10,
        Family,
        Variant,
        Name,
        Panose,
        // 15 was FileSize, which never had a column
        FsType = 16,
        Tags,
        Lang,
        InfoKey,
        InfoValue,
    };
    enum InfoItem {
        Copyright = 0,
        FontFamily = 1,
        FontSubfamily = 2,
        UniqueFontIdentifier = 3,
        FullFontName = 4,
        VersionString = 5,
        PostscriptName = 6,
        Trademark = 7,
        ManufacturerName = 8,
        Designer = 9,
        Description = 10,
        URLVendor = 11,
        URLDesigner = 12,
        LicenseDescription = 13,
        LicenseInfoURL = 14,
        // 					 	Reserved=15,
        PreferredFamily = 16,
        PreferredSubfamily = 17,
        CompatibleMacintosh = 18,
        SampleText = 19,
        PostScriptCIDName = 20,
        AllInfo = 99
    };

    /// Here are somehow "actual" public api
    static FMFontDb *DB();
    void TransactionBegin();
    bool TransactionEnd();

    void initRecord(const QString &id);
    void setValue(const QString &id, Field field, QVariant value);
    void setValues(const QString &id, QList<Field> fields, QVariantList values);

    void setInfoMap(const QString &id, const FontInfoMap &info);

    QVariant getValue(const QString &id, Field field, bool useCache = true);
    QList<FontDBResult> getValues(Field field, const QList<FontItem *> &fonts = QList<FontItem *>());
    FontInfoMap getInfoMap(const QString &id);
    /// every font's value of one information field; a negative codeLang takes the record of any language
    QList<FontDBResult> getInfo(const QList<FontItem *> &fonts, InfoItem info, int codeLang = 0);

public Q_SLOTS:

    bool Remove(const QString &id);

public:
    FontItem *Font(const QString &id, bool noTemporary = false);
    /**
     * Records a font item that was built outside (a remote font, known from a
     * directory's index rather than from its file). The item is owned by the
     * database from then on; on failure it is deleted and nullptr returned.
     */
    FontItem *AddFont(FontItem *fitem);
    /// whether the font is in the database (without creating it, as Font() would)
    bool Knows(const QString &id);
    /// after tags were given to many fonts with addTag(): the tag list is rebuilt once
    void announceTagsChanged();
    QList<FontItem *> AllFonts();
    QStringList AllFontNames();
    QList<FontItem *> FamilySet(const QString &family);
    // 		FontItem* FirstFont();
    // 		FontItem* NextFont();
    QList<FontItem *> Fonts(const QVariant &pattern, FMFontDb::Field field);
    // codeLang is the language id of the name records, any of them by default
    QList<FontItem *> Fonts(const QVariant &pattern, FMFontDb::InfoItem info, int codeLang = -1);
    int FontCount();
    // Sorted and without duplicates
    QStringList getTags();
    void addTagToDB(const QString &t);
    void removeTagFromDB(const QString &t);
    void editTag(const QString &tOld, const QString &tNew);
    void addTag(const QString &id, const QString &t);
    void addTag(const QStringList &idlist, const QString &t);
    void removeTag(const QString &id, const QString &t);
    void setTags(const QString &id, const QStringList &tl);
    bool insertTemporaryFont(const QString &path);
    void clearFilteredFonts();
    void filterAllFonts();
    QList<FontItem *> getFilteredFonts(bool familyOnly = false);
    void setFilterdFonts(const QList<FontItem *> &flist);
    [[nodiscard]] int countFilteredFonts() const;
    void insertFilteredFont(FontItem *item);
    void removeFilteredFont(FontItem *item);
    bool isFiltered(FontItem *item) const;

private:
    // ensure tables are created
    void initFMDb();

    // whereString carries one '?' per entry of values
    QList<FontItem *> Fonts(const QString &whereString, const QVariantList &values, Table table);
    bool execBound(QSqlQuery &query, const QString &statement, const QVariantList &values = QVariantList());
    // An empty id means that several fonts are concerned
    void invalidateTags(const QString &id = QString());

    QStringList priorList;
    QMap<Field, QString> fieldName;
    QMap<Table, QString> tableName;
    QMap<QString, int> cacheId;
    QMap<int, QString> reverseCacheId;

    int getId(const QString &fontid);
    int internalCounter = 0;

    // We maintain a map of instanciated fonts
    QMap<int, FontItem *> fontMap;
    QMap<int, FontItem *>::iterator fontCursor;
    // 		void clearFontMap();
    QMap<QString, FontItem *> temporaryFont;

    // Not sure it's that good idea but still, go to put current filtered fonts here
    QList<FontItem *> currentFonts;
    QList<FontItem *> currentFamiliesCache;

    QList<QSqlError> transactionError;
    QMap<QString, QMap<Field, QVariant>> rValueCache;
    QStringList tagsCache;
    bool tagsCacheValid;

    int transactionDeep;

Q_SIGNALS:
    void tagsChanged();
};

#endif
