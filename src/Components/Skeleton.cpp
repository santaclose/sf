#include "Skeleton.h"

int sf::Skeleton::counter = 0;

sf::Skeleton::Skeleton(const SkeletonData* skeletonData)
{
	this->skeletonData = skeletonData;
	this->id = counter;
	counter++;
}
