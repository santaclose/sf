#include <glm/glm.hpp>
#include <imnodes.h>
#include <cassert>

#define NODE_EDITOR_QUATERNION_PIN_COLOR IM_COL32(100, 100, 255, 255)
#define NODE_EDITOR_QUATERNION_PIN_HOVERED_COLOR IM_COL32(100, 100, 200, 255)

#define NODE_EDITOR_VECTOR_PIN_COLOR IM_COL32(100, 255, 100, 255)
#define NODE_EDITOR_VECTOR_PIN_HOVERED_COLOR IM_COL32(100, 200, 100, 255)

#define NODE_EDITOR_FLOAT_PIN_COLOR IM_COL32(255, 100, 100, 255)
#define NODE_EDITOR_FLOAT_PIN_HOVERED_COLOR IM_COL32(200, 100, 100, 255)

#define MAX_NODES_TO_THE_RIGHT 256

#define ADD_PIN(inputOrOutput, type, name, id) \
	ImNodes::PushColorStyle(ImNodesCol_Pin, NODE_EDITOR_##type##_PIN_COLOR); \
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, NODE_EDITOR_##type##_PIN_HOVERED_COLOR); \
	ImNodes::Begin##inputOrOutput##Attribute(nodeId * 1000 + (id)); \
	ImGui::Text(name); \
	ImNodes::End##inputOrOutput##Attribute(); \
	ImNodes::PopColorStyle(); \
	ImNodes::PopColorStyle();


namespace sf::Game
{
	enum class RetargetNodeType { InputQuat, QuatMultiply, InputBones, OutputBones, Slerp, ScaleVector, RotSandwich, INVALID };
	enum class RetargetNodePinType { Float, Quat, Vector, INVALID };
	struct NodeInputQuat
	{
		RetargetNodeType type;
		const char* name;
		int nodesToTheRight[MAX_NODES_TO_THE_RIGHT];
		glm::vec3 euler;
		glm::quat quat;
	};
	struct NodeQuatMultiply
	{
		RetargetNodeType type;
		const char* name;
		int nodesToTheRight[MAX_NODES_TO_THE_RIGHT];
		glm::quat quat;
		const glm::quat* inputA;
		const glm::quat* inputB;
	};
	struct NodeInputBones
	{
		RetargetNodeType type;
		const char* name;
		int nodesToTheRight[MAX_NODES_TO_THE_RIGHT];
		const char** boneNames;
		const std::vector<Transform>* localTransforms;
	};
	struct NodeOutputBones
	{
		RetargetNodeType type;
		const char* name;
		int nodesToTheRight[MAX_NODES_TO_THE_RIGHT];
		const char** boneNames;
		std::vector<Transform>* localTransforms;
		const glm::vec3** inputPos;
		const glm::quat** inputRot;
		const float** inputScale;
	};
	struct NodeSlerp
	{
		RetargetNodeType type;
		const char* name;
		int nodesToTheRight[MAX_NODES_TO_THE_RIGHT];
		glm::quat quat;
		const glm::quat* inputA;
		const glm::quat* inputB;
		float t;
	};
	struct NodeScaleVector
	{
		RetargetNodeType type;
		const char* name;
		int nodesToTheRight[MAX_NODES_TO_THE_RIGHT];
		const glm::vec3* input;
		float scale;
		glm::vec3 output;
	};
	struct NodeRotSandwich
	{
		RetargetNodeType type;
		const char* name;
		int nodesToTheRight[MAX_NODES_TO_THE_RIGHT];
		glm::vec3 eulerA;
		glm::vec3 eulerB;
		glm::quat output;
		const glm::quat* input;
		bool useInverse;
	};

	union RetargetNode
	{
		NodeInputQuat inputQuat;
		NodeQuatMultiply quatMultiply;
		NodeInputBones inputBones;
		NodeOutputBones outputBones;
		NodeSlerp slerp;
		NodeScaleVector scaleVector;
		NodeRotSandwich rotSandwich;
	};

	void RetargetNodeAddToRight(RetargetNode& node, int nodeToAdd)
	{
		int i = 0;
		for (; i < MAX_NODES_TO_THE_RIGHT && node.inputQuat.nodesToTheRight[i] != -1; i++);
		assert(i != MAX_NODES_TO_THE_RIGHT);
		node.inputQuat.nodesToTheRight[i] = nodeToAdd;
		node.inputQuat.nodesToTheRight[i + 1] = -1;
	}

	void RetargetNodeRemoveFromRight(RetargetNode& node, int nodeToRemove)
	{
		int i = 0;
		for (; i < MAX_NODES_TO_THE_RIGHT && node.inputQuat.nodesToTheRight[i] != nodeToRemove; i++);
		assert(i < MAX_NODES_TO_THE_RIGHT - 1);
		while (true)
		{
			node.inputQuat.nodesToTheRight[i] = node.inputQuat.nodesToTheRight[i + 1];
			if (node.inputQuat.nodesToTheRight[i] == -1)
				break;
			i++;
		}
	}

	void RetargetNodeSetInput(RetargetNode& node, int pinIndex, const void* pointer)
	{
		switch (node.inputQuat.type)
		{
		case RetargetNodeType::InputQuat:
		{
			assert(false);
			return;
		}
		case RetargetNodeType::QuatMultiply:
		{
			if (pinIndex == 0)
				node.quatMultiply.inputA = (const glm::quat*) pointer;
			else
				node.quatMultiply.inputB = (const glm::quat*) pointer;
			return;
		}
		case RetargetNodeType::OutputBones:
		{
			int transformIndex = pinIndex / 3;
			int transformComponent = pinIndex % 3;
			if (transformComponent == 0)
				node.outputBones.inputPos[transformIndex] = (const glm::vec3*) pointer;
			else if (transformComponent == 1)
				node.outputBones.inputRot[transformIndex] = (const glm::quat*) pointer;
			else
				node.outputBones.inputScale[transformIndex] = (const float*) pointer;
			return;
		}
		case RetargetNodeType::InputBones:
		{
			assert(false);
			return;
		}
		case RetargetNodeType::Slerp:
		{
			if (pinIndex == 0)
				node.slerp.inputA = (glm::quat*) pointer;
			else
				node.slerp.inputB = (glm::quat*) pointer;
			return;
		}
		case RetargetNodeType::ScaleVector:
		{
			node.scaleVector.input = (glm::vec3*) pointer;
			return;
		}
		case RetargetNodeType::RotSandwich:
		{
			node.rotSandwich.input = (glm::quat*) pointer;
			return;
		}
		default:
		{
			printf("Node type was: %u\n", (uint32_t)node.inputQuat.type);
			assert(false);
		}
		}
	}

	const void* RetargetNodeGetOutput(RetargetNode& node, int pinIndex)
	{
		switch (node.inputQuat.type)
		{
		case RetargetNodeType::InputQuat:
		{
			return &node.inputQuat.quat;
		}
		case RetargetNodeType::QuatMultiply:
		{
			return &node.quatMultiply.quat;
		}
		case RetargetNodeType::OutputBones:
		{
			assert(false);
			return nullptr;
		}
		case RetargetNodeType::InputBones:
		{
			int transformIndex = pinIndex / 3;
			int transformComponent = pinIndex % 3;
			if (transformComponent == 0)
				return &(node.inputBones.localTransforms[0][transformIndex].position);
			if (transformComponent == 1)
				return &(node.inputBones.localTransforms[0][transformIndex].rotation);
			return &(node.inputBones.localTransforms[0][transformIndex].scale);
		}
		case RetargetNodeType::Slerp:
		{
			return &node.slerp.quat;
		}
		case RetargetNodeType::ScaleVector:
		{
			return &node.scaleVector.output;
		}
		case RetargetNodeType::RotSandwich:
		{
			return &node.rotSandwich.output;
		}
		default:
		{
			printf("Node type was: %u\n", (uint32_t)node.inputQuat.type);
			assert(false);
			return nullptr;
		}
		}
	}

	void RetargetNodeRun(RetargetNode& node)
	{
		switch (node.inputQuat.type)
		{
		case RetargetNodeType::InputQuat:
		{
			node.inputQuat.quat = glm::quat(glm::radians(node.inputQuat.euler));
			break;
		}
		case RetargetNodeType::QuatMultiply:
		{
			if (node.quatMultiply.inputA == nullptr || node.quatMultiply.inputB == nullptr)
				break;
			node.quatMultiply.quat = node.quatMultiply.inputB[0] * node.quatMultiply.inputA[0];
			break;
		}
		case RetargetNodeType::OutputBones:
		{
			for (int i = 0; i < node.outputBones.localTransforms->size(); i++)
			{
				if (node.outputBones.inputPos[i] != nullptr)
					node.outputBones.localTransforms[0][i].position = node.outputBones.inputPos[i][0];
				if (node.outputBones.inputRot[i] != nullptr)
					node.outputBones.localTransforms[0][i].rotation = node.outputBones.inputRot[i][0];
				if (node.outputBones.inputScale[i] != nullptr)
					node.outputBones.localTransforms[0][i].scale = node.outputBones.inputScale[i][0];
			}
			break;
		}
		case RetargetNodeType::InputBones:
		{
			// nothing to do
			break;
		}
		case RetargetNodeType::Slerp:
		{
			if (node.slerp.inputA == nullptr || node.slerp.inputB == nullptr)
				break;
			node.slerp.quat = glm::slerp(node.slerp.inputA[0], node.slerp.inputB[0], node.slerp.t);
			break;
		}
		case RetargetNodeType::ScaleVector:
		{
			if (node.scaleVector.input == nullptr)
				break;
			node.scaleVector.output = node.scaleVector.input[0] * node.scaleVector.scale;
			break;
		}
		case RetargetNodeType::RotSandwich:
		{
			if (node.rotSandwich.input == nullptr)
				break;
			if (!node.rotSandwich.useInverse)
				node.rotSandwich.output = glm::quat(glm::radians(node.rotSandwich.eulerB)) * node.rotSandwich.input[0] * glm::quat(glm::radians(node.rotSandwich.eulerA));
			else
			{
				glm::quat a = glm::quat(glm::radians(node.rotSandwich.eulerA));
				node.rotSandwich.output = glm::conjugate(a) * node.rotSandwich.input[0] * a;
			}
			break;
		}
		default:
		{
			printf("Node type was: %u\n", (uint32_t)node.inputQuat.type);
			assert(false);
		}
		}
	}

	void RetargetNodeInitialize(RetargetNodeType type, RetargetNode& node)
	{
		node.inputQuat.type = type;
		node.inputQuat.nodesToTheRight[0] = -1;
		switch (type)
		{
		case RetargetNodeType::InputQuat:
		{
			node.inputQuat.name = "Quaternion from euler";
			node.inputQuat.euler = glm::vec3(0.0f, 0.0f, 0.0f);
			node.inputQuat.quat = glm::quat(node.inputQuat.euler);
			break;
		}
		case RetargetNodeType::QuatMultiply:
		{
			node.quatMultiply.name = "Multiply quaternions";
			node.quatMultiply.quat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			node.quatMultiply.inputA = nullptr;
			node.quatMultiply.inputB = nullptr;
			break;
		}
		case RetargetNodeType::OutputBones:
		{
			node.outputBones.name = "Output bones";
			node.outputBones.localTransforms = nullptr;
			node.outputBones.boneNames = nullptr;
			node.outputBones.inputPos = nullptr;
			node.outputBones.inputRot = nullptr;
			node.outputBones.inputScale = nullptr;
			break;
		}
		case RetargetNodeType::InputBones:
		{
			node.inputBones.name = "Input bones";
			node.inputBones.localTransforms = nullptr;
			node.inputBones.boneNames = nullptr;
			break;
		}
		case RetargetNodeType::Slerp:
		{
			node.slerp.name = "Slerp";
			node.slerp.quat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			node.slerp.inputA = nullptr;
			node.slerp.inputB = nullptr;
			node.slerp.t = 0.5f;
			break;
		}
		case RetargetNodeType::ScaleVector:
		{
			node.scaleVector.name = "Scale vector";
			node.scaleVector.input = nullptr;
			node.scaleVector.scale = 0.5f;
			break;
		}
		case RetargetNodeType::RotSandwich:
		{
			node.rotSandwich.name = "Rotation sandwich";
			node.rotSandwich.input = nullptr;
			node.rotSandwich.useInverse = true;
			node.rotSandwich.eulerA = glm::vec3(0.0f, 0.0f, 0.0f);
			node.rotSandwich.eulerB = glm::vec3(0.0f, 0.0f, 0.0f);
			break;
		}
		default:
		{
			printf("Node type was: %u\n", (uint32_t)node.inputQuat.type);
			assert(false);
		}
		}
	}
	bool RetargetNodeImGui(int nodeId, RetargetNode& node)
	{
		bool returnValue = false;
		ImNodes::BeginNode(nodeId);
		ImNodes::BeginNodeTitleBar();
		ImGui::Text("%s", node.inputQuat.name);
		ImNodes::EndNodeTitleBar();
		ImGui::PushItemWidth(80);

		switch (node.inputQuat.type)
		{
		case RetargetNodeType::InputQuat:
		{
			returnValue |= ImGui::DragFloat("X", &node.inputQuat.euler.x, 0.1f);
			returnValue |= ImGui::DragFloat("Y", &node.inputQuat.euler.y, 0.1f);
			returnValue |= ImGui::DragFloat("Z", &node.inputQuat.euler.z, 0.1f);
			ADD_PIN(Output, QUATERNION, "Out", 0);
			break;
		}
		case RetargetNodeType::QuatMultiply:
		{
			ADD_PIN(Input, QUATERNION, "A", 0);
			ADD_PIN(Input, QUATERNION, "B", 1);
			ADD_PIN(Output, QUATERNION, "Out", 2);
			break;
		}
		case RetargetNodeType::InputBones:
		{
			if (node.inputBones.localTransforms == nullptr)
				break;
			for (int i = 0; i < node.inputBones.localTransforms->size(); i++)
			{
				ImGui::TextUnformatted(node.inputBones.boneNames[i]);
				ADD_PIN(Output, VECTOR, "Bone translation", i * 3 + 0);
				ADD_PIN(Output, QUATERNION, "Bone rotation", i * 3 + 1);
				ADD_PIN(Output, FLOAT, "Bone scale", i * 3 + 2);
			}
			break;
		}
		case RetargetNodeType::OutputBones:
		{
			if (node.outputBones.localTransforms == nullptr)
				break;
			for (int i = 0; i < node.outputBones.localTransforms->size(); i++)
			{
				ImGui::TextUnformatted(node.outputBones.boneNames[i]);
				ADD_PIN(Input, VECTOR, "Bone translation", i * 3 + 0);
				ADD_PIN(Input, QUATERNION, "Bone rotation", i * 3 + 1);
				ADD_PIN(Input, FLOAT, "Bone scale", i * 3 + 2);
			}
			break;
		}
		case RetargetNodeType::Slerp:
		{
			ADD_PIN(Input, QUATERNION, "A", 0);
			ADD_PIN(Input, QUATERNION, "B", 1);
			returnValue |= ImGui::DragFloat("t", &node.slerp.t, 0.1f);
			ADD_PIN(Output, QUATERNION, "Out", 2);
			break;
		}
		case RetargetNodeType::ScaleVector:
		{
			ADD_PIN(Input, VECTOR, "In", 0);
			returnValue |= ImGui::DragFloat("scale", &node.scaleVector.scale, 0.1f);
			ADD_PIN(Output, VECTOR, "Out", 1);
			break;
		}
		case RetargetNodeType::RotSandwich:
		{
			returnValue |= ImGui::DragFloat("aX", &node.rotSandwich.eulerA.x, 0.1f);
			returnValue |= ImGui::DragFloat("aY", &node.rotSandwich.eulerA.y, 0.1f);
			returnValue |= ImGui::DragFloat("aZ", &node.rotSandwich.eulerA.z, 0.1f);
			ADD_PIN(Input, QUATERNION, "In", 0);
			returnValue |= ImGui::Checkbox("Use inverse", &node.rotSandwich.useInverse);
			if (!node.rotSandwich.useInverse)
			{
				returnValue |= ImGui::DragFloat("bX", &node.rotSandwich.eulerB.x, 0.1f);
				returnValue |= ImGui::DragFloat("bY", &node.rotSandwich.eulerB.y, 0.1f);
				returnValue |= ImGui::DragFloat("bZ", &node.rotSandwich.eulerB.z, 0.1f);
			}
			ADD_PIN(Output, QUATERNION, "Out", 1);
			break;
		}
		}

		ImGui::PopItemWidth();
		ImNodes::EndNode();
		return returnValue;
	}

	RetargetNodePinType RetargetGetPinType(RetargetNode& node, int pinIndex)
	{
		switch (node.inputQuat.type)
		{
		case RetargetNodeType::InputQuat:
		{
			return RetargetNodePinType::Quat;
		}
		case RetargetNodeType::QuatMultiply:
		{
			return RetargetNodePinType::Quat;
		}
		case RetargetNodeType::OutputBones:
		case RetargetNodeType::InputBones:
		{
			if (pinIndex % 3 == 0)
				return RetargetNodePinType::Vector;
			if (pinIndex % 3 == 1)
				return RetargetNodePinType::Quat;
			else
				return RetargetNodePinType::Float;
		}
		case RetargetNodeType::Slerp:
		{
			return RetargetNodePinType::Quat;
		}
		case RetargetNodeType::ScaleVector:
		{
			return RetargetNodePinType::Vector;
		}
		case RetargetNodeType::RotSandwich:
		{
			return RetargetNodePinType::Quat;
		}
		default:
		{
			assert(false);
			return RetargetNodePinType::INVALID;
		}
		}
	}
}