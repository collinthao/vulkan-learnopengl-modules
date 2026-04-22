#include "./descriptor.h"

Descriptor::Descriptor(){};

Descriptor::Descriptor(VkDescriptorSetLayout layout)
:layout(layout){}
