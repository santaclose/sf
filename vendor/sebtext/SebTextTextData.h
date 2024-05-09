#pragma once

#include "SebTextFontData.h"

#include <string>

namespace SebText
{
    struct PrintableCharacter
    {
        int GlyphIndex;
        float letterAdvance;
        float wordAdvance;
        float offsetX;
        float offsetY;
        int line;

        inline PrintableCharacter(int glyphIndex, float letterAdvance, float wordAdvance, float offsetX, float offsetY, int line)
        {
            GlyphIndex = glyphIndex;
            this->letterAdvance = letterAdvance;
            this->wordAdvance = wordAdvance;
            this->offsetX = offsetX;
            this->offsetY = offsetY;
            this->line = line;
        }
    };

    struct TextData
    {
        std::vector<const GlyphData*> UniquePrintableCharacters;
        std::vector<PrintableCharacter> PrintableCharacters;
        std::vector<int> LastCharacterPerLine;
        int LineCount;

        TextData(const std::string& text, const FontData& fontData);
    };
}