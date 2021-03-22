#pragma once

#include <string>

class Texture
{
public:
	enum Type {
		Albedo = 0, Normals = 1, Roughness = 2, Metallic = 3, HDR = 4
	};

private:
	unsigned int m_gl_id;
	int m_width, m_height;
	Type m_type;

public:
	void CreateFromGltf(unsigned int gltfID, unsigned int textureIndex);
	void CreateFromFile(const std::string& path, Type type, bool mipmap = true);
	void ComputeMipmap();
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline unsigned int GlId() const { return m_gl_id; }
	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};