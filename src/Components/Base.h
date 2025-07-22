#pragma once

#include <Scene/Entity.h>

namespace sf {

	struct Base {

		bool isEntityEnabled;
		Entity entity;

		inline Base(Entity entity)
		{
			this->entity = entity;
			this->isEntityEnabled = true;
		}
	};
}