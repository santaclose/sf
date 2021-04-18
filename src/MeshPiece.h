#pragma once

#include <Material.h>
#include <Defaults.h>

namespace sf {

	struct MeshPiece {

		unsigned int indexStart;
		Material* material = nullptr;
		MeshPiece::MeshPiece(unsigned int indexStart)
		{
			this->indexStart = indexStart;
			this->material = &Defaults::material;
		}
	};
}