#include "TextData.h"

#include <unordered_map>

#define SPACE_SIZE_EM 0.333f
#define LINE_HEIGHT_EM 1.3f

SebText::TextData::TextData(const std::string& text, const FontData& fontData)
{
    std::unordered_map<const GlyphData*, int> charToIndexTable;

    float scale = 1.0f / fontData.UnitsPerEm;
    float letterAdvance = 0;
    float wordAdvance = 0;
    float lineAdvance = 0;

    for (int i = 0; i < text.size(); i++)
    {
        if (text[i] == ' ')
        {
            wordAdvance += SPACE_SIZE_EM;
        }
        else if (text[i] == '\t')
        {
            wordAdvance += SPACE_SIZE_EM * 4; // TODO: proper tab implementation
        }
        else if (text[i] == '\n')
        {
            lineAdvance += LINE_HEIGHT_EM;
            wordAdvance = 0;
            letterAdvance = 0;
        }
        else //if (!char.IsControl(text[i])) assume it isn't a control character
        {
            const GlyphData* character;
            fontData.TryGetGlyph(text[i], character);

            int uniqueIndex;
            if (charToIndexTable.find(character) == charToIndexTable.end())
            {
                uniqueIndex = UniquePrintableCharacters.size();
                charToIndexTable.insert({ character, uniqueIndex });
                UniquePrintableCharacters.push_back(character);
            }
            else
                uniqueIndex = charToIndexTable[character];

            float offsetX = (character->MinX + character->Width() / 2) * scale;
            float offsetY = (character->MinY + character->Height() / 2) * scale;

            PrintableCharacters.emplace_back(uniqueIndex, letterAdvance, wordAdvance, lineAdvance, offsetX, offsetY);
            letterAdvance += character->AdvanceWidth * scale;
        }
    }
}