#include "SebTextRenderData.h"

#include "SebTextFontData.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

SebText::Vertex SebText::MeshVertices[4] = {
    { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f} }
};

unsigned int SebText::MeshIndices[6] = { 0, 1, 2, 2, 3, 0 };

void SplitAtTurningPointY(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, glm::vec2& a1, glm::vec2& a2, glm::vec2& b1, glm::vec2& b2)
{
    glm::vec2 a = p0 - 2.0f * p1 + p2;
    glm::vec2 b = 2.0f * (p1 - p0);
    glm::vec2 c = p0;

    // Calculate turning point by setting gradient.y to 0: 2at + b = 0; therefore t = -b / 2a
    float turningPointT = -b.y / (2 * a.y);
    glm::vec2 turningPoint = a * turningPointT * turningPointT + b * turningPointT + c;

    // Calculate the new p1 point for curveA with points: p0, p1A, turningPoint
    // This is done by saying that p0 + gradient(t=0) * ? = p1A = (p1A.x, turningPoint.y)
    // Solve for lambda using the known turningPoint.y, and then solve for p1A.x
    float lambdaA = (turningPoint.y - p0.y) / b.y;
    float p1A_x = p0.x + b.x * lambdaA;

    // Calculate the new p1 point for curveB with points: turningPoint, p1B, p2
    // This is done by saying that p2 + gradient(t=1) * ? = p1B = (p1B.x, turningPoint.y)
    // Solve for lambda using the known turningPoint.y, and then solve for p1B.x
    float lambdaB = (turningPoint.y - p2.y) / (2.0f * a.y + b.y);
    float p1B_x = p2.x + (2 * a.x + b.x) * lambdaB;

    a1 = { p1A_x, turningPoint.y };
    a2 = turningPoint;
    b1 = { p1B_x, turningPoint.y };
    b2 = p2;
}

std::vector<glm::vec2> MakeMonotonic(std::vector<glm::vec2>& original)
{
    std::vector<glm::vec2> monotonic; monotonic.reserve(original.size());
    monotonic.push_back(original[0]);

    for (int i = 0; i < original.size() - 1; i += 2)
    {
        glm::vec2 p0 = original[i];
        glm::vec2 p1 = original[i + 1];
        glm::vec2 p2 = original[i + 2];

        if ((p1.y < MIN(p0.y, p2.y) || p1.y > MAX(p0.y, p2.y)))
        {
            glm::vec2 a1, a2, b1, b2;
            SplitAtTurningPointY(p0, p1, p2, a1, a2, b1, b2);
            monotonic.push_back(a1);
            monotonic.push_back(a2);
            monotonic.push_back(b1);
            monotonic.push_back(b2);
        }
        else
        {
            monotonic.push_back(p1);
            monotonic.push_back(p2);
        }
    }
    return monotonic;
}

void GetBounds(const SebText::GlyphData& character, const SebText::FontData& fontData, glm::vec2& outCenter, glm::vec2& outSize)
{
    const float antiAliasPadding = 0.005f;
    float scale = 1.0f / fontData.UnitsPerEm;

    float left = character.MinX * scale;
    float right = character.MaxX * scale;
    float top = character.MaxY * scale;
    float bottom = character.MinY * scale;

    outCenter = glm::vec2(left + right, top + bottom) / 2.0f;
    outSize = glm::vec2(right - left, top - bottom) + glm::vec2(1.0f, 1.0f) * antiAliasPadding;
}

void CreateContoursWithImpliedPoints(const SebText::GlyphData& character, float scale, std::vector<std::vector<glm::vec2>>& contours)
{
    const bool convertStraightLinesToBezier = true;

    int debug = 0;
    int startPointIndex = 0;
    int contourCount = character.ContourEndIndices.size();

    for (int contourIndex = 0; contourIndex < contourCount; contourIndex++)
    {
        int contourEndIndex = character.ContourEndIndices[contourIndex];
        int numPointsInContour = contourEndIndex - startPointIndex + 1;
        const SebText::Point* contourPoints = &(character.Points[startPointIndex]);

        contours.emplace_back();

        // Get index of first on-curve point (seems to not always be first point for whatever reason)
        int firstOnCurvePointIndex = 0;
        for (int i = 0; i < numPointsInContour; i++)
        {
            if (contourPoints[i].OnCurve)
            {
                firstOnCurvePointIndex = i;
                break;
            }
        }

        for (int i = 0; i < numPointsInContour; i++)
        {
            SebText::Point curr = contourPoints[(i + firstOnCurvePointIndex + 0) % numPointsInContour];
            SebText::Point next = contourPoints[(i + firstOnCurvePointIndex + 1) % numPointsInContour];

            contours.back().push_back({ curr.X * scale, curr.Y * scale });
            bool isConsecutiveOffCurvePoints = !curr.OnCurve && !next.OnCurve;
            bool isStraightLine = curr.OnCurve && next.OnCurve;

            if (isConsecutiveOffCurvePoints || (isStraightLine && convertStraightLinesToBezier))
            {
                float newX = (curr.X + next.X) / 2.0f * scale;
                float newY = (curr.Y + next.Y) / 2.0f * scale;
                contours.back().push_back({ newX, newY });
            }
        }
        contours.back().push_back(contours.back()[0]);
        contours.back() = MakeMonotonic(contours.back());

        startPointIndex = contourEndIndex + 1;
    }
}

SebText::TextRenderData SebText::CreateRenderData(const std::vector<const GlyphData*>& uniqueCharacters, const FontData& fontData)
{
    TextRenderData renderData;

    float scale = 1.0f / fontData.UnitsPerEm;
    for (int charIndex = 0; charIndex < uniqueCharacters.size(); charIndex++)
    {
        std::vector<std::vector<glm::vec2>> contours;
        CreateContoursWithImpliedPoints(*uniqueCharacters[charIndex], scale, contours);

        glm::vec2 glyphBoundsCenter, glyphBoundsSize;
        GetBounds(*uniqueCharacters[charIndex], fontData, glyphBoundsCenter, glyphBoundsSize);
        GlyphRenderData glyphData =
        {
            (int) contours.size(),
            (int) renderData.GlyphMetaData.size(),
            (int) renderData.BezierPoints.size(),
            glyphBoundsSize
        };

        renderData.AllGlyphData.push_back(glyphData);
        renderData.GlyphMetaData.push_back(renderData.BezierPoints.size());
        renderData.GlyphMetaData.push_back(contours.size());

        for (const auto& contour : contours)
        {
            renderData.GlyphMetaData.push_back(contour.size() - 1);
            for (int i = 0; i < contour.size(); i++)
                renderData.BezierPoints.push_back(contour[i] - glyphBoundsCenter);
        }
    }

    return renderData;
}

void SebText::CreateInstanceData(std::vector<InstanceData>& instanceData, const TextData& textData, const std::vector<GlyphRenderData>& prevGlyphRenderData, const LayoutSettings& layoutSettings)
{
    instanceData.resize(textData.PrintableCharacters.size());

    for (int i = 0; i < textData.PrintableCharacters.size(); i++)
    {
        const PrintableCharacter& layout = textData.PrintableCharacters[i];
        const GlyphRenderData& info = prevGlyphRenderData[layout.GlyphIndex];
        instanceData[i] = { info.Size, {layout.offsetX, layout.offsetY }, layout.letterAdvance, layout.wordAdvance, info.ContourDataOffset, layout.line };
    }
}