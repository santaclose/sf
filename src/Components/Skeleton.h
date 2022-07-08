#pragma once

#include <SkeletonData.h>

namespace sf {

	struct Skeleton
	{
	private:
		static int counter;

	public:
		int id;
		const SkeletonData* skeletonData;

		Skeleton(const SkeletonData* skeletonData);
	};
}