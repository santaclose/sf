

namespace hom
{
	float Sink_4325117_LodDistances[] = { 3.000000f, 10.000000f, 56.000000f };
	float Sink_4325117_LodRatios[] = { 1.000000f, 0.400000f, 0.100000f };
	float Toilet_7291921_LodDistances[] = { 3.000000f, 10.000000f, 18.000000f, 56.000000f };
	float Toilet_7291921_LodRatios[] = { 1.000000f, 0.400000f, 0.200000f, 0.001000f };
	float _1950s_Refrigerator_6463987_LodDistances[] = { 6.000000f, 8.000000f, 12.000000f, 56.000000f };
	float _1950s_Refrigerator_6463987_LodRatios[] = { 1.000000f, 0.500000f, 0.250000f, 0.125000f };
	float WashingMachine_9800066_LodDistances[] = { 3.000000f, 10.000000f, 56.000000f };
	float WashingMachine_9800066_LodRatios[] = { 1.000000f, 0.400000f, 0.010000f };
	float MotelShower_4150183_LodDistances[] = { 3.000000f, 7.000000f, 30.000000f };
	float MotelShower_4150183_LodRatios[] = { 1.000000f, 0.250000f, 0.005000f };
	float Towel_Rack_7188108_LodDistances[] = { 3.000000f, 6.000000f, 56.000000f };
	float Towel_Rack_7188108_LodRatios[] = { 1.000000f, 0.800000f, 0.010000f };
	float Dryer_4537255_LodDistances[] = { 3.000000f, 56.000000f };
	float Dryer_4537255_LodRatios[] = { 1.000000f, 0.000010f };
	float MotelToilet_7890507_LodDistances[] = { 3.000000f, 10.000000f, 56.000000f };
	float MotelToilet_7890507_LodRatios[] = { 1.000000f, 0.400000f, 0.010000f };
	float MotelBed_a_8538645_LodDistances[] = { 5.000000f, 10.000000f, 56.000000f };
	float MotelBed_a_8538645_LodRatios[] = { 1.000000f, 0.323000f, 0.015000f };

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
		"assets/examples/rb3d/UpholsteredWoodBaseSofa_8248509_f.glb"
	};

	void LoadAsset(const std::string& path, const sf::BufferLayout& vertexBufferLayout)
	{
		int gltfid = sf::GltfImporter::Load(path);

		float* ratios = nullptr;
		float* distances = nullptr;
		uint32_t lodCount = ~0;
		if (path == "assets/examples/rb3d/Sink_4325117_f.glb")
		{
			distances = Sink_4325117_LodDistances;
			ratios = Sink_4325117_LodRatios;
			lodCount = sizeof(Sink_4325117_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/Toilet_7291921_f.glb")
		{
			distances = Toilet_7291921_LodDistances;
			ratios = Toilet_7291921_LodRatios;
			lodCount = sizeof(Toilet_7291921_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/1950s Refrigerator_6463987_f.glb")
		{
			distances = _1950s_Refrigerator_6463987_LodDistances;
			ratios = _1950s_Refrigerator_6463987_LodRatios;
			lodCount = sizeof(_1950s_Refrigerator_6463987_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/WashingMachine_9800066_f.glb")
		{
			distances = WashingMachine_9800066_LodDistances;
			ratios = WashingMachine_9800066_LodRatios;
			lodCount = sizeof(WashingMachine_9800066_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/MotelShower_4150183_f.glb")
		{
			distances = MotelShower_4150183_LodDistances;
			ratios = MotelShower_4150183_LodRatios;
			lodCount = sizeof(MotelShower_4150183_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/Towel Rack_7188108_f.glb")
		{
			distances = Towel_Rack_7188108_LodDistances;
			ratios = Towel_Rack_7188108_LodRatios;
			lodCount = sizeof(Towel_Rack_7188108_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/TowelRack_8073790_f.glb")
		{
			distances = Towel_Rack_7188108_LodDistances;
			ratios = Towel_Rack_7188108_LodRatios;
			lodCount = sizeof(Towel_Rack_7188108_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/Dryer_4537255_f.glb")
		{
			distances = Dryer_4537255_LodDistances;
			ratios = Dryer_4537255_LodRatios;
			lodCount = sizeof(Dryer_4537255_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/MotelToilet_7890507_f.glb")
		{
			distances = MotelToilet_7890507_LodDistances;
			ratios = MotelToilet_7890507_LodRatios;
			lodCount = sizeof(MotelToilet_7890507_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/MotelBed-a_8538645_f.glb")
		{
			distances = MotelBed_a_8538645_LodDistances;
			ratios = MotelBed_a_8538645_LodRatios;
			lodCount = sizeof(MotelBed_a_8538645_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/MotelBed-b_4880464_f.glb")
		{
			distances = MotelBed_a_8538645_LodDistances;
			ratios = MotelBed_a_8538645_LodRatios;
			lodCount = sizeof(MotelBed_a_8538645_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/Nightstand_2001378_f.glb")
		{
			distances = Toilet_7291921_LodDistances;
			ratios = Toilet_7291921_LodRatios;
			lodCount = sizeof(Toilet_7291921_LodRatios) / sizeof(float);
		}
		else if (path == "assets/examples/rb3d/UpholsteredWoodBaseSofa_8248509_f.glb")
		{
			distances = MotelBed_a_8538645_LodDistances;
			ratios = MotelBed_a_8538645_LodRatios;
			lodCount = sizeof(MotelBed_a_8538645_LodRatios) / sizeof(float);
		}

		assert(ratios != nullptr && distances != nullptr && lodCount != ~0);

		meshDataPerObject[path] = new sf::MeshData();
		sf::GltfImporter::GenerateMeshData(gltfid, vertexBufferLayout, *meshDataPerObject[path]);
		lodGroupPerObject[path] = new sf::LODGroup();
		lodGroupPerObject[path]->Create(*meshDataPerObject[path], ratios, distances, lodCount);
	}

	void Initialize(const sf::BufferLayout& vertexBufferLayout)
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
		else // if (type == "fridge")
		{
			return lodGroupPerObject["assets/examples/rb3d/1950s Refrigerator_6463987_f.glb"];
		}
	}
}