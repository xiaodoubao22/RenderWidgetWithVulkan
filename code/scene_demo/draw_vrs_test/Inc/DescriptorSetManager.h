#ifndef DESCRIPTOR_SET_MANAGER_H
#define DESCRIPTOR_SET_MANAGER_H

#include "FrameworkHeaders.h"

class DescriptorSetManager {
public:
    static constexpr uint32_t TYPE_FIXED = 0;
    static constexpr uint32_t TYPE_PIPELINE = 1;
    static constexpr uint32_t TYPE_DRAWCALL = 2;
    static constexpr uint32_t TYPE_MAX_COUNT = 3;

    struct DSEntry {
        uint32_t updateType = 0;
        uint32_t binding = 0;
        VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        VkShaderStageFlags stage = 0;
    };
        
    struct InitInfo {
        VkDevice device = VK_NULL_HANDLE;
        const DSEntry* pEntries = nullptr;
        const uint32_t entriesSize = 0;
    };

    DescriptorSetManager();
    ~DescriptorSetManager();

    bool Init(const InitInfo* info);
    void CleanUp();

private:
    bool CreatePool(const DSEntry* pEntries, uint32_t entriesSize);
    bool CreateDS(const DSEntry* pEntries, uint32_t entriesSize);

private:
    bool m_init = false;
    bool m_initFailed = false;

    VkDevice m_device = VK_NULL_HANDLE;

    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets = std::vector<VkDescriptorSet>(TYPE_MAX_COUNT, VK_NULL_HANDLE);
    std::vector<VkDescriptorSetLayout> m_dsLayouts = std::vector<VkDescriptorSetLayout>(TYPE_MAX_COUNT, VK_NULL_HANDLE);
};


#endif // !DESCRIPTOR_SET_MANAGER_H

