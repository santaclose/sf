#include <FileUtils.h>
#include <VoxelVolumeData.h>
#include <MeshProcessor.h>

#include <fstream>
#include <sstream>

namespace hom
{
	std::unordered_map<std::string, sf::MeshData*> meshDataPerObject;
	std::unordered_map<std::string, sf::LODGroup*> lodGroupPerObject;

	const char* assetPaths[] = {
		"assets/examples/rb3d/Sink_4325117_f.glb",
		"assets/examples/rb3d/Toilet_7291921_f.glb",
		"assets/examples/rb3d/1950s Refrigerator_6463987_f.glb",
		"assets/examples/rb3d/WashingMachine_9800066_f.glb",
		"assets/examples/rb3d/MotelShower_4150183_f.glb",
		"assets/examples/rb3d/Towel Rack_7188108_f.glb",
		"assets/examples/rb3d/TowelRack_8073790_f.glb",
		"assets/examples/rb3d/Dryer_4537255_f.glb",
		"assets/examples/rb3d/MotelToilet_7890507_f.glb",
		"assets/examples/rb3d/MotelBed-a_8538645_f.glb",
		"assets/examples/rb3d/MotelBed-b_4880464_f.glb",
		"assets/examples/rb3d/Nightstand_2001378_f.glb",
		"assets/examples/rb3d/UpholsteredWoodBaseSofa_8248509_f.glb",
		"assets/examples/rb3d/European Ash 01_7694850.glb",
		"assets/examples/rb3d/River Red Gum 01_8489243.glb",
		"assets/examples/rb3d/Sugar Maple 01_2988619.glb",
	};

	void LoadAsset(const std::string& path, const sf::Buffer::Layout& vertexBufferLayout)
	{
		std::string cachedMeshPath = sf::FileUtils::RemoveExtension(path) + std::string(".mesh");
		std::string lodPath = sf::FileUtils::RemoveExtension(path) + std::string(".lod");

		sf::LODSpec ls;
		bool loadSucceeded = ls.LoadFromFile(lodPath.c_str());
		assert(loadSucceeded);

		meshDataPerObject[path] = new sf::MeshData();
		if (!meshDataPerObject[path]->LoadFromFile(cachedMeshPath.c_str()))
		{
			if (sf::FileUtils::ExtensionIs(path, "obj"))
			{
				int objid = sf::ObjImporter::Load(path);
				sf::ObjImporter::GenerateMeshData(objid, vertexBufferLayout, *meshDataPerObject[path]);
			}
			else
			{
				int gltfid = sf::GltfImporter::Load(path);
				sf::GltfImporter::GenerateMeshData(gltfid, vertexBufferLayout, *meshDataPerObject[path]);
			}

			sf::VoxelVolumeData vv;
			vv.BuildFromMesh(*meshDataPerObject[path], 0.03f);
			sf::MeshProcessor::ComputeVertexAmbientOcclusion(*meshDataPerObject[path], &vv);
			meshDataPerObject[path]->SaveToFile(cachedMeshPath.c_str());
		}

		lodGroupPerObject[path] = new sf::LODGroup();
		lodGroupPerObject[path]->Create(*meshDataPerObject[path], ls);
	}

	void Initialize(const sf::Buffer::Layout& vertexBufferLayout)
	{
		for (uint32_t assetIndex = 0; assetIndex < (sizeof(assetPaths) / sizeof(char*)); assetIndex++)
		{
			LoadAsset(assetPaths[assetIndex], vertexBufferLayout);
		}
	}

	const sf::LODGroup* GetLodsByType(const std::string type)
	{
		if (type == "toilet")
		{
			if (sf::Random::Int(2) == 0)
				return lodGroupPerObject["assets/examples/rb3d/MotelToilet_7890507_f.glb"];
			else
				return lodGroupPerObject["assets/examples/rb3d/Toilet_7291921_f.glb"];
		}
		else if (type == "sink")
		{
			return lodGroupPerObject["assets/examples/rb3d/Sink_4325117_f.glb"];
		}
		else if (type == "shower")
		{
			return lodGroupPerObject["assets/examples/rb3d/MotelShower_4150183_f.glb"];
		}
		else if (type == "washingMachine")
		{
			return lodGroupPerObject["assets/examples/rb3d/WashingMachine_9800066_f.glb"];
		}
		else if (type == "dryer")
		{
			return lodGroupPerObject["assets/examples/rb3d/Dryer_4537255_f.glb"];
		}
		else if (type == "nightStand")
		{
			return lodGroupPerObject["assets/examples/rb3d/Nightstand_2001378_f.glb"];
		}
		else if (type == "sofa")
		{
			return lodGroupPerObject["assets/examples/rb3d/UpholsteredWoodBaseSofa_8248509_f.glb"];
		}
		else if (type == "towelRack")
		{
			if (sf::Random::Int(2) == 0)
				return lodGroupPerObject["assets/examples/rb3d/Towel Rack_7188108_f.glb"];
			else
				return lodGroupPerObject["assets/examples/rb3d/TowelRack_8073790_f.glb"];
		}
		else if (type == "bed")
		{
			if (sf::Random::Int(2) == 0)
				return lodGroupPerObject["assets/examples/rb3d/MotelBed-a_8538645_f.glb"];
			else
				return lodGroupPerObject["assets/examples/rb3d/MotelBed-b_4880464_f.glb"];
		}
		else if (type == "fridge")
		{
			return lodGroupPerObject["assets/examples/rb3d/1950s Refrigerator_6463987_f.glb"];
		}
		else // if (type == "tree")
		{
			int rv = sf::Random::Int(3);
			switch (rv)
			{
			case 0: return lodGroupPerObject["assets/examples/rb3d/European Ash 01_7694850.glb"];
			case 1: return lodGroupPerObject["assets/examples/rb3d/River Red Gum 01_8489243.glb"];
			case 2: return lodGroupPerObject["assets/examples/rb3d/Sugar Maple 01_2988619.glb"];
			default: assert(false); return nullptr;
			}
		}
	}
}