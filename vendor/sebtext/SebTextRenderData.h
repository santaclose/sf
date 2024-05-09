#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <SebTextTextData.h>

#define ALIGNMENT_LEFT 0
#define ALIGNMENT_TOP 0
#define ALIGNMENT_CENTER 1
#define ALIGNMENT_RIGHT 2
#define ALIGNMENT_BOTTOM 2

namespace SebText
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 uvs;
    };
    extern Vertex MeshVertices[4];
    extern unsigned int MeshIndices[6];

    struct GlyphRenderData
    {
        int NumContours;
        int ContourDataOffset;
        int PointDataOffset;
        glm::vec2 Size;
    };

    struct TextRenderData
    {
        std::vector<glm::vec2> BezierPoints;
        std::vector<GlyphRenderData> AllGlyphData;
        // Metadata for each glyph: bezier data offset, num contours, contour length/s
        std::vector<int> GlyphMetaData;
    };

    struct LayoutSettings
    {
        float FontSize = 1.0f;
        float LineSpacing = 1.0f;
        float LetterSpacing = 1.0f;
        float WordSpacing = 1.0f;
        int AlignmentH = ALIGNMENT_LEFT;
        int AlignmentV = ALIGNMENT_TOP;

        LayoutSettings() = default;
        inline LayoutSettings(float fontSize, float lineSpacing, float letterSpacing, float wordSpacing)
        {
            FontSize = fontSize;
            LineSpacing = lineSpacing;
            LetterSpacing = letterSpacing;
            WordSpacing = wordSpacing;
        }

        inline bool operator==(const LayoutSettings& rhs) const
        {
            return FontSize == rhs.FontSize && LineSpacing == rhs.LineSpacing && LetterSpacing == rhs.LetterSpacing && WordSpacing == rhs.WordSpacing;
        }
        inline bool operator!=(const LayoutSettings& rhs) const
        {
            return !operator==(rhs);
        }
    };

    struct InstanceData
    {
        glm::vec2 boundsSize;
        glm::vec2 offset;
        float letterAdvance;
        float wordAdvance;
        int contourDataOffset;
        int line;
    };

    TextRenderData CreateRenderData(const std::vector<const GlyphData*>& uniqueCharacters, const FontData& fontData);
    void CreateInstanceData(std::vector<InstanceData>& instanceData, const TextData& textData, const std::vector<GlyphRenderData>& prevGlyphRenderData, const LayoutSettings& layoutSettings);
}