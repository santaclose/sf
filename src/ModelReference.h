#pragma once

#include <Object.h>
#include <Material.h>
#include <Model.h>

namespace sf {

	// a model reference only stores transform data
	class ModelReference : public Object
	{
		friend Model;

		Model* m_originalModel;
		void SendMatrixToShader();

	public:
		void CreateFomModel(Model& theModel);
	};
}