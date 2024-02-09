#include "DrawTriangleThread.h"

#include "WindowTemplate.h"
#include "Utils.h"
#include "DebugUtils.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <array>

namespace render {
    DrawTriangleThread::DrawTriangleThread(window::WindowTemplate& w) : RenderBase(w)
    {
        mRenderPassTest = new RenderPassTest();
        mPipelineTest = new PipelineTest();
    }

    DrawTriangleThread::~DrawTriangleThread() {
        delete mPipelineTest;
        delete mRenderPassTest;
    }

    void DrawTriangleThread::OnThreadInit() {
        RenderBase::Init();

        // create render objects
        CreateSyncObjects();
        mRenderPassTest->Init(RenderBase::mPhysicalDevice, RenderBase::mDevice, RenderBase::mSwapchain);
        mPipelineTest->Init(RenderBase::mDevice, mWindow.GetWindowExtent(), { mRenderPassTest->Get(), 0 });
        mCommandBuffer = RenderBase::mDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        CreateDepthResources();
        CreateFramebuffers();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffer();
        CreateDescriptorPool();
        CreateDescriptorSets();
    }

    void DrawTriangleThread::OnThreadLoop() {
        // 等待前一帧结束(等待队列中的命令执行完)，然后上锁，表示开始画了
        vkWaitForFences(RenderBase::mDevice->Get(), 1, &mInFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(RenderBase::mDevice->Get(), 1, &mInFlightFence);

        // 获取图像
        uint32_t imageIndex;
        if (!mSwapchain->AcquireImage(mImageAvailableSemaphore, imageIndex)) {
            Resize();
        }

        // 更新uniform
        UpdataUniformBuffer();

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
        if (vkQueueSubmit(RenderBase::mDevice->GetGraphicsQueue(), 1, &submitInfo, mInFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // 提交显示
        if (!mSwapchain->QueuePresent(imageIndex, renderFinishedSemaphore)) {
            Resize();
        }

        // 主动重建交换链
        if (Thread::IsFbResized()) {
            Thread::ResetFbResized();
            Resize();
        }
    }

    void DrawTriangleThread::OnThreadDestroy() {
        vkDeviceWaitIdle(mDevice->Get());

        // destroy render objects
        CleanUpDescriptorPool();
        CleanUpUniformBuffer();
        CleanUpIndexBuffer();
        CleanUpVertexBuffer();
        CleanUpFramebuffers();
        CleanUpDepthResources();
        RenderBase::mDevice->FreeCommandBuffer(mCommandBuffer);
        mPipelineTest->CleanUp();
        mRenderPassTest->CleanUp();
        CleanUpSyncObjects();

        // destroy basic objects
        RenderBase::CleanUp();
    }

    void DrawTriangleThread::RecordCommandBuffer(VkCommandBuffer commandBuffer, int32_t imageIndex) {
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
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineTest->GetPipeline());
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
            mPipelineTest->GetPipelineLayout(),
            0, 1, &mDescriptorSet,
            0, nullptr);

        //画图
        vkCmdDrawIndexed(commandBuffer, mTriangleIndices.size(), 1, 0, 0, 0);
        //vkCmdDraw(commandBuffer, gVertices.size(), 1, 0, 0);

        // 结束Pass
        vkCmdEndRenderPass(commandBuffer);

        // 写入完成
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void DrawTriangleThread::UpdataUniformBuffer() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UboMvpMatrix uboMvpMatrixs{};
        uboMvpMatrixs.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        uboMvpMatrixs.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        float aspectRatio = (float)mSwapchain->GetExtent().width / (float)mSwapchain->GetExtent().height;
        uboMvpMatrixs.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
        uboMvpMatrixs.proj[1][1] *= -1;

        memcpy(mUniformBuffersMapped, &uboMvpMatrixs, sizeof(uboMvpMatrixs));
    }

    void DrawTriangleThread::Resize() {
        vkDeviceWaitIdle(mDevice->Get());

        CleanUpFramebuffers();
        CleanUpDepthResources();

        mSwapchain->Recreate(mWindow.GetWindowExtent());
        CreateDepthResources();
        CreateFramebuffers();
    }

    void DrawTriangleThread::CreateDepthResources() {
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
        RenderBase::mDevice->CreateImage(&imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);

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
        if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mDepthImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        //// 转换格式
        //TransitionImageLayout(mDepthImage, depthFormat,
        //	VK_IMAGE_LAYOUT_UNDEFINED,
        //	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
    }

    void DrawTriangleThread::CleanUpDepthResources() {
        // 销毁深度缓冲
        vkDestroyImageView(mDevice->Get(), mDepthImageView, nullptr);
        vkDestroyImage(mDevice->Get(), mDepthImage, nullptr);
        vkFreeMemory(mDevice->Get(), mDepthImageMemory, nullptr);
    }

    void DrawTriangleThread::CreateFramebuffers() {
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

            if (vkCreateFramebuffer(mDevice->Get(), &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create frambuffer!");
            }
        }
    }

    void DrawTriangleThread::CleanUpFramebuffers() {
        // 销毁frame buffer
        for (auto framebuffer : mSwapchainFramebuffers) {
            vkDestroyFramebuffer(mDevice->Get(), framebuffer, nullptr);
        }
    }

    void DrawTriangleThread::CreateSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;		//（初始化解锁）

        if (vkCreateSemaphore(mDevice->Get(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice->Get(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(mDevice->Get(), &fenceInfo, nullptr, &mInFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    void DrawTriangleThread::CleanUpSyncObjects() {
        vkDestroySemaphore(mDevice->Get(), mImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(mDevice->Get(), mRenderFinishedSemaphore, nullptr);
        vkDestroyFence(mDevice->Get(), mInFlightFence, nullptr);
    }

    void DrawTriangleThread::CreateVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(mTriangleVertices[0]) * mTriangleVertices.size();

        // 创建临时缓冲
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        RenderBase::mDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,		// 用途：transfer src
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // 数据拷贝到临时缓冲
        void* data;
        vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data,mTriangleVertices.data(), (size_t)bufferSize);
        vkUnmapMemory(mDevice->Get(), stagingBufferMemory);

        // 创建 mVertexBuffer
        RenderBase::mDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,	// 用途：transfer src + 顶点缓冲
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mVertexBuffer, mVertexBufferMemory);

        // 复制 stagingBuffer -> mVertexBuffer
        RenderBase::mDevice->CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

        // 清理临时缓冲
        vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
        vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);
    }

    void DrawTriangleThread::CleanUpVertexBuffer() {
        // 销毁顶点缓冲区及显存
        vkDestroyBuffer(mDevice->Get(), mVertexBuffer, nullptr);
        vkFreeMemory(mDevice->Get(), mVertexBufferMemory, nullptr);
    }

    void DrawTriangleThread::CreateIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(mTriangleIndices[0]) * mTriangleIndices.size();

        // 创建临时缓冲
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        RenderBase::mDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // 数据拷贝到临时缓冲
        void* data;
        vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, mTriangleIndices.data(), (size_t)bufferSize);
        vkUnmapMemory(mDevice->Get(), stagingBufferMemory);

        // 创建索引缓冲
        RenderBase::mDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mIndexBuffer, mIndexBufferMemory);

        // 复制 stagingBuffer -> mIndexBuffer
        RenderBase::mDevice->CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

        // 清理
        vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
        vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);
    }

    void DrawTriangleThread::CleanUpIndexBuffer() {
        vkDestroyBuffer(mDevice->Get(), mIndexBuffer, nullptr);
        vkFreeMemory(mDevice->Get(), mIndexBufferMemory, nullptr);
    }

    void DrawTriangleThread::CreateUniformBuffer() {
        VkDeviceSize bufferSize = sizeof(UboMvpMatrix);

        RenderBase::mDevice->CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mUniformBuffer, mUniformBuffersMemory);

        vkMapMemory(mDevice->Get(), mUniformBuffersMemory, 0, bufferSize, 0, &mUniformBuffersMapped);
    }

    void DrawTriangleThread::CleanUpUniformBuffer() {
        vkDestroyBuffer(mDevice->Get(), mUniformBuffer, nullptr);
        vkFreeMemory(mDevice->Get(), mUniformBuffersMemory, nullptr);
    }

    void DrawTriangleThread::CreateDescriptorPool() {
        std::vector<VkDescriptorPoolSize> poolSizes = mPipelineTest->GetDescriptorSize();   // 池中各种类型的Descriptor个数

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;   // 池中最大能申请descriptorSet的个数
        if (vkCreateDescriptorPool(mDevice->Get(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void DrawTriangleThread::CleanUpDescriptorPool() {
        vkDestroyDescriptorPool(mDevice->Get(), mDescriptorPool, nullptr);
    }

    void DrawTriangleThread::CreateDescriptorSets() {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = mPipelineTest->GetDescriptorSetLayouts();

        // 从池中申请descriptor set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;		// 从这个池中申请
        allocInfo.descriptorSetCount = descriptorSetLayouts.size();
        allocInfo.pSetLayouts = descriptorSetLayouts.data();
        if (vkAllocateDescriptorSets(mDevice->Get(), &allocInfo, &mDescriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        // 向descriptor set写入信息，个人理解目的是绑定buffer
        std::vector<VkDescriptorBufferInfo> bufferInfos(2);
        bufferInfos[0].buffer = mUniformBuffer;     // mvp矩阵的ubo
        bufferInfos[0].offset = 0;
        bufferInfos[0].range = sizeof(UboMvpMatrix);

        std::vector<VkWriteDescriptorSet> descriptorWrites(1);
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mDescriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].dstArrayElement = 0;		// descriptors can be arrays
        descriptorWrites[0].descriptorCount = 1;	    // 想要更新多少个元素（从索引dstArrayElement开始）
        descriptorWrites[0].pBufferInfo = &bufferInfos[0];			//  ->
        descriptorWrites[0].pImageInfo = nullptr;					//  -> 三选一
        descriptorWrites[0].pTexelBufferView = nullptr;				//  ->
        vkUpdateDescriptorSets(mDevice->Get(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}