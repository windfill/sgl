/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2021, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Utils/File/Logfile.hpp>
#include "../Utils/Device.hpp"
#include "../Utils/Swapchain.hpp"
#include "../Render/GraphicsPipeline.hpp"
#include "../Render/ComputePipeline.hpp"
#include "../Render/RayTracingPipeline.hpp"
#include "../Buffers/Buffer.hpp"
#include "Data.hpp"
#include "Renderer.hpp"

namespace sgl { namespace vk {

Renderer::Renderer(Device* device) : device(device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // VK_SHADER_STAGE_ALL_GRAPHICS

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(
            device->getVkDevice(), &descriptorSetLayoutInfo, nullptr,
            &matrixBufferDesciptorSetLayout) != VK_SUCCESS) {
        Logfile::get()->throwError("Error in Renderer::Renderer: Failed to create descriptor set layout!");
    }


    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = maxFrameCacheSize;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxFrameCacheSize;

    if (vkCreateDescriptorPool(
            device->getVkDevice(), &poolInfo, nullptr, &matrixBufferDescriptorPool) != VK_SUCCESS) {
        Logfile::get()->throwError("Error in Renderer::Renderer: Failed to create descriptor pool!");
    }
}

Renderer::~Renderer() {
    for (FrameCache& frameCache : frameCaches) {
        while (!frameCache.allMatrixBlockDescriptorSets.is_empty()) {
            VkDescriptorSet descriptorSet = frameCache.allMatrixBlockDescriptorSets.pop_front();
            vkFreeDescriptorSets(
                    device->getVkDevice(), matrixBufferDescriptorPool, 1, &descriptorSet);
        }
    }

    vkDestroyDescriptorSetLayout(device->getVkDevice(), matrixBufferDesciptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device->getVkDevice(), matrixBufferDescriptorPool, nullptr);
}

void Renderer::beginCommandBuffer() {
    Swapchain* swapchain = AppSettings::get()->getSwapchain();
    frameIndex = swapchain->getImageIndex();
    if (frameCaches.size() != swapchain->getNumImages()) {
        frameCaches.resize(swapchain->getNumImages());
    }
    frameCaches.at(frameIndex).freeCameraMatrixBuffers = frameCaches.at(frameIndex).allCameraMatrixBuffers;
    frameCaches.at(frameIndex).freeMatrixBlockDescriptorSets = frameCaches.at(frameIndex).allMatrixBlockDescriptorSets;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        Logfile::get()->throwError(
                "Error in Renderer::beginCommandBuffer: Could not begin recording a command buffer.");
    }

    recordingCommandBufferStarted = true;
}

VkCommandBuffer Renderer::endCommandBuffer() {
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        Logfile::get()->throwError(
                "Error in Renderer::beginCommandBuffer: Could not record a command buffer.");
    }

    return commandBuffer;
}

void Renderer::render(RasterDataPtr rasterData) {
    bool isNewPipeline = false;
    GraphicsPipelinePtr newGraphicsPipeline = rasterData->getGraphicsPipeline();
    if (graphicsPipeline != newGraphicsPipeline) {
        graphicsPipeline = newGraphicsPipeline;
        isNewPipeline = true;
    }

    const FramebufferPtr& framebuffer = graphicsPipeline->getFramebuffer();

    if (updateMatrixBlock() || recordingCommandBufferStarted) {
        vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getVkPipelineLayout(),
                7, 1, &matrixBlockDescriptorSet, 0, nullptr);
    }
    // Use: currentMatrixBlockBuffer


    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = framebuffer->getVkRenderPass();
    renderPassBeginInfo.framebuffer = framebuffer->getVkFramebuffer();
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = framebuffer->getExtent2D();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = clearDepthStencil;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    if (isNewPipeline) {
        vkCmdBindPipeline(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicsPipeline->getVkPipeline());
    }

    std::vector<VkBuffer> vertexBuffers = rasterData->getVkVertexBuffers();
    VkDeviceSize offsets[] = { 0 };
    if (rasterData->hasIndexBuffer()) {
        VkBuffer indexBuffer = rasterData->getVkIndexBuffer();
        // VK_INDEX_TYPE_UINT32
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, rasterData->getIndexType());
    }
    vkCmdBindVertexBuffers(commandBuffer, 0, uint32_t(vertexBuffers.size()), vertexBuffers.data(), offsets);


    //std::vector<VkDescriptorSet>& dataDescriptorSets = rasterData->getVkDescriptorSets();
    //vkCmdBindDescriptorSets(
    //        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getVkPipelineLayout(),
    //        0, uint32_t(dataDescriptorSets.size()), dataDescriptorSets.data(), 0, nullptr);

    if (rasterData->hasIndexBuffer()) {
        vkCmdDrawIndexed(
                commandBuffer, static_cast<uint32_t>(rasterData->getNumIndices()),
                static_cast<uint32_t>(rasterData->getNumInstances()), 0, 0, 0);
    } else {
        vkCmdDraw(
                commandBuffer, static_cast<uint32_t>(rasterData->getNumVertices()),
                static_cast<uint32_t>(rasterData->getNumInstances()), 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::setModelMatrix(const glm::mat4 &matrix) {
    matrixBlock.mMatrix = matrix;
    matrixBlockNeedsUpdate = true;
}

void Renderer::setViewMatrix(const glm::mat4 &matrix) {
    matrixBlock.vMatrix = matrix;
    matrixBlockNeedsUpdate = true;
}

void Renderer::setProjectionMatrix(const glm::mat4 &matrix) {
    matrixBlock.pMatrix = matrix;
    matrixBlockNeedsUpdate = true;
}

bool Renderer::updateMatrixBlock() {
    if (matrixBlockNeedsUpdate) {
        matrixBlock.mvpMatrix = matrixBlock.pMatrix * matrixBlock.vMatrix * matrixBlock.mMatrix;
        if (frameCaches.at(frameIndex).freeCameraMatrixBuffers.is_empty()) {
            BufferPtr buffer(new Buffer(
                    device, sizeof(MatrixBlock), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VMA_MEMORY_USAGE_CPU_TO_GPU));
            frameCaches.at(frameIndex).allCameraMatrixBuffers.push_back(buffer);
            frameCaches.at(frameIndex).freeCameraMatrixBuffers.push_back(buffer);

            VkDescriptorSet descriptorSet;
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = matrixBufferDescriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &matrixBufferDesciptorSetLayout;

            if (vkAllocateDescriptorSets(device->getVkDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
                Logfile::get()->throwError(
                        "Error in Renderer::updateMatrixBlock: Failed to allocate descriptor sets!");
            }

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = buffer->getVkBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MatrixBlock);

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(
                    device->getVkDevice(), 1, &descriptorWrite,
                    0, nullptr);

            frameCaches.at(frameIndex).allMatrixBlockDescriptorSets.push_back(descriptorSet);
            frameCaches.at(frameIndex).freeMatrixBlockDescriptorSets.push_back(descriptorSet);
        }
        currentMatrixBlockBuffer = frameCaches.at(frameIndex).freeCameraMatrixBuffers.pop_front();
        matrixBlockDescriptorSet = frameCaches.at(frameIndex).freeMatrixBlockDescriptorSets.pop_front();

        void* bufferMemory = currentMatrixBlockBuffer->mapMemory();
        memcpy(bufferMemory, &matrixBlock, sizeof(MatrixBlock));
        currentMatrixBlockBuffer->unmapMemory();

        matrixBlockNeedsUpdate = false;
        return true;
    }
    return false;
}

void Renderer::dispatch(ComputeDataPtr computeData, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    bool isNewPipeline = false;
    ComputePipelinePtr newComputePipeline = computeData->getComputePipeline();
    if (computePipeline != newComputePipeline) {
        computePipeline = newComputePipeline;
        isNewPipeline = true;
    }

    if (isNewPipeline) {
        vkCmdBindPipeline(
                commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                computePipeline->getVkPipeline());
    }

    //std::vector<VkDescriptorSet>& dataDescriptorSets = rasterData->getVkDescriptorSets();
    //vkCmdBindDescriptorSets(
    //        commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, graphicsPipeline->getVkPipelineLayout(),
    //        1, uint32_t(dataDescriptorSets.size()), dataDescriptorSets.data(), 0, nullptr);

    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void Renderer::traceRays(RayTracingDataPtr rayTracingData) {
    bool isNewPipeline = false;
    RayTracingPipelinePtr newRayTracingPipeline = rayTracingData->getRayTracingPipeline();
    if (rayTracingPipeline != newRayTracingPipeline) {
        rayTracingPipeline = newRayTracingPipeline;
        isNewPipeline = true;
    }

    const FramebufferPtr& framebuffer = graphicsPipeline->getFramebuffer();

    updateMatrixBlock();

    if (isNewPipeline) {
        vkCmdBindPipeline(
                commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                rayTracingPipeline->getVkPipeline());
    }

    //vkCmdTraceRaysKHR(
    //        commandBuffer,
    //        &raygenShaderSbtEntry, &missShaderSbtEntry, &hitShaderSbtEntry, &callableShaderSbtEntry,
    //        framebuffer->getWidth(), framebuffer->getHeight(), framebuffer->getLayers());
}

}}
