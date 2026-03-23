#include <string>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "mesh.h"
#include <vector>
#include <iostream>

class Model
{
public:
	Model();
	Model(std::string path);
	const aiScene * scene;	
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;
	bool gammaCorrection;
	uint32_t nodeIndex = 0;

	void getMeshes();

	void processNode(aiNode *node, const aiScene *scene);

	Mesh processMesh(aiMesh*mesh, const aiScene * scene);

private:
	std::vector<Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName);
};

