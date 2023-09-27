#include "RenderPassTest.h"

#include <stdexcept>

namespace render {
    RenderPassTest::RenderPassTest() {}

    RenderPassTest::~RenderPassTest() {}

    void RenderPassTest::FillAttachmentDescriptions(std::vector<VkAttachmentDescription>& attachments) {
		attachments.clear();
		attachments.resize(2);

		// 查找合适的深度缓冲格式
		mDepthFormat = GetGraphicsDevice()->FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		// 颜色附件
		attachments[0].format = GetSwapchain()->GetFormat();		// 格式
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;				// 多重采样
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// 渲染前清屏
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;				// 渲染后储存
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		// 模板操作此处不用
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板操作此处不用
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// 进来的格式无所谓
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 渲染后被设为该格式(作为颜色附件)
		// 深度附件
		attachments[1].format = mDepthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		// 渲染前清屏
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// 渲染后不关心
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板操作此处不用
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板操作此处不用
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// 进来的格式无所谓
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		// 渲染后被设为该格式
    }

    void RenderPassTest::CreateRenderPass(VkRenderPass& renderPass) {
		// subpass
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;	// attachment的下标，第0个
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;	// attachment的下标，第1个
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkSubpassDescription> subpasses(1);
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &colorAttachmentRef;		// 颜色缓冲
		subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;	// 深度缓冲
		subpasses[0].pInputAttachments = nullptr;
		subpasses[0].pResolveAttachments = nullptr;
		subpasses[0].pPreserveAttachments = nullptr;

		std::vector<VkSubpassDependency> dependencys(1);
		dependencys[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencys[0].dstSubpass = 0;
		dependencys[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencys[0].srcAccessMask = 0;
		dependencys[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencys[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = GetAttachments().size();
		renderPassInfo.pAttachments = GetAttachments().data();
		renderPassInfo.subpassCount = subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = dependencys.size();
		renderPassInfo.pDependencies = dependencys.data();

		if (vkCreateRenderPass(GetGraphicsDevice()->GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
    }
}