#include "PipelineTest.h"
#include "Utils.h"
#include "Mesh.h"

#include <stdexcept>

namespace render {
    PipelineTest::PipelineTest() {

    }

    PipelineTest::~PipelineTest() {

    }

	std::vector<VkDescriptorPoolSize> PipelineTest::GetDescriptorSize() {	
		std::vector<VkDescriptorPoolSize> descriptorSize(1);
		descriptorSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSize[0].descriptorCount = 1;

		return descriptorSize;
	}

    void PipelineTest::CreateShaderModules(ShaderModules& shaderModules) {
        shaderModules = {};

        // 读取shader
		auto vertShaderCode = utils::ReadFile(setting::dirSpvFiles + std::string("DrawTriangleTestVert.spv"));
		auto fragShaderCode = utils::ReadFile(setting::dirSpvFiles + std::string("DrawTriangleTestFrag.spv"));

        // 创建
        shaderModules.vertexShader = CreateShaderModule(PipelineTemplate::GetDevice(), vertShaderCode);
        shaderModules.fragmentShader = CreateShaderModule(PipelineTemplate::GetDevice(), fragShaderCode);
        return;
    }

	void PipelineTest::CreateDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) {
		descriptorSetLayouts = std::vector<VkDescriptorSetLayout>(1);

		// descriptor bindings
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(1);
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr; // Optional

		// descriptor layout
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = layoutBindings.size();
		layoutInfo.pBindings = layoutBindings.data();
		if (vkCreateDescriptorSetLayout(PipelineTemplate::GetDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[0]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

    void PipelineTest::ConfigPipelineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& configInfo) {
		// shader stage
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shaderModules.vertexShader;	// 代码
		vertShaderStageInfo.pName = consts::MAIN_FUNC_NAME.c_str();		// 入口点
		vertShaderStageInfo.pSpecializationInfo = nullptr;	// 常量值，此处没有

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shaderModules.fragmentShader;
		fragShaderStageInfo.pName = consts::MAIN_FUNC_NAME.c_str();
		fragShaderStageInfo.pSpecializationInfo = nullptr;	// 常量值，此处没有

		// view port
		configInfo.viewport.x = 0.0f;
		configInfo.viewport.y = 0.0f;
		configInfo.viewport.width = GetWindowExtent().width;
		configInfo.viewport.height = GetWindowExtent().height;
		configInfo.viewport.minDepth = 0.0f;
		configInfo.viewport.maxDepth = 1.0f;

		configInfo.scissor.offset = { 0, 0 };
		configInfo.scissor.extent = GetWindowExtent();

		configInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportState.viewportCount = 1;
		configInfo.viewportState.pViewports = &configInfo.viewport;
		configInfo.viewportState.scissorCount = 1;
		configInfo.viewportState.pScissors = &configInfo.scissor;

		// shader
		configInfo.shaderStageInfo = { vertShaderStageInfo, fragShaderStageInfo };

		// 顶点输入
		configInfo.vertexBindingDescriptions = { Vertex2DColor::GetBindingDescription() };
		configInfo.vertexAttributeDescriptions = Vertex2DColor::getAttributeDescriptions();

		configInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configInfo.vertexInputInfo.vertexBindingDescriptionCount = configInfo.vertexBindingDescriptions.size();
		configInfo.vertexInputInfo.pVertexBindingDescriptions = configInfo.vertexBindingDescriptions.data();
		configInfo.vertexInputInfo.vertexAttributeDescriptionCount = configInfo.vertexAttributeDescriptions.size();
		configInfo.vertexInputInfo.pVertexAttributeDescriptions = configInfo.vertexAttributeDescriptions.data();

		// 图元
		configInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

		// 光栅化
		configInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizer.depthClampEnable = VK_FALSE;
		configInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;	// 开启后基本不会项帧缓冲上输出任何内容
		configInfo.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;	// FILL/LINE/POINT
		configInfo.rasterizer.lineWidth = 1.0f;
		configInfo.rasterizer.cullMode = VK_CULL_MODE_NONE;	// 面剔除
		configInfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// 指定正面顶点顺序为顺时针
		configInfo.rasterizer.depthBiasEnable = VK_FALSE;			// 深度偏置
		configInfo.rasterizer.depthBiasConstantFactor = 0.0f;
		configInfo.rasterizer.depthBiasClamp = 0.0f;
		configInfo.rasterizer.depthBiasSlopeFactor = 0.0f;

		// 多重采样
		configInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampling.sampleShadingEnable = VK_FALSE;
		configInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampling.minSampleShading = 1.0f;
		configInfo.multisampling.pSampleMask = nullptr;
		configInfo.multisampling.alphaToCoverageEnable = VK_FALSE;
		configInfo.multisampling.alphaToOneEnable = VK_FALSE;

		// 深度、模板测试
		configInfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencil.depthTestEnable = VK_TRUE;
		configInfo.depthStencil.depthWriteEnable = VK_TRUE;
		configInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencil.minDepthBounds = 0.0f; // Optional
		configInfo.depthStencil.maxDepthBounds = 0.0f; // Optional
		configInfo.depthStencil.stencilTestEnable = VK_FALSE;
		configInfo.depthStencil.front = {}; // Optional
		configInfo.depthStencil.back = {}; // Optional

		// 颜色混合
		configInfo.colorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>(1);
		configInfo.colorBlendAttachments[0].colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachments[0].blendEnable = VK_FALSE;
		configInfo.colorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachments[0].alphaBlendOp = VK_BLEND_OP_ADD;

		configInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlending.logicOpEnable = VK_FALSE;
		configInfo.colorBlending.logicOp = VK_LOGIC_OP_COPY;
		configInfo.colorBlending.attachmentCount = configInfo.colorBlendAttachments.size();
		configInfo.colorBlending.pAttachments = configInfo.colorBlendAttachments.data();
		configInfo.colorBlending.blendConstants[0] = 0.0f;
		configInfo.colorBlending.blendConstants[1] = 0.0f;
		configInfo.colorBlending.blendConstants[2] = 0.0f;
		configInfo.colorBlending.blendConstants[3] = 0.0f;

		// 可动态改变的状态
		configInfo.dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};
		configInfo.dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStateCreateInfo.dynamicStateCount = configInfo.dynamicStates.size();
		configInfo.dynamicStateCreateInfo.pDynamicStates = configInfo.dynamicStates.data();
		return;
    }
}