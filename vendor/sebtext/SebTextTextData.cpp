#include "SebTextTextData.h"

#include <unordered_map>

// SPACE_SIZE_EM should be consistent with LINE_HEIGHT_EM in shader.vert.glsl
#define SPACE_SIZE_EM 0.333f

SebText::TextData::TextData(const std::string& text, const FontData& fontData)
{
    std::unordered_map<const GlyphData*, int> charToIndexTable;

    LineCount = 1;
    LastCharacterPerLine.clear();
    float scale = 1.0f / fontData.UnitsPerEm;
    float letterAdvance = 0;
    float wordAdvance = 0;

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
            LastCharacterPerLine.push_back(PrintableCharacters.size() - 1);
            LineCount++;
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

            PrintableCharacters.emplace_back(uniqueIndex, letterAdvance, wordAdvance, offsetX, offsetY, LineCount - 1);
            letterAdvance += character->AdvanceWidth * scale;
        }
    }
    LastCharacterPerLine.push_back(PrintableCharacters.size() - 1);
}