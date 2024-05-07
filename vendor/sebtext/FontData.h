#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace SebText
{
    struct Point
    {
        int X;
        int Y;
        bool OnCurve;

        Point() { X = 0; Y = 0; }

        Point(int x, int y)
        {
            X = x;
            Y = y;
        }

        Point(int x, int y, bool onCurve)
        {
            X = x;
            Y = y;
            OnCurve = onCurve;
        }
    };

    struct GlyphData
    {
        unsigned int UnicodeValue;
        char Ascii;
        unsigned int GlyphIndex;
        std::vector<Point> Points;
        std::vector<int> ContourEndIndices;
        int AdvanceWidth;
        int LeftSideBearing;

        int MinX;
        int MaxX;
        int MinY;
        int MaxY;

        int Width() const { return MaxX - MinX; };
        int Height() const { return MaxY - MinY; };
    };

    struct FontData
    {
        std::vector<unsigned char> TtfBytes;
        std::vector<GlyphData> Glyphs;
        GlyphData* MissingGlyph;
        int UnitsPerEm;

        std::unordered_map<unsigned int, unsigned int> glyphLookup;

        FontData(const std::string& fontFilePath);
        bool TryGetGlyph(unsigned int unicode, const GlyphData*& character) const;
    };
}