#pragma once

#include "FontData.h"

#include <string>

namespace SebText
{
    struct PrintableCharacter
    {
        int GlyphIndex;
        float letterAdvance;
        float wordAdvance;
        float lineAdvance;
        float offsetX;
        float offsetY;

        inline PrintableCharacter(int glyphIndex, float letterAdvance, float wordAdvance, float lineAdvance, float offsetX, float offsetY)
        {
            GlyphIndex = glyphIndex;
            this->letterAdvance = letterAdvance;
            this->wordAdvance = wordAdvance;
            this->lineAdvance = lineAdvance;
            this->offsetX = offsetX;
            this->offsetY = offsetY;
        }

        inline float GetAdvanceX(float fontSize, float letterSpacing, float wordSpacing) const
        {
            return (letterAdvance * letterSpacing + wordAdvance * wordSpacing + offsetX) * fontSize;
        }

        inline float GetAdvanceY(float fontSize, float lineSpacing) const
        {
            return (-lineAdvance * lineSpacing + offsetY) * fontSize;
        }
    };

    struct TextData
    {
        std::vector<const GlyphData*> UniquePrintableCharacters;
        std::vector<PrintableCharacter> PrintableCharacters;

        TextData(const std::string& text, const FontData& fontData);
    };
}