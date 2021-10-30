// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <gpu.h>
#include "command_scheduler.h"

namespace skyline::gpu {
    CommandScheduler::CommandBufferSlot::CommandBufferSlot(vk::raii::Device &device, vk::CommandBuffer commandBuffer, vk::raii::CommandPool &pool)
        : device(device),
          commandBuffer(device, commandBuffer, pool),
          fence(device, vk::FenceCreateInfo{}),
          cycle(std::make_shared<FenceCycle>(device, *fence)) {}

    bool CommandScheduler::CommandBufferSlot::AllocateIfFree(CommandScheduler::CommandBufferSlot &slot) {
        if (!slot.active.test_and_set(std::memory_order_acq_rel)) {
            if (slot.cycle->Poll()) {
                slot.commandBuffer.reset();
                slot.cycle = std::make_shared<FenceCycle>(slot.device, *slot.fence);
                return true;
            } else {
                slot.active.clear(std::memory_order_release);
            }
        }
        return false;
    }

    CommandScheduler::CommandScheduler(GPU &pGpu) : gpu(pGpu), pool(std::ref(pGpu.vkDevice), vk::CommandPoolCreateInfo{
        .flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = pGpu.vkQueueFamilyIndex,
    }) {}

    CommandScheduler::ActiveCommandBuffer CommandScheduler::AllocateCommandBuffer() {
        auto slot{std::find_if(pool->buffers.begin(), pool->buffers.end(), CommandBufferSlot::AllocateIfFree)};
        auto slotId{std::distance(pool->buffers.begin(), slot)};
        if (slot != pool->buffers.end())
            return ActiveCommandBuffer(*slot);

        vk::CommandBuffer commandBuffer;
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
            .commandPool = *pool->vkCommandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        auto result{(*gpu.vkDevice).allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer, *gpu.vkDevice.getDispatcher())};
        if (result != vk::Result::eSuccess)
            vk::throwResultException(result, __builtin_FUNCTION());
        return ActiveCommandBuffer(pool->buffers.emplace_back(gpu.vkDevice, commandBuffer, pool->vkCommandPool));
    }

    void CommandScheduler::SubmitCommandBuffer(const vk::raii::CommandBuffer &commandBuffer, vk::Fence fence) {
        std::scoped_lock lock(gpu.queueMutex);
        gpu.vkQueue.submit(vk::SubmitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers = &*commandBuffer,
        }, fence);
    }
}
