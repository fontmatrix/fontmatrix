/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXTPROGRESSION_H
#define TEXTPROGRESSION_H

// #include "ui_textprogression.h"

class TextProgression
{
public:
    enum Progression {
        INLINE_LTR,
        INLINE_RTL,
        INLINE_TTB,
        INLINE_BTT,
        BLOCK_TTB,
        BLOCK_LTR,
        BLOCK_RTL,
        UNDEFINED
    };

    TextProgression();

    Progression inBlock();
    Progression inLine();

    static TextProgression *getInstance()
    {
        return instance;
    }

private:
    static TextProgression *instance;
    //	signals:
    //		void stateChanged();
    //	private slots:
    //		void forwardStateChanged();
};

#endif
