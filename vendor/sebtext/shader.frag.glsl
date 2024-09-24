#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragInPos;
layout(location = 1) flat in int fragInDataOffset;

layout (std430, binding = 2) buffer bezierSSBO
{
	vec2 BezierData[];
};
layout (std430, binding = 3) buffer metadataSSBO
{
	int GlyphMetaData[];
};

uniform vec4 textCol = vec4(0.0, 0.0, 0.0, 0.5);

// Calculate roots of quadratic equation(value/s for which: a×t ^ 2 + b×t + c = 0)
vec2 CalculateQuadraticRoots(float a, float b, float c)
{
	const float epsilon = 1e-5;
	vec2 roots = vec2(-99999, -99999);

	// For a straight line, solve: b×t + c = 0; therefore t = -c/b
	if (abs(a) < epsilon)
	{
		if (b != 0) roots[0] = -c / b;
	}
	else
	{
		// Solve using quadratic formula: t = (-b ± sqrt(b^2 - 4ac)) / (2a)
		// If the value under the sqrt is negative, the equation has no real roots
		float discriminant = b * b - 4 * a * c;

		// Allow discriminant to be slightly negative to avoid a curve being missed due
		// to precision limitations. Must be clamped to zero before it's used in sqrt though!
		if (discriminant > -epsilon)
		{
			float s = sqrt(max(0, discriminant));
			roots[0] = (-b + s) / (2 * a);
			roots[1] = (-b - s) / (2 * a);
		}
	}

	return roots;
}

// Calculate the fraction [0,1] of the pixel that is covered by the glyph (along the x axis).
// This is done by looking at the distances to the intersection points of a horizontal ray
// (at the pixel pos) with all the curves of the glyph.
float CalculateHorizontalCoverage(vec2 pixelPos, int dataOffset, float pixelSize)
{
	float coverage = 0;
	float invPixelSize = 1 / pixelSize;

	int pointOffset = GlyphMetaData[dataOffset];
	int numContours = GlyphMetaData[dataOffset + 1];
	dataOffset += 2;

	// Loop over all contours
	for (int contourIndex = 0; contourIndex < numContours; contourIndex++)
	{
		int numPoints = GlyphMetaData[dataOffset + contourIndex];

		for (int i = 0; i < numPoints; i += 2)
		{
			// Get positions of curve's control points relative to the current pixel
			vec2 p0 = BezierData[i + 0 + pointOffset] - pixelPos;
			vec2 p1 = BezierData[i + 1 + pointOffset] - pixelPos;
			vec2 p2 = BezierData[i + 2 + pointOffset] - pixelPos;

			// Check if curve segment is going downwards (this means that a ray crossing
			// it from left to right would be exiting the shape at this point).
			// Note: curves are assumed to be monotonic (strictly increasing or decreasing on the y axis)
			bool isDownwardCurve = p0.y > 0 || p2.y < 0;

			// Skip curves that are entirely above or below the ray
			// When two curves are in the same direction (upward or downward), only one of them should be
			// counted at their meeting point to avoid double-counting. When in opposite directions, however,
			// the curve is not crossing the contour (but rather just grazing it) and so the curves should
			// either both be skipped, or both counted (so as not to affect the end result).
			if (isDownwardCurve)
			{
				if (p0.y < 0 && p2.y <= 0) continue;
				if (p0.y > 0 && p2.y >= 0) continue;
			}
			else
			{
				if (p0.y <= 0 && p2.y < 0) continue;
				if (p0.y >= 0 && p2.y > 0) continue;
			}

			// Calculate a,b,c of quadratic equation for current bezier curve
			vec2 a = p0 - 2 * p1 + p2;
			vec2 b = 2 * (p1 - p0);
			vec2 c = p0;

			// Calculate roots to see if ray intersects curve segment.
			// Note: intersection is allowed slightly outside of [0, 1] segment to tolerate precision issues.
			const float epsilon = 1e-4;
			vec2 roots = CalculateQuadraticRoots(a.y, b.y, c.y);
			bool onSeg0 = roots[0] >= -epsilon && roots[0] <= 1 + epsilon;
			bool onSeg1 = roots[1] >= -epsilon && roots[1] <= 1 + epsilon;

			// Calculate distance to intersection (negative if to left of ray)
			float t0 = clamp(roots[0], 0.0, 1.0);
			float t1 = clamp(roots[1], 0.0, 1.0);
			float intersect0 = a.x * t0 * t0 + b.x * t0 + c.x;
			float intersect1 = a.x * t1 * t1 + b.x * t1 + c.x;

			// Calculate the fraction of the ray that passes through the glyph (within the current pixel):
			// A value [0, 1] is calculated based on where the intersection occurs: 0 at the left edge of
			// the pixel, increasing to 1 at the right edge. This value is added to the total coverage
			// value when the ray exits a shape, and subtracted when the ray enters a shape.
			int sign = isDownwardCurve ? 1 : -1;
			if (onSeg0) coverage += clamp(0.5 + intersect0 * invPixelSize, 0.0, 1.0) * sign;
			if (onSeg1) coverage += clamp(0.5 + intersect1 * invPixelSize, 0.0, 1.0) * sign;

		}

		pointOffset += numPoints + 1; 
	}

	return clamp(coverage, 0.0, 1.0);
}

// Run for every pixel in the glyph's quad mesh
void main()
{
	// Size of pixel in glyph space
	float pixelSize = dFdx(fragInPos.x);
	float alphaSum = 0;

	// Render 3 times (with slight y offset) for anti-aliasing
	for (int yOffset = -1; yOffset <= 1; yOffset++)
	{
		vec2 samplePos = fragInPos + vec2(0, yOffset) * pixelSize / 3.0;
		float coverage = CalculateHorizontalCoverage(samplePos, fragInDataOffset, pixelSize);
		alphaSum += coverage;
	}

	float alpha = alphaSum / 3.0;
	outColor = vec4(textCol.rgb, min(alpha, textCol.a));
}