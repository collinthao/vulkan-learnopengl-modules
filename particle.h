struct Particle
{
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 color;

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Particle, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Particle, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Particle, velocity);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Particle, color);



		return attributeDescriptions;
	}    
	
	static VkVertexInputBindingDescription getBindingDesciption()
    	{
    	VkVertexInputBindingDescription bindingDescription{};
    
    	bindingDescription.binding = 0;
    	bindingDescription.stride = sizeof(Particle);
    	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    	return bindingDescription;
    	}
};

