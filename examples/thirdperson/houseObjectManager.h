#include <FileUtils.h>

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

	bool ReadLodFile(const std::string& filename,
					std::vector<uint32_t>& remeshResVector,
					std::vector<float>& ratioVector,
					std::vector<float>& distanceVector)
	{
		remeshResVector.clear();
		ratioVector.clear();
		distanceVector.clear();

		std::ifstream file(filename);
		if (!file.is_open())
		{
			std::cerr << "Error: Could not open file " << filename << '\n';
			return false;
		}

		std::string line;
		if (std::getline(file, line))
		{
			std::stringstream ss(line);
			uint32_t val;
			while (ss >> val)
				remeshResVector.push_back(val);
		}
		else
			return false;

		if (std::getline(file, line))
		{
			std::stringstream ss(line);
			float val;
			while (ss >> val)
				ratioVector.push_back(val);
		}
		else
			return false;

		if (std::getline(file, line))
		{
			std::stringstream ss(line);
			float val;
			while (ss >> val)
				distanceVector.push_back(val);
		}
		else
			return false;

		return true;
	}

	void LoadAsset(const std::string& path, const sf::Buffer::Layout& vertexBufferLayout)
	{
		int gltfid = sf::GltfImporter::Load(path);

		std::vector<uint32_t> remeshResVector;
		std::vector<float> ratioVector;
		std::vector<float> distanceVector;
		bool lodReadResult = ReadLodFile(sf::FileUtils::RemoveExtension(path) + std::string(".lod"), remeshResVector, ratioVector, distanceVector);
		assert(lodReadResult);

		meshDataPerObject[path] = new sf::MeshData();
		sf::GltfImporter::GenerateMeshData(gltfid, vertexBufferLayout, *meshDataPerObject[path]);
		lodGroupPerObject[path] = new sf::LODGroup();
		lodGroupPerObject[path]->Create(*meshDataPerObject[path], ratioVector.data(), distanceVector.data(), ratioVector.size(), remeshResVector.data());
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