#include "SebTextFontData.h"
#include <cassert>

static unsigned int ToLittleEndian(unsigned int bigEndianValue)
{
	unsigned int a = (bigEndianValue >> 24) & 0b11111111;
	unsigned int b = (bigEndianValue >> 16) & 0b11111111;
	unsigned int c = (bigEndianValue >> 8) & 0b11111111;
	unsigned int d = (bigEndianValue >> 0) & 0b11111111;
	return a | b << 8 | c << 16 | d << 24;
}

static unsigned short ToLittleEndian(unsigned short bigEndianValue)
{
	return (unsigned short)(bigEndianValue >> 8 | bigEndianValue << 8);
}

#define READ_U8 (*((unsigned char*)(cursor)))
#define READ_U16 ToLittleEndian(*((unsigned short*)(cursor)))
#define READ_U32 ToLittleEndian(*((unsigned int*)(cursor)))
#define READ_I8 (*((char*)(cursor)))
#define READ_I16 ((short)ToLittleEndian(*((unsigned short*)(cursor))))
#define READ_I32 ((int)ToLittleEndian(*((unsigned int*)(cursor))))
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define FLAG_BIT_IS_SET(flag, bitIndex) (((flag >> bitIndex) & 1) == 1)
#define READ_FIXED_POINT_2_DOT_14 ((short)(READ_U16) / (double)(1 << 14))
#define MAX_INT 2147483647
#define MIN_INT (-2147483647 - 1)

static void ReadTableLocations(const std::vector<unsigned char>& ttfBytes, std::unordered_map<std::string, unsigned int>& out)
{
	// -- offset subtable --
	const unsigned char* cursor = ttfBytes.data();
	cursor += 4; // unused: scalerType
	int numTables = READ_U16; cursor += 2;
	cursor += 6; // unused: searchRange, entrySelector, rangeShift

	// -- table directory --
	for (int i = 0; i < numTables; i++)
	{
		std::string tag; tag.resize(4);
		memcpy(tag.data(), cursor, 4); cursor += 4;
		unsigned int checksum = READ_U32; cursor += 4;
		unsigned int offset = READ_U32; cursor += 4;
		unsigned int length = READ_U32; cursor += 4;

		out.insert({ tag, offset });
	}
}

static void GetAllGlyphLocations(const std::vector<unsigned char>& ttfBytes, int numGlyphs, int bytesPerEntry, unsigned int locaTableLocation, unsigned int glyfTableLocation, std::vector<unsigned int>& allGlyphLocations)
{
	allGlyphLocations.resize(numGlyphs);
	bool isTwoByteEntry = bytesPerEntry == 2;

	for (int glyphIndex = 0; glyphIndex < numGlyphs; glyphIndex++)
	{
		const unsigned char* cursor = ttfBytes.data() + (locaTableLocation + glyphIndex * bytesPerEntry);
		// If 2-byte format is used, the stored location is half of actual location (so multiply by 2)
		unsigned int glyphDataOffset;
		if (isTwoByteEntry) { glyphDataOffset = READ_U16 * 2u; cursor += 2; }
		else { glyphDataOffset = READ_U32; cursor += 4; }
		allGlyphLocations[glyphIndex] = glyfTableLocation + glyphDataOffset;
	}
}

// Create a lookup from unicode to font's internal glyph index
static void GetUnicodeToGlyphIndexMappings(const std::vector<unsigned char>& ttfBytes, unsigned int cmapOffset, std::vector<std::pair<unsigned int, unsigned int>>& out)
{
    const unsigned char* cursor = ttfBytes.data() + cmapOffset;

    unsigned int version = READ_U16; cursor += 2;
    unsigned int numSubtables = READ_U16; cursor += 2; // font can contain multiple character maps for different platforms

    // --- Read through metadata for each character map to find the one we want to use ---
    unsigned int cmapSubtableOffset = 0;
    int selectedUnicodeVersionID = -1;

    for (int i = 0; i < numSubtables; i++)
    {
        int platformID = READ_U16; cursor += 2;
        int platformSpecificID = READ_U16; cursor += 2;
        unsigned int offset = READ_U32; cursor += 4;

        // Unicode encoding
        if (platformID == 0)
        {
            // Use highest supported unicode version
            if (platformSpecificID > -1 && platformSpecificID < 5 && platformSpecificID > selectedUnicodeVersionID)
            {
                cmapSubtableOffset = offset;
                selectedUnicodeVersionID = platformSpecificID;
            }
        }
        // Microsoft Encoding
        else if (platformID == 3 && selectedUnicodeVersionID == -1)
        {
            if (platformSpecificID == 1 || platformSpecificID == 10)
            {
                cmapSubtableOffset = offset;
            }
        }
    }

    if (cmapSubtableOffset == 0)
    {
        assert(false);
        //throw new Exception("Font does not contain supported character map type (TODO)");
    }

    // Go to the character map
    cursor = ttfBytes.data() + cmapOffset + cmapSubtableOffset;
    int format = READ_U16; cursor += 2;
    bool hasReadMissingCharGlyph = false;

    if (format != 12 && format != 4)
    {
        assert(false);
        //throw new Exception("Font cmap format not supported (TODO): " + format);
    }

    // ---- Parse Format 4 ----
    if (format == 4)
    {
        int length = READ_U16; cursor += 2;
        int languageCode = READ_U16; cursor += 2;
        // Number of contiguous segments of character codes
        int segCount2X = READ_U16; cursor += 2;
        int segCount = segCount2X / 2;
        cursor += 6; // Skip: searchRange, entrySelector, rangeShift

        // Ending character code for each segment (last = 2^16 - 1)
        std::vector<int> endCodes; endCodes.reserve(segCount);
        for (int i = 0; i < segCount; i++)
        {
            endCodes.push_back(READ_U16); cursor += 2;
        }

        cursor += 2; // Reserved pad

        std::vector<int> startCodes; startCodes.reserve(segCount);
        for (int i = 0; i < segCount; i++)
        {
            startCodes.push_back(READ_U16); cursor += 2;
        }

        std::vector<int> idDeltas; idDeltas.reserve(segCount);
        for (int i = 0; i < segCount; i++)
        {
            idDeltas.push_back(READ_U16); cursor += 2;
        }

        std::vector<std::pair<int, int>> idRangeOffsets; idRangeOffsets.resize(segCount);
        for (int i = 0; i < segCount; i++)
        {
            int readLoc = (int)(cursor - ttfBytes.data());
            int offset = READ_U16; cursor += 2;
            idRangeOffsets[i] = { offset, readLoc };
        }

        for (int i = 0; i < startCodes.size(); i++)
        {
            int endCode = endCodes[i];
            int currCode = startCodes[i];

            if (currCode == 65535) break; // not sure about this (hack to avoid out of bounds on a specific font)

            while (currCode <= endCode)
            {
                int glyphIndex;
                // If idRangeOffset is 0, the glyph index can be calculated directly
                if (idRangeOffsets[i].first == 0)
                {
                    glyphIndex = (currCode + idDeltas[i]) % 65536;
                }
                // Otherwise, glyph index needs to be looked up from an array
                else
                {
                    unsigned int readerLocationOld = (cursor - ttfBytes.data());
                    int rangeOffsetLocation = idRangeOffsets[i].second + idRangeOffsets[i].first;
                    int glyphIndexArrayLocation = 2 * (currCode - startCodes[i]) + rangeOffsetLocation;

                    cursor = ttfBytes.data() + glyphIndexArrayLocation;
                    glyphIndex = READ_U16; cursor += 2;

                    if (glyphIndex != 0)
                    {
                        glyphIndex = (glyphIndex + idDeltas[i]) % 65536;
                    }

                    cursor = ttfBytes.data() + readerLocationOld;
                }

                out.emplace_back((unsigned int)currCode, (unsigned int)glyphIndex);
                hasReadMissingCharGlyph |= glyphIndex == 0;
                currCode++;
            }
        }
    }
    // ---- Parse Format 12 ----
    else if (format == 12)
    {
        cursor += 10; // Skip: reserved, subtableByteLengthInlcudingHeader, languageCode
        unsigned int numGroups = READ_U32; cursor += 4;

        for (int i = 0; i < numGroups; i++)
        {
            unsigned int startCharCode = READ_U32; cursor += 4;
            unsigned int endCharCode = READ_U32; cursor += 4;
            unsigned int startGlyphIndex = READ_U32; cursor += 4;

            unsigned int numChars = endCharCode - startCharCode + 1;
            for (int charCodeOffset = 0; charCodeOffset < numChars; charCodeOffset++)
            {
                unsigned int charCode = (unsigned int)(startCharCode + charCodeOffset);
                unsigned int glyphIndex = (unsigned int)(startGlyphIndex + charCodeOffset);

                out.emplace_back(charCode, glyphIndex);
                hasReadMissingCharGlyph |= glyphIndex == 0;
            }
        }
    }

    if (!hasReadMissingCharGlyph)
    {
        out.emplace_back(65535, 0);
    }
}

static void ReadGlyph(const std::vector<unsigned char>& ttfBytes, const std::vector<unsigned int>& glyphLocations, unsigned int glyphIndex, SebText::GlyphData& out);
static bool ReadNextComponentGlyph(const std::vector<unsigned char>& ttfBytes, const unsigned char*& cursor, const std::vector<unsigned int>& glyphLocations, unsigned int glyphLocation, SebText::GlyphData& out)
{
    unsigned int flag = READ_U16; cursor += 2;
    unsigned int glyphIndex = READ_U16; cursor += 2;

    unsigned int componentGlyphLocation = glyphLocations[glyphIndex];
    // If compound glyph refers to itself, return empty glyph to avoid infinite loop.
    // Had an issue with this on the 'carriage return' character in robotoslab.
    // There's likely a bug in my parsing somewhere, but this is my work-around for now...
    if (componentGlyphLocation == glyphLocation)
        return false;

    // Decode flags
    bool argsAre2Bytes = FLAG_BIT_IS_SET(flag, 0);
    bool argsAreXYValues = FLAG_BIT_IS_SET(flag, 1);
    bool roundXYToGrid = FLAG_BIT_IS_SET(flag, 2);
    bool isSingleScaleValue = FLAG_BIT_IS_SET(flag, 3);
    bool isMoreComponentsAfterThis = FLAG_BIT_IS_SET(flag, 5);
    bool isXAndYScale = FLAG_BIT_IS_SET(flag, 6);
    bool is2x2Matrix = FLAG_BIT_IS_SET(flag, 7);
    bool hasInstructions = FLAG_BIT_IS_SET(flag, 8);
    bool useThisComponentMetrics = FLAG_BIT_IS_SET(flag, 9);
    bool componentsOverlap = FLAG_BIT_IS_SET(flag, 10);

    // Read args (these are either x/y offsets, or point number)
    int arg1, arg2;
    if (argsAre2Bytes)
    {
        arg1 = READ_I16; cursor += 2;
        arg2 = READ_I16; cursor += 2;
    }
    else
    {
        arg1 = READ_I8; cursor += 1;
        arg2 = READ_I8; cursor += 1;
    }

    //if (!argsAreXYValues) throw new Exception("TODO: Args1&2 are point indices to be matched, rather than offsets");
    assert(argsAreXYValues);

    double offsetX = arg1;
    double offsetY = arg2;

    double iHat_x = 1;
    double iHat_y = 0;
    double jHat_x = 0;
    double jHat_y = 1;

    if (isSingleScaleValue)
    {
        iHat_x = READ_FIXED_POINT_2_DOT_14; cursor += 2;
        jHat_y = iHat_x;
    }
    else if (isXAndYScale)
    {
        iHat_x = READ_FIXED_POINT_2_DOT_14; cursor += 2;
        jHat_y = READ_FIXED_POINT_2_DOT_14; cursor += 2;
    }
    // Todo: incomplete implemntation
    else if (is2x2Matrix)
    {
        iHat_x = READ_FIXED_POINT_2_DOT_14; cursor += 2;
        iHat_y = READ_FIXED_POINT_2_DOT_14; cursor += 2;
        jHat_x = READ_FIXED_POINT_2_DOT_14; cursor += 2;
        jHat_y = READ_FIXED_POINT_2_DOT_14; cursor += 2;
    }

    unsigned int currentCompoundGlyphReadLocation = (cursor - ttfBytes.data());
    ReadGlyph(ttfBytes, glyphLocations, glyphIndex, out);
    cursor = ttfBytes.data() + currentCompoundGlyphReadLocation;

    for (int i = 0; i < out.Points.size(); i++)
    {
        double xPrime, yPrime;
        {
            xPrime = iHat_x * out.Points[i].X + jHat_x * out.Points[i].Y + offsetX;
            yPrime = iHat_y * out.Points[i].X + jHat_y * out.Points[i].Y + offsetY;
        }
        out.Points[i].X = (int)xPrime;
        out.Points[i].Y = (int)yPrime;
    }

    return isMoreComponentsAfterThis;
}

static void ReadCompoundGlyph(const std::vector<unsigned char>& ttfBytes, const std::vector<unsigned int>& glyphLocations, unsigned int glyphIndex, SebText::GlyphData& out)
{
    out.GlyphIndex = glyphIndex;

    unsigned int glyphLocation = glyphLocations[glyphIndex];
    const unsigned char* cursor = ttfBytes.data() + glyphLocation;
    cursor += 2;

    out.MinX = READ_I16; cursor += 2;
    out.MinY = READ_I16; cursor += 2;
    out.MaxX = READ_I16; cursor += 2;
    out.MaxY = READ_I16; cursor += 2;

    out.Points.clear();
    out.ContourEndIndices.clear();

    while (true)
    {
        SebText::GlyphData componentGlyph;
        bool hasMoreGlyphs = ReadNextComponentGlyph(ttfBytes, cursor, glyphLocations, glyphLocation, componentGlyph);

        // Add all contour end indices from the simple glyph component to the compound glyph's data
        // Note: indices must be offset to account for previously-added component glyphs
        for (int endIndex : componentGlyph.ContourEndIndices)
            out.ContourEndIndices.push_back(endIndex + out.Points.size());

        for (const auto& point : componentGlyph.Points)
            out.Points.push_back(point);

        if (!hasMoreGlyphs) break;
    }
}

static void ReadCoords(bool readingX, const unsigned char*& cursor, int numPoints, int OnCurve, int IsSingleByteX, int IsSingleByteY, int InstructionX, int InstructionY, std::vector<unsigned char>& allFlags, SebText::GlyphData& out)
{
    int min = MAX_INT;
    int max = MIN_INT;

    int singleByteFlagBit = readingX ? IsSingleByteX : IsSingleByteY;
    int instructionFlagMask = readingX ? InstructionX : InstructionY;

    int coordVal = 0;

    for (int i = 0; i < numPoints; i++)
    {
        unsigned char currFlag = allFlags[i];

        // Offset value is represented with 1 byte (unsigned)
        // Here the instruction flag tells us whether to add or subtract the offset
        if (FLAG_BIT_IS_SET(currFlag, singleByteFlagBit))
        {
            int coordOffset = READ_U8; cursor += 1;
            bool positiveOffset = FLAG_BIT_IS_SET(currFlag, instructionFlagMask);
            coordVal += positiveOffset ? coordOffset : -coordOffset;
        }
        // Offset value is represented with 2 bytes (signed)
        // Here the instruction flag tells us whether an offset value exists or not
        else if (!FLAG_BIT_IS_SET(currFlag, instructionFlagMask))
        {
            coordVal += READ_I16; cursor += 2;
        }

        if (readingX) out.Points[i].X = coordVal;
        else out.Points[i].Y = coordVal;
        out.Points[i].OnCurve = FLAG_BIT_IS_SET(currFlag, OnCurve);

        min = MIN(min, coordVal);
        max = MAX(max, coordVal);
    }
}
static void ReadSimpleGlyph(const std::vector<unsigned char>& ttfBytes, const std::vector<unsigned int>& glyphLocations, unsigned int glyphIndex, SebText::GlyphData& out)
{
    // Flag masks
    const int OnCurve = 0;
    const int IsSingleByteX = 1;
    const int IsSingleByteY = 2;
    const int Repeat = 3;
    const int InstructionX = 4;
    const int InstructionY = 5;


    const unsigned char* cursor = ttfBytes.data() + glyphLocations[glyphIndex];

    out.GlyphIndex = glyphIndex;

    int contourCount = READ_I16; cursor += 2;
    //if (contourCount < 0) throw new Exception("Expected simple glyph, but found compound glyph instead");
    assert(contourCount > -1);

    out.MinX = READ_I16; cursor += 2;
    out.MinY = READ_I16; cursor += 2;
    out.MaxX = READ_I16; cursor += 2;
    out.MaxY = READ_I16; cursor += 2;

    // Read contour ends
    int numPoints = 0;
    out.ContourEndIndices.resize(contourCount);

    for (int i = 0; i < contourCount; i++)
    {
        int contourEndIndex = READ_U16; cursor += 2;
        numPoints = MAX(numPoints, contourEndIndex + 1);
        out.ContourEndIndices[i] = contourEndIndex;
    }

    int instructionsLength = READ_I16; cursor += 2;
    cursor += instructionsLength; // skip instructions (hinting stuff)

    std::vector<unsigned char> allFlags; allFlags.resize(numPoints);
    out.Points.resize(numPoints);

    for (int i = 0; i < numPoints; i++)
    {
        unsigned char flag = READ_U8; cursor += 1;
        allFlags[i] = flag;

        if (FLAG_BIT_IS_SET(flag, Repeat))
        {
            int repeatCount = READ_U8; cursor += 1;

            for (int r = 0; r < repeatCount; r++)
            {
                i++;
                allFlags[i] = flag;
            }
        }
    }

    ReadCoords(true, cursor, numPoints, OnCurve, IsSingleByteX, IsSingleByteY, InstructionX, InstructionY, allFlags, out);
    ReadCoords(false, cursor, numPoints, OnCurve, IsSingleByteX, IsSingleByteY, InstructionX, InstructionY, allFlags, out);
}

static void ReadGlyph(const std::vector<unsigned char>& ttfBytes, const std::vector<unsigned int>& glyphLocations, unsigned int glyphIndex, SebText::GlyphData& out)
{
    unsigned int glyphLocation = glyphLocations[glyphIndex];

    const unsigned char* cursor = ttfBytes.data() + glyphLocation;
    int contourCount = READ_I16; cursor += 2;

    // Glyph is either simple or compound
    // * Simple: outline data is stored here directly
    // * Compound: two or more simple glyphs need to be looked up, transformed, and combined
    bool isSimpleGlyph = contourCount >= 0;

    if (isSimpleGlyph) ReadSimpleGlyph(ttfBytes, glyphLocations, glyphIndex, out);
    else ReadCompoundGlyph(ttfBytes, glyphLocations, glyphIndex, out);
}

static void ReadAllGlyphs(const std::vector<unsigned char>& ttfBytes, const std::vector<unsigned int>& glyphLocations, const std::vector<std::pair<unsigned int, unsigned int>>& mappings, std::vector<SebText::GlyphData>& out)
{
    out.resize(mappings.size());
    for (int i = 0; i < mappings.size(); i++)
    {
        const auto& mapping = mappings[i];
        SebText::GlyphData glyphData;
        ReadGlyph(ttfBytes, glyphLocations, mapping.second, glyphData);
        glyphData.UnicodeValue = mapping.first;
        out[i] = glyphData;
    }
}

// Get horizontal layout information from the "hhea" and "hmtx" tables
static void ApplyLayoutInfo(const std::vector<unsigned char>& ttfBytes, std::unordered_map<std::string, unsigned int>& tableLocationLookup, std::vector<SebText::GlyphData>& glyphs, int numGlyphs)
{
    // first: advance, second: left
    std::vector<std::pair<int, int>> layoutData; layoutData.resize(numGlyphs);

    // Get number of metrics from the 'hhea' table
    const unsigned char* cursor = ttfBytes.data() + tableLocationLookup["hhea"];

    cursor += 8; // unused: version, ascent, descent
    int lineGap = READ_I16; cursor += 2;
    int advanceWidthMax = READ_I16; cursor += 2;
    cursor += 22; // unused: minLeftSideBearing, minRightSideBearing, xMaxExtent, caretSlope/Offset, reserved, metricDataFormat
    int numAdvanceWidthMetrics = READ_I16; cursor += 2;

    // Get the advance width and leftsideBearing metrics from the 'hmtx' table
    cursor = ttfBytes.data() + tableLocationLookup["hmtx"];
    int lastAdvanceWidth = 0;

    for (int i = 0; i < numAdvanceWidthMetrics; i++)
    {
        int advanceWidth = READ_U16; cursor += 2;
        int leftSideBearing = READ_I16; cursor += 2;
        lastAdvanceWidth = advanceWidth;

        layoutData[i] = { advanceWidth, leftSideBearing };
    }

    // Some fonts have a run of monospace characters at the end
    int numRem = numGlyphs - numAdvanceWidthMetrics;

    for (int i = 0; i < numRem; i++)
    {
        int leftSideBearing = READ_I16; cursor += 2;
        int glyphIndex = numAdvanceWidthMetrics + i;

        layoutData[glyphIndex] = { lastAdvanceWidth, leftSideBearing };
    }

    // Apply
    for (SebText::GlyphData& c : glyphs)
    {
        c.AdvanceWidth = layoutData[c.GlyphIndex].first;
        c.LeftSideBearing = layoutData[c.GlyphIndex].second;
    }
}

SebText::FontData::FontData(const std::string& fontFilePath)
{
    // --- Load file to memory ---
    long ttfFileSize;
    FILE* fontFile = fopen(fontFilePath.c_str(), "rb");
    fseek(fontFile, 0, SEEK_END);
    ttfFileSize = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);
    TtfBytes.resize(ttfFileSize);
    fread(TtfBytes.data(), ttfFileSize, 1, fontFile);
    fclose(fontFile);

    // --- Get table locations ---
    std::unordered_map<std::string, unsigned int> tableLocationLookup;
    ReadTableLocations(TtfBytes, tableLocationLookup);
    unsigned int glyphTableLocation = tableLocationLookup["glyf"];
    unsigned int locaTableLocation = tableLocationLookup["loca"];
    unsigned int cmapLocation = tableLocationLookup["cmap"];

    // ---- Read Head Table ----
    const unsigned char* cursor = TtfBytes.data();
    cursor += tableLocationLookup["head"];
    cursor += 18;
    // Design units to Em size (range from 64 to 16384)
    UnitsPerEm = READ_U16; cursor += 2;
    cursor += 30;
    // Number of bytes used by the offsets in the 'loca' table (for looking up glyph locations)
    int numBytesPerLocationLookup = (READ_I16 == 0 ? 2 : 4);

    // --- Read 'maxp' table ---
    cursor = TtfBytes.data();
    cursor += tableLocationLookup["maxp"];
    cursor += 4;
    int numGlyphs = READ_U16; cursor += 2;
    std::vector<unsigned int> glyphLocations;
    GetAllGlyphLocations(TtfBytes, numGlyphs, numBytesPerLocationLookup, locaTableLocation, glyphTableLocation, glyphLocations);

    std::vector<std::pair<unsigned int, unsigned int>> mappings;
    GetUnicodeToGlyphIndexMappings(TtfBytes, cmapLocation, mappings);

    ReadAllGlyphs(TtfBytes, glyphLocations, mappings, Glyphs);

    ApplyLayoutInfo(TtfBytes, tableLocationLookup, Glyphs, numGlyphs);

    bool foundMissingGlyph = false;
    for (unsigned int i = 0; i < Glyphs.size(); i++)
    {
        GlyphData& c = Glyphs[i];
        glyphLookup.insert({ c.UnicodeValue, i });
        if (c.GlyphIndex == 0) { MissingGlyph = &c; foundMissingGlyph = true; }
    }

    assert(foundMissingGlyph);
    //if (MissingGlyph == null) throw new System.Exception("No missing character glyph provided!");
}

bool SebText::FontData::TryGetGlyph(unsigned int unicode, const GlyphData*& character) const
{
    bool found = (glyphLookup.find(unicode) != glyphLookup.end());
    character = found ? &Glyphs[glyphLookup.at(unicode)] : MissingGlyph;
    return found;
}














//SebText::FontData::FontData(const std::string& fontFilePath)
//{
//	long size;
//	unsigned char* fontBuffer;
//
//	FILE* fontFile = fopen(fontFilePath.c_str(), "rb");
//	fseek(fontFile, 0, SEEK_END);
//	size = ftell(fontFile); /* how long is the file ? */
//	fseek(fontFile, 0, SEEK_SET); /* reset */
//
//	fontBuffer = (unsigned char*)malloc(size);
//
//	fread(fontBuffer, size, 1, fontFile);
//	fclose(fontFile);
//
//	/* prepare font */
//	stbtt_fontinfo info;
//	if (!stbtt_InitFont(&info, fontBuffer, 0))
//	{
//		printf("[FontData] Failed\n");
//	}
//
//	printf("[FontData] Glyph count: %d\n", info.numGlyphs);
//
//	//stbtt_FindGlyphIndex(&info, 0);
//
//	//int ascent, descent, lineGap;
//	//stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
//	//printf("[FontData] Glyph[0] vmetrics: %d %d %d\n", ascent, descent, lineGap);
//
//	stbtt_vertex* vertices = nullptr;
//	//int num_verts = stbtt_GetCodepointShape(&info, '5', &vertices);
//	//printf("[FontData] %d\n", stbtt_FindGlyphIndex(&info, '0'));
//	//int i = 20;
//	for (int i = 0; i < info.numGlyphs; i++)
//	{
//		int num_verts = stbtt_GetGlyphShape(&info, i, &vertices);
//		//if (num_verts == 0)
//			//continue;
//
//		//out.emplace_back();
//		Glyphs.emplace_back();
//		for (int j = 0; j < num_verts; j++)
//		{
//			if (Glyphs.back().Points.size() == 0 || Glyphs.back().Points.back().X != vertices[j].cx && Glyphs.back().Points.back().Y != vertices[j].cy)
//				Glyphs.back().Points.emplace_back(vertices[j].cx, vertices[j].cy, false);
//			Glyphs.back().Points.emplace_back(vertices[j].x, vertices[j].y, true);
//			Glyphs.back().Points.emplace_back(vertices[j].cx1, vertices[j].cy1, false);
//			//out.back().push_back({ vertices[j].x, vertices[j].y });
//		}
//
//		//char out = '\0';
//		//for (char c : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
//			//if (stbtt_FindGlyphIndex(&info, c) != 0) { out = c; break; }
//		//if (num_verts == 36)
//		//printf("[FontData] Number of vertices for glyph %d: %d\n", i, num_verts);
//		//if (out != '\0')
//			//printf("[FontData]          Codepoint for glyph %d: %c\n", i, out);
//		//	if (num_verts > 0)
//		//		printf("lal");
//	}
//	//int num_verts = stbtt_GetGlyphShape(&info, 4, &vertices);
//	//int num_verts = stbtt_GetCodepointShape(&info, 0, &vertices);
//	//printf("[FontData] Glyph[0] point count: %d\n", num_verts);
//
//	//int x0, x1, y0, y1;
//	//stbtt_GetGlyphBox(&info, 1, &x0, &y0, &x1, &y1);
//	//printf("[FontData] Glyph[0] minX: %d\n", x0);
//	//printf("[FontData] Glyph[0] maxX: %d\n", x1);
//	//printf("[FontData] Glyph[0] minY: %d\n", y0);
//	//printf("[FontData] Glyph[0] maxY: %d\n", y1);
//}