#include "DrawTextureThread.h"
#include "WindowTemplate.h"
#include "Utils.h"
#include "DebugUtils.h"
#include "Mesh.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace render {
    std::vector<Vertex2DColorTexture> gQuadVertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},     // 左上
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},      // 右上
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},       // 右下
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}       // 左下
    };

    const std::vector<uint16_t> gQuadIndices = {
        0, 1, 2, 2, 3, 0
    };

    DrawTextureThread::DrawTextureThread(window::WindowTemplate& w) : RenderBase(w)
    {
        mRenderPassTest = new RenderPassTest();
        mPipelineDrawTexture = new PipelineDrawTexture();
    }

    DrawTextureThread::~DrawTextureThread() {
        delete mPipelineDrawTexture;
        delete mRenderPassTest;
    }

    void DrawTextureThread::SetFramebufferResized() {
        std::unique_lock<std::mutex> lock(mFramebufferResizeMutex);
        mFramebufferResized = true;
    }

    void DrawTextureThread::OnThreadInit() {
        RenderBase::Init();

        // create render objects
        CreateSyncObjects();
        mRenderPassTest->Init(RenderBase::mGraphicsDevice, RenderBase::mSwapchain);
        mPipelineDrawTexture->Init(RenderBase::mGraphicsDevice, RenderBase::mWindow.GetWindowExtent(),
            { mRenderPassTest->Get(), 0 });
        mCommandBuffer = RenderBase::mGraphicsDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        CreateDepthResources();
        CreateFramebuffers();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateTexture();
        CreateTextureSampler();
        CreateDescriptorPool();
        CreateDescriptorSets();
    }

    void DrawTextureThread::OnThreadLoop() {
        // 等待前一帧结束(等待队列中的命令执行完)
        vkWaitForFences(RenderBase::GetDevice(), 1, &mInFlightFence, VK_TRUE, UINT64_MAX);
    
        // 获取图像
        uint32_t imageIndex;
        if (!mSwapchain->AcquireImage(mImageAvailableSemaphore, imageIndex)) {
            Resize();
            return;     // 获取不到图像直接return，不渲染，不上锁
        }

        // 开始画，上锁
        vkResetFences(RenderBase::GetDevice(), 1, &mInFlightFence);

        // 记录命令
        vkResetCommandBuffer(mCommandBuffer, 0);
        RecordCommandBuffer(mCommandBuffer, imageIndex);

        // 提交命令
        std::vector<VkSemaphore> imageAvailiableSemaphore = { mImageAvailableSemaphore };
        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        std::vector<VkCommandBuffer> commandBuffers = { mCommandBuffer };
        std::vector<VkSemaphore> renderFinishedSemaphore = { mRenderFinishedSemaphore };
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = imageAvailiableSemaphore.size();
        submitInfo.pWaitSemaphores = imageAvailiableSemaphore.data();    // 指定要等待的信号量
        submitInfo.pWaitDstStageMask = waitStages.data();      // 指定等待的阶段（颜色附件可写入）
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();
        submitInfo.signalSemaphoreCount = renderFinishedSemaphore.size();
        submitInfo.pSignalSemaphores = renderFinishedSemaphore.data();    // 指定命令执行完触发mRenderFinishedSemaphore，意思是等我画完再返回交换链
        // 把命令提交到图形队列中，第三个参数指定命令执行完毕后触发mInFlightFence，告诉CPU当前帧画完可以画下一帧了（解锁）
        if (vkQueueSubmit(RenderBase::mGraphicsDevice->GetGraphicsQueue(), 1, &submitInfo, mInFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // 提交显示
        if (!mSwapchain->QueuePresent(imageIndex, renderFinishedSemaphore)) {
            Resize();
        }

        // 主动重建交换链
        {
            std::unique_lock<std::mutex> lock(mFramebufferResizeMutex);
            if (mFramebufferResized) {
                mFramebufferResized = false;
                Resize();
            }
        }
        
    }

    void DrawTextureThread::OnThreadDestroy() {
        vkDeviceWaitIdle(RenderBase::GetDevice());

        // destroy render objects
        CleanUpDescriptorPool();
        CleanUpTextureSampler();
        CleanUpTexture();
        CleanUpIndexBuffer();
        CleanUpVertexBuffer();
        CleanUpFramebuffers();
        CleanUpDepthResources();
        RenderBase::mGraphicsDevice->FreeCommandBuffer(mCommandBuffer);
        mPipelineDrawTexture->CleanUp();
        mRenderPassTest->CleanUp();
        CleanUpSyncObjects();

        // destroy basic objects
        RenderBase::CleanUp();
    }

    void DrawTextureThread::RecordCommandBuffer(VkCommandBuffer commandBuffer, int32_t imageIndex) {
        // 开始写入
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("fiaile to begin recording command buffer!");
        }

        // 启动Pass
        std::array<VkClearValue, 2> clearValues = {
            consts::CLEAR_COLOR_NAVY_FLT,
            consts::CLEAR_DEPTH_ONE_STENCIL_ZERO
        };
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPassTest->Get();
        renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = mSwapchain->GetExtent();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 绑定Pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineDrawTexture->GetPipeline());
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)mSwapchain->GetExtent().width;
        viewport.height = (float)mSwapchain->GetExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = mSwapchain->GetExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // 绑定顶点缓冲
        std::vector<VkBuffer> vertexBuffers = { mVertexBuffer };
        std::vector<VkDeviceSize> offsets = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());

        // 绑定索引缓冲
        vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

        // 绑定DescriptorSet
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            mPipelineDrawTexture->GetPipelineLayout(),
            0, 1, &mDescriptorSet,
            0, nullptr);

        //画图
        vkCmdDrawIndexed(commandBuffer, gQuadIndices.size(), 1, 0, 0, 0);
        //vkCmdDraw(commandBuffer, gVertices.size(), 1, 0, 0);

        // 结束Pass
        vkCmdEndRenderPass(commandBuffer);

        // 写入完成
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void DrawTextureThread::Resize() {
        vkDeviceWaitIdle(RenderBase::GetDevice());

        CleanUpFramebuffers();
        CleanUpDepthResources();

        mSwapchain->Recreate(mWindow.GetWindowExtent());
        CreateDepthResources();
        CreateFramebuffers();
    }

    void DrawTextureThread::CreateDepthResources() {
        // image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = mSwapchain->GetExtent().width;
        imageInfo.extent.height = mSwapchain->GetExtent().height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = mRenderPassTest->GetDepthFormat();
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;		// 可选TILING_LINEAR:行优先 TILLING_OPTIMAL:一种容易访问的结构
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;	// 可选标志，例如3D纹理中避免体素中的空气值
        mGraphicsDevice->CreateImage(&imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);

        // imageView
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = mDepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = mRenderPassTest->GetDepthFormat();
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(RenderBase::GetDevice(), &viewInfo, nullptr, &mDepthImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        //// 转换格式
        //TransitionImageLayout(mDepthImage, depthFormat,
        //	VK_IMAGE_LAYOUT_UNDEFINED,
        //	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
    }

    void DrawTextureThread::CleanUpDepthResources() {
        // 销毁深度缓冲
        vkDestroyImageView(RenderBase::GetDevice(), mDepthImageView, nullptr);
        vkDestroyImage(RenderBase::GetDevice(), mDepthImage, nullptr);
        vkFreeMemory(RenderBase::GetDevice(), mDepthImageMemory, nullptr);
    }

    void DrawTextureThread::CreateFramebuffers() {
        // 对每一个imageView创建帧缓冲
        std::vector<VkImageView> swapChainImageViews = mSwapchain->GetImageViews();
        mSwapchainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                mDepthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = mRenderPassTest->Get();
            framebufferInfo.attachmentCount = attachments.size();	// framebuffer的附件个数
            framebufferInfo.pAttachments = attachments.data();								// framebuffer的附件
            framebufferInfo.width = mSwapchain->GetExtent().width;
            framebufferInfo.height = mSwapchain->GetExtent().height;
            framebufferInfo.layers = 1;		// single pass

            if (vkCreateFramebuffer(RenderBase::GetDevice(), &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create frambuffer!");
            }
        }
    }

    void DrawTextureThread::CleanUpFramebuffers() {
        // 销毁frame buffer
        for (auto framebuffer : mSwapchainFramebuffers) {
            vkDestroyFramebuffer(RenderBase::GetDevice(), framebuffer, nullptr);
        }
    }

    void DrawTextureThread::CreateSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;		//（初始化解锁）

        if (vkCreateSemaphore(RenderBase::GetDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(RenderBase::GetDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(RenderBase::GetDevice(), &fenceInfo, nullptr, &mInFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    void DrawTextureThread::CleanUpSyncObjects() {
        vkDestroySemaphore(RenderBase::GetDevice(), mImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(RenderBase::GetDevice(), mRenderFinishedSemaphore, nullptr);
        vkDestroyFence(RenderBase::GetDevice(), mInFlightFence, nullptr);
    }

    void DrawTextureThread::CreateVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(gQuadVertices[0]) * gQuadVertices.size();

        // 创建临时缓冲
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        mGraphicsDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,		// 用途：transfer src
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // 数据拷贝到临时缓冲
        void* data;
        vkMapMemory(RenderBase::GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, gQuadVertices.data(), (size_t)bufferSize);
        vkUnmapMemory(RenderBase::GetDevice(), stagingBufferMemory);

        // 创建 mVertexBuffer
        mGraphicsDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,	// 用途：transfer src + 顶点缓冲
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mVertexBuffer, mVertexBufferMemory);

        // 复制 stagingBuffer -> mVertexBuffer
        mGraphicsDevice->CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

        // 清理临时缓冲
        vkDestroyBuffer(RenderBase::GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(RenderBase::GetDevice(), stagingBufferMemory, nullptr);
    }

    void DrawTextureThread::CleanUpVertexBuffer() {
        // 销毁顶点缓冲区及显存
        vkDestroyBuffer(RenderBase::GetDevice(), mVertexBuffer, nullptr);
        vkFreeMemory(RenderBase::GetDevice(), mVertexBufferMemory, nullptr);
    }

    void DrawTextureThread::CreateIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(gQuadIndices[0]) * gQuadIndices.size();

        // 创建临时缓冲
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        mGraphicsDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // 数据拷贝到临时缓冲
        void* data;
        vkMapMemory(RenderBase::GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, gQuadIndices.data(), (size_t)bufferSize);
        vkUnmapMemory(RenderBase::GetDevice(), stagingBufferMemory);

        // 创建索引缓冲
        mGraphicsDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mIndexBuffer, mIndexBufferMemory);

        // 复制 stagingBuffer -> mIndexBuffer
        mGraphicsDevice->CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

        // 清理临时缓冲
        vkDestroyBuffer(RenderBase::GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(RenderBase::GetDevice(), stagingBufferMemory, nullptr);
    }

    void DrawTextureThread::CleanUpIndexBuffer() {
        vkDestroyBuffer(RenderBase::GetDevice(), mIndexBuffer, nullptr);
        vkFreeMemory(RenderBase::GetDevice(), mIndexBufferMemory, nullptr);
    }

    void DrawTextureThread::CreateTexture() {
        // 读取图片
        int texWidth, texHeight, texChannels;
        std::string texturePath = setting::dirTexture + std::string("test_texure.jpg");
        stbi_set_flip_vertically_on_load(true);
        stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        if (!pixels) {
            throw std::runtime_error("failed to load test_texure.jpg!");
        }

        // 创建临时缓冲
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        mGraphicsDevice->CreateBuffer(imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // 图像数据拷贝到临时缓冲，并释放pixels
        void* data;
        vkMapMemory(RenderBase::GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(RenderBase::GetDevice(), stagingBufferMemory);
        stbi_image_free(pixels);

        // 创建纹理图像
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional
        mGraphicsDevice->CreateImage(&imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mTestTextureImage, mTestTextureImageMemory);

        // 转换image格式，undefined -> transferDst
        ImageMemoryBarrierInfo imageBarrierInfo{};
        imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        imageBarrierInfo.srcAccessMask = 0;
        imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        imageBarrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mGraphicsDevice->TransitionImageLayout(mTestTextureImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

        // 拷贝数据，从buffer到image
        mGraphicsDevice->CopyBufferToImage(stagingBuffer, mTestTextureImage, texWidth, texHeight);

        // 转换image格式，transferDst -> shaderReadOnly
        imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        imageBarrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        mGraphicsDevice->TransitionImageLayout(mTestTextureImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

        // 清理临时缓冲
        vkDestroyBuffer(RenderBase::GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(RenderBase::GetDevice(), stagingBufferMemory, nullptr);

        // 创建imageView
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = mTestTextureImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(RenderBase::GetDevice(), &viewInfo, nullptr, &mTestTextureImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void DrawTextureThread::CleanUpTexture() {
        vkDestroyImageView(RenderBase::GetDevice(), mTestTextureImageView, nullptr);
        vkDestroyImage(RenderBase::GetDevice(), mTestTextureImage, nullptr);
        vkFreeMemory(RenderBase::GetDevice(), mTestTextureImageMemory, nullptr);
    }

    void DrawTextureThread::CreateTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        // 关闭各向异性滤波
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        // border color
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        // true:[0,texWidth][0,texHeight]  false:[0,1][0,1]
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        // 比较：不开启， 主要用在shadow-map的PCF中
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // mipmap
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(1);

        if (vkCreateSampler(RenderBase::GetDevice(), &samplerInfo, nullptr, &mTexureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texure sampler!");
        }
    }

    void DrawTextureThread::CleanUpTextureSampler() {
        vkDestroySampler(RenderBase::GetDevice(), mTexureSampler, nullptr);
    }

    void DrawTextureThread::CreateDescriptorPool() {
        std::vector<VkDescriptorPoolSize> poolSizes = mPipelineDrawTexture->GetDescriptorSize();   // 池中各种类型的Descriptor个数

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;   // 池中最大能申请descriptorSet的个数
        if (vkCreateDescriptorPool(RenderBase::GetDevice(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void DrawTextureThread::CleanUpDescriptorPool() {
        vkDestroyDescriptorPool(RenderBase::GetDevice(), mDescriptorPool, nullptr);
    }

    void DrawTextureThread::CreateDescriptorSets() {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = mPipelineDrawTexture->GetDescriptorSetLayouts();

        //// 从池中申请descriptor set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;		// 从这个池中申请
        allocInfo.descriptorSetCount = descriptorSetLayouts.size();
        allocInfo.pSetLayouts = descriptorSetLayouts.data();
        if (vkAllocateDescriptorSets(RenderBase::GetDevice(), &allocInfo, &mDescriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        // 向descriptor set写入信息，个人理解目的是绑定buffer
        std::vector<VkDescriptorImageInfo> imageInfos(1);
        imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[0].imageView = mTestTextureImageView;
        imageInfos[0].sampler = mTexureSampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites(1);
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mDescriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].dstArrayElement = 0;		// descriptors can be arrays
        descriptorWrites[0].descriptorCount = 1;	    // 想要更新多少个元素（从索引dstArrayElement开始）
        descriptorWrites[0].pBufferInfo = nullptr;			            //  ->
        descriptorWrites[0].pImageInfo = imageInfos.data();				//  -> 三选一
        descriptorWrites[0].pTexelBufferView = nullptr;				    //  ->
        vkUpdateDescriptorSets(RenderBase::GetDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}