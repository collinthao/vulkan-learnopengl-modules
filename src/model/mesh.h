#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../bindings/vertex.h"

struct Texture
{
	std::string id;
	std::string type;
	std::string path;
	unsigned int width, height;	
	aiTexel * data;
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	}

	void setupMesh()
	{
	
	}
};
