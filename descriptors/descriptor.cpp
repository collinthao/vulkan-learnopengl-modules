#include "./descriptor.h"

Descriptor::Descriptor(){};

Descriptor::Descriptor(VkDescriptorSetLayout layout, VkDescriptorPool pool)
:layout(layout), pool(pool) {}
