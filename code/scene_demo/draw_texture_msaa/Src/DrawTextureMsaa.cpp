#include "DrawTextureMsaa.h"

namespace framework {

void DrawTextureMsaa::Init(const RenderInitInfo& initInfo)
{

}
void DrawTextureMsaa::CleanUp()
{

}

std::vector<VkCommandBuffer>& DrawTextureMsaa::RecordCommand(const RenderInputInfo& input)
{
    std::vector<VkCommandBuffer> res = {};
    return res;
}

void DrawTextureMsaa::OnResize(VkExtent2D newExtent)
{

}
}   // namespace framework