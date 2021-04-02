#pragma once

#include <Model.h>

namespace sf {

	class ModelProcessor {
	public:
		static void ComputeNormals(Model& model, bool normalize = false);
		static void ComputeTangentSpace(Model& model);
		static void BakeAo(Model& model, int rayCount = 30, bool onlyCastRaysUpwards = false, bool intersectFromBothSides = true, float rayOriginOffset = 0.001f, float rayDistance = 5.0f);
	};
}