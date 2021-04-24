#pragma once

#include <Scene/Entity.h>

namespace sf {

	struct Base {

		bool isEntityEnabled;
		Entity entity;
		Base(Entity entity);
	};
}