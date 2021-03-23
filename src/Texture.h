#pragma once

#include <string>

class Texture
{
public:
	enum Type {
		None = -1, Albedo = 0, Normals = 1, Roughness = 2, Metallic = 3, HDR = 4
	};

private:
	unsigned int m_gl_id;
	int m_width, m_height;
	Type m_type = Type::None;

public:
	void Create(unsigned int width, unsigned int height, bool mipmap = true, bool isHdr = false, int channelCount = 3);
	void CreateFromGltf(unsigned int gltfID, unsigned int textureIndex);
	void CreateFromFile(const std::string& path, Type type, bool mipmap = true, bool flipVertically = true);
	void ComputeMipmap();
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline unsigned int GlId() const { return m_gl_id; }
	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};