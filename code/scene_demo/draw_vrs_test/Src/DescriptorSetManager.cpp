#include "DescriptorSetManager.h"
#include <unordered_map>

#include "Log.h"
#undef LOG_TAG
#define LOG_TAG "DescriptorSetManager"

DescriptorSetManager::DescriptorSetManager()
{

}

DescriptorSetManager::~DescriptorSetManager()
{

}

bool DescriptorSetManager::Init(const InitInfo* info)
{
    if (m_init) {
        return true;
    }
    if (m_initFailed) {
        return false;
    }
    if (info == nullptr || info->device == VK_NULL_HANDLE ||
        info->pEntries == nullptr || info->entriesSize == 0) {
        LOGE("input failed");
        m_initFailed = true;
        return false;
    }

    m_device = info->device;

    if (!CreatePool(info->pEntries, info->entriesSize)) {
        LOGE("CreatePool failed");
        m_initFailed = true;
        return false;
    }

    if (!CreateDS(info->pEntries, info->entriesSize)) {
        LOGE("CreateDS failed");
        m_initFailed = true;
        return false;
    }

    LOGI("Init success");
    m_init = true;
    return true;
}

void DescriptorSetManager::CleanUp()
{

}

bool DescriptorSetManager::CreatePool(const DSEntry* pEntries, uint32_t entriesSize)
{
    std::unordered_map<VkDescriptorType, uint32_t> dsTypeCounts = {};
    std::unordered_set<uint32_t> updateTypeSet = {};
    for (uint32_t i = 0; i < entriesSize; i++) {
        VkDescriptorType type = pEntries[i].descriptorType;
        auto it = dsTypeCounts.find(type);
        if (it == dsTypeCounts.end()) {
            dsTypeCounts[type] = 1;
        } else {
            it->second++;
        }
        updateTypeSet.emplace(pEntries[i].updateType);
    }

    std::vector<VkDescriptorPoolSize> poolSizes = {};
    for (auto it = dsTypeCounts.begin(); it != dsTypeCounts.end(); it++) {
        VkDescriptorPoolSize dsSize = {
            .type = it->first,
            .descriptorCount = it->second,
        };
        poolSizes.push_back(dsSize);
    }

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = static_cast<uint32_t>(updateTypeSet.size()),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_pool) != VK_SUCCESS) {
        LOGE("CreateDescriptorPool failed");
        return false;
    }

    LOGI("CreatePool success");
    return true;
}

bool DescriptorSetManager::CreateDS(const DSEntry* pEntries, uint32_t entriesSize)
{
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindingsPerDS =
        std::vector<std::vector<VkDescriptorSetLayoutBinding>>(TYPE_MAX_COUNT);
    for (uint32_t i = 0; i < entriesSize; i++) {
        uint32_t updateType = pEntries[i].updateType;
        VkDescriptorSetLayoutBinding binding = {
            .binding = pEntries[i].binding,
            .descriptorType = pEntries[i].descriptorType,
            .descriptorCount = 1,
            .stageFlags = pEntries[i].stage,
            .pImmutableSamplers = nullptr,
        };
        bindingsPerDS[updateType].emplace_back(binding);
    }

    for (uint32_t i = 0; i < TYPE_MAX_COUNT; i++) {
        std::vector<VkDescriptorSetLayoutBinding>& bindings = bindingsPerDS[i];
        if (bindings.empty()) {
            continue;
        }
        VkDescriptorSetLayoutCreateInfo layoutInfo = vulkanInitializers::DescriptorSetLayoutCreateInfo(bindings);
        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_dsLayouts[i]) != VK_SUCCESS) {
            LOGE("failed to CreateDescriptorSetLayout!");
            return false;
        }
    }

    for (uint32_t i = 0; i < TYPE_MAX_COUNT; i++) {
        if (m_dsLayouts[i] == VK_NULL_HANDLE) {
            continue;
        }
        VkDescriptorSetAllocateInfo allocInfo = { 
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_dsLayouts[i],
        };
        if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSets[i]) != VK_SUCCESS) {
           LOGE("failed to allocate descriptor sets!");
           return false;
        }
    }

    LOGI("CreateDS success");
    return true;
}
