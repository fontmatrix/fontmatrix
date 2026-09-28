/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// try to make it the more compact & simple as possible

#include "fmkernfeat.h"
#include "fontmatrix_debug.h"

#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include <QDebug>
#include <QStringList>

FMKernFeature::FMKernFeature(FT_Face face)
    : p_face(face)
{
    FT_ULong length = 0;
    if (!FT_Load_Sfnt_Table(face, TTAG_GPOS, 0, nullptr, &length)) {
        if (length > 32) {
            GPOSTableRaw.resize(length);
            FT_Load_Sfnt_Table(face, TTAG_GPOS, 0, (FT_Byte *)GPOSTableRaw.data(), &length);

            makeCoverage();
        } else
            GPOSTableRaw.clear();
    }
}

FMKernFeature::~FMKernFeature() = default;

void FMKernFeature::makeCoverage()
{
    if (GPOSTableRaw.isEmpty())
        return;

    bool out(true);
    quint16 FeatureList_Offset = toUint16(6);
    quint16 LookupList_Offset = toUint16(8);

    // Find the offsets of the kern feature tables
    quint16 FeatureCount = toUint16(FeatureList_Offset);
    ;
    QList<quint16> FeatureKern_Offset;
    for (quint16 FeatureRecord(0); FeatureRecord < FeatureCount; ++FeatureRecord) {
        int rawIdx(FeatureList_Offset + 2 + (6 * FeatureRecord));
        quint32 tag(FT_MAKE_TAG(GPOSTableRaw.at(rawIdx), GPOSTableRaw.at(rawIdx + 1), GPOSTableRaw.at(rawIdx + 2), GPOSTableRaw.at(rawIdx + 3)));
        if (tag == TTAG_kern) {
            FeatureKern_Offset << (toUint16(rawIdx + 4) + FeatureList_Offset);
            if (out) {
                qCDebug(FONTMATRIX_LOG) << "KERN" << FeatureKern_Offset;
            }
        }
    }

    // Extract indices of lookups for feture kern
    QList<quint16> LookupListIndex;
    for (const auto &kern : std::as_const(FeatureKern_Offset)) {
        quint16 LookupCount(toUint16(kern + 2));
        if (out) {
            qCDebug(FONTMATRIX_LOG) << "\tParams" << toUint16(kern);
            qCDebug(FONTMATRIX_LOG) << "\tLookupCount" << LookupCount;
        }
        for (int llio(0); llio < LookupCount; ++llio) {
            quint16 Idx(toUint16(kern + 4 + (llio * 2)));
            if (!LookupListIndex.contains(Idx)) {
                LookupListIndex << Idx;
            }
        }
        if (out)
            qCDebug(FONTMATRIX_LOG) << "\tLookupIndex" << LookupListIndex;
    }

    // Extract offsets of lookup tables for feature kern
    for (int i(0); i < LookupListIndex.size(); ++i) {
        int rawIdx(LookupList_Offset + 2 + (LookupListIndex[i] * 2));
        quint16 Lookup(toUint16(rawIdx) + LookupList_Offset);
        quint16 SubTableCount(toUint16(Lookup + 4));
        for (int stIdx(0); stIdx < SubTableCount; ++stIdx) {
            quint16 SubTable(toUint16(Lookup + 6 + (2 * stIdx)) + Lookup);

            quint16 PosFormat(toUint16(SubTable));
            quint16 Coverage_Offset(toUint16(SubTable + 2) + SubTable);
            quint16 CoverageFormat(toUint16(Coverage_Offset));

            if (out) {
                qCDebug(FONTMATRIX_LOG) << "\t\tPosFormat" << PosFormat;
                qCDebug(FONTMATRIX_LOG) << "\t\tCoverageFormat" << CoverageFormat;
            }
            if (1 == CoverageFormat) // glyph indices based
            {
                quint16 GlyphCount(toUint16(Coverage_Offset + 2));
                quint16 GlyphID(Coverage_Offset + 4);
                if (out)
                    qCDebug(FONTMATRIX_LOG) << "\t\t\tGlyphCount" << GlyphCount;
                for (unsigned int gl(0); gl < GlyphCount; ++gl) {
                    coverages[SubTable] << toUint16(GlyphID + (gl * 2));
                }
            } else if (2 == CoverageFormat) // Coverage Format2 => ranges based
            {
                quint16 RangeCount(toUint16(Coverage_Offset + 2));
                if (out)
                    qCDebug(FONTMATRIX_LOG) << "\t\t\tRangeCount" << RangeCount;
                for (int r(0); r < RangeCount; ++r) {
                    quint16 rBase(Coverage_Offset + 4 + (r * 6));
                    quint16 Start(toUint16(rBase));
                    quint16 End(toUint16(rBase + 2));
                    quint16 StartCoverageIndex(toUint16(rBase + 4));
                    if (out)
                        qCDebug(FONTMATRIX_LOG) << "\t\t\t\tRange" << Start << End << StartCoverageIndex;
                    for (unsigned int gl(Start); gl <= End; ++gl)
                        coverages[SubTable] << gl;
                }
            } else
                qCDebug(FONTMATRIX_LOG) << "Unknow Coverage Format:" << CoverageFormat;

            if (out)
                qCDebug(FONTMATRIX_LOG) << "\t\t**Built a coverage array of length:" << coverages[SubTable].size();
            makePairs(SubTable);
        }
    }
}

void FMKernFeature::makePairs(quint16 subtableOffset)
{
    /*
    Lookup Type 2:
    Pair Adjustment Positioning Subtable
    */

    quint16 PosFormat(toUint16(subtableOffset));

    if (PosFormat == 1) {
        quint16 ValueFormat1(toUint16(subtableOffset + 4));
        quint16 ValueFormat2(toUint16(subtableOffset + 6));
        quint16 PairSetCount(toUint16(subtableOffset + 8));
        if (ValueFormat1 && ValueFormat2) {
            for (int psIdx(0); psIdx < PairSetCount; ++psIdx) {
                unsigned int FirstGlyph(coverages[subtableOffset][psIdx]);
                quint16 PairSetOffset(toUint16(subtableOffset + 10 + (2 * psIdx)) + subtableOffset);
                quint16 PairValueCount(toUint16(PairSetOffset));
                quint16 PairValueRecord(PairSetOffset + 2);
                for (int pvIdx(0); pvIdx < PairValueCount; ++pvIdx) {
                    quint16 recordBase(PairValueRecord + ((2 + 2 + 2) * pvIdx));
                    quint16 SecondGlyph(toUint16(recordBase));
                    qint16 Value1(toInt16(recordBase + 2));
                    pairs[FirstGlyph][SecondGlyph] = double(Value1);
                }
            }
        } else if (ValueFormat1 && (!ValueFormat2)) {
            for (int psIdx(0); psIdx < PairSetCount; ++psIdx) {
                unsigned int FirstGlyph(coverages[subtableOffset][psIdx]);
                quint16 PairSetOffset(toUint16(subtableOffset + 10 + (2 * psIdx)) + subtableOffset);
                quint16 PairValueCount(toUint16(PairSetOffset));
                quint16 PairValueRecord(PairSetOffset + 2);
                for (int pvIdx(0); pvIdx < PairValueCount; ++pvIdx) {
                    quint16 recordBase(PairValueRecord + ((2 + 2) * pvIdx));
                    quint16 SecondGlyph(toUint16(recordBase));
                    qint16 Value1(toInt16(recordBase + 2));
                    pairs[FirstGlyph][SecondGlyph] = double(Value1);
                }
            }
        } else {
            qCDebug(FONTMATRIX_LOG) << "ValueFormat1 is null or both ValueFormat1 and ValueFormat2 are null";
        }

    } else if (PosFormat == 2) {
        quint16 ValueFormat1(toUint16(subtableOffset + 4));
        quint16 ValueFormat2(toUint16(subtableOffset + 6));
        quint16 ClassDef1(toUint16(subtableOffset + 8) + subtableOffset);
        quint16 ClassDef2(toUint16(subtableOffset + 10) + subtableOffset);
        quint16 Class1Count(toUint16(subtableOffset + 12));
        quint16 Class2Count(toUint16(subtableOffset + 14));
        quint16 Class1Record(subtableOffset + 16);

        // first extract classses
        ClassDefTable Class1Data(getClass(ClassDef1, subtableOffset));
        ClassDefTable Class2Data(getClass(ClassDef2, subtableOffset));

        if (ValueFormat1 && ValueFormat2) {
            for (quint16 C1(0); C1 < Class1Count; ++C1) {
                QList<quint16> Class1(Class1Data[C1]);
                quint16 Class2Record(Class1Record + (C1 * (2 * 2 * Class2Count)));
                for (quint16 C2(0); C2 < Class2Count; ++C2) {
                    qint16 Value1(toInt16(Class2Record + (C2 * (2 * 2))));
                    QList<quint16> Class2(Class2Data[C2]);
                    // keep it barbarian :D
                    for (const auto &FirstGlyph : Class1) {
                        for (const auto &SecondGlyph : Class2) {
                            if (Value1)
                                pairs[FirstGlyph][SecondGlyph] = double(Value1);
                        }
                    }
                }
            }
        } else if (ValueFormat1 && (!ValueFormat2)) {
            for (quint16 C1(0); C1 < Class1Count; ++C1) {
                QList<quint16> Class1(Class1Data[C1]);
                quint16 Class2Record(Class1Record + (C1 * (2 * Class2Count)));
                for (quint16 C2(0); C2 < Class2Count; ++C2) {
                    qint16 Value1(toInt16(Class2Record + (C2 * 2)));
                    QList<quint16> Class2(Class2Data[C2]);

                    for (const auto &FirstGlyph : Class1) {
                        for (const auto &SecondGlyph : Class2) {
                            if (Value1)
                                pairs[FirstGlyph][SecondGlyph] = double(Value1);
                        }
                    }
                }
            }
        } else {
            qCDebug(FONTMATRIX_LOG) << "ValueFormat1 is null or both ValueFormat1 and ValueFormat2 are null";
        }

    } else
        qCDebug(FONTMATRIX_LOG) << "unknown PosFormat" << PosFormat;
}

quint16 FMKernFeature::toUint16(quint16 index)
{
    // two bytes from index on: the last pair of the table is a valid read
    if ((index + 2) > GPOSTableRaw.size()) {
        return 0;
    }
    quint16 c1(static_cast<unsigned char>(GPOSTableRaw.at(index)));
    quint16 c2(static_cast<unsigned char>(GPOSTableRaw.at(index + 1)));
    quint16 ret((c1 << 8) | c2);
    // 	qDebug()<<"**"<<index<<"("<<c1 << c2 <<")"<<ret;
    return ret;
}

qint16 FMKernFeature::toInt16(quint16 index)
{
    if ((index + 2) > GPOSTableRaw.size()) {
        return 0;
    }
    // FIXME I just do not know how it has to be done *properly*
    quint16 c1(static_cast<unsigned char>(GPOSTableRaw.at(index)));
    quint16 c2(static_cast<unsigned char>(GPOSTableRaw.at(index + 1)));
    qint16 ret((c1 << 8) | c2);
    return ret;
}

QString FMKernFeature::glyphname(int index)
{
    QByteArray key(1001, 0);
    if (FT_HAS_GLYPH_NAMES(p_face)) {
        FT_Get_Glyph_Name(p_face, index, key.data(), 1000);
        if (key[0] == char(0)) {
            key = "noname";
        }
    } else {
        key = "noname";
    }
    return QString::fromLatin1(key);
}

FMKernFeature::ClassDefTable FMKernFeature::getClass(quint16 classDefOffset, quint16 coverageId)
{
    ClassDefTable ret;
    ret[0] = coverages[coverageId];
    // 	ret[0] = QList<quint16>();
    quint16 ClassFormat(toUint16(classDefOffset));
    if (ClassFormat == 1) {
        // 		qDebug()<<"ClassFormat1";
        quint16 StartGlyph(toUint16(classDefOffset + 2));
        quint16 GlyphCount(toUint16(classDefOffset + 4));
        quint16 ClassValueArray(classDefOffset + 6);
        for (quint16 CV(0); CV < GlyphCount; ++CV) {
            ret[0].removeAll(StartGlyph + CV);
            ret[toUint16(ClassValueArray + (CV * 2))] << StartGlyph + CV;
        }
    } else if (ClassFormat == 2) {
        // 		qDebug()<<"ClassFormat2";
        quint16 ClassRangeCount(toUint16(classDefOffset + 2));
        quint16 ClassRangeRecord(classDefOffset + 4);
        for (int CRR(0); CRR < ClassRangeCount; ++CRR) {
            quint16 Start(toUint16(ClassRangeRecord + (CRR * 6)));
            quint16 End(toUint16(ClassRangeRecord + (CRR * 6) + 2));
            quint16 Class(toUint16(ClassRangeRecord + (CRR * 6) + 4));
            // 			qDebug()<<"CRC"<<Start<<End<<Class;
            for (quint16 gl(Start); gl <= End; ++gl) {
                ret[0].removeAll(gl);
                ret[Class] << gl;
            }
        }
    } else
        qCDebug(FONTMATRIX_LOG) << "Unknown Class Table type";

    // 	for (const auto& c : ret.keys())
    // 	{
    // 		if(c>0)
    // 		{
    // 			QStringList dl;
    // 			for (const auto& lg : ret[c])
    // 			{
    // 				dl << glyphname(lg);
    // 			}
    // 			if(!dl.contains("noname"))
    // 				qDebug()<<"\t"<< c <<dl.join(";");
    // 			else
    // 				qDebug()<<"ERROR nonames"<<dl.size();
    // 		}
    // 		else
    // 			qDebug()<<"\t"<< c <<ret[c].size();
    // 	}

    return ret;
}
