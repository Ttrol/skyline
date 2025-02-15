// SPDX-License-Identifier: MIT OR MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <soc/gm20b/gpfifo.h>
#include <services/common/fence.h>
#include "services/nvdrv/devices/nvdevice.h"

namespace skyline::service::nvdrv::device::nvhost {
    /**
     * @brief nvhost::GpuChannel is used to create and submit commands to channels which are effectively GPU processes
     * @url https://switchbrew.org/wiki/NV_services#Channels
     */
    class GpuChannel : public NvDevice {
      private:
        u32 channelSyncpoint{};
        u32 channelUserData{};
        std::shared_ptr<type::KEvent> smExceptionBreakpointIntReportEvent;
        std::shared_ptr<type::KEvent> smExceptionBreakpointPauseReportEvent;
        std::shared_ptr<type::KEvent> errorNotifierEvent;

      public:
        /**
         * @brief A bitfield of the flags that can be supplied for a specific GPFIFO submission
         */
        union SubmitGpfifoFlags {
            struct __attribute__((__packed__)) {
                bool fenceWait : 1;
                bool fenceIncrement : 1;
                bool hwFormat : 1;
                u8 _pad0_ : 1;
                bool suppressWfi : 1;
                u8 _pad1_ : 3;
                bool incrementWithValue : 1;
            };
            u32 raw;
        };

        GpuChannel(const DeviceState &state, Core &core, const SessionContext &ctx);

        /**
         * @brief Sets the nvmap handle id to be used for channel submits (does nothing for GPU channels)
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_SET_NVMAP_FD
         */
        PosixResult SetNvmapFd(In<core::NvMap::Handle::Id> id);

        /**
         * @brief Sets the timeout for channel submits
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_SET_TIMEOUT
         */
        PosixResult SetTimeout(In<u32> timeout);

        /**
         * @brief Submits GPFIFO entries for this channel
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_SUBMIT_GPFIFO
         */
        PosixResult SubmitGpfifo(In<u64> userAddress, In<u32> numEntries, InOut<SubmitGpfifoFlags> flags, InOut<Fence> fence, span<soc::gm20b::GpEntry> gpEntries);

        /**
         * @brief Ioctl2 variant of SubmitGpfifo
         */
        PosixResult SubmitGpfifo2(span<u8> inlineBuffer, In<u64> userAddress, In<u32> numEntries, InOut<SubmitGpfifoFlags> flags, InOut<Fence> fence);

        /**
         * @brief Allocates a graphic context object
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_ALLOC_OBJ_CTX
         */
        PosixResult AllocObjCtx(In<u32> classId, In<u32> flags, Out<u64> objId);

        /**
         * @brief Binds a zcull context to the channel
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_ZCULL_BIND
         */
        PosixResult ZcullBind(In<u64> gpuVa, In<u32> mode);

        /**
         * @brief Initializes the error notifier for this channel
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_SET_ERROR_NOTIFIER
         */
        PosixResult SetErrorNotifier(In<u64> offset, In<u64> size, In<u32> mem);

        /**
         * @brief Sets the priority of the channel
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_SET_PRIORITY
         */
        PosixResult SetPriority(In<u32> priority);

        /**
         * @brief Allocates a GPFIFO entry
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_ALLOC_GPFIFO_EX2
         */
        PosixResult AllocGpfifoEx2(In<u32> numEntries, In<u32> numJobs, In<u32> flags, Out<Fence> fence);

        /**
         * @brief Sets the timeslice of the channel
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_SET_TIMESLICE)
         */
        PosixResult SetTimeslice(In<u32> timeslice);

        /**
         * @brief Sets the user specific data
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_SET_USER_DATA
         */
        PosixResult SetUserData(In<u64> userData);

        /**
         * @brief Sets the user specific data
         * @url https://switchbrew.org/wiki/NV_services#NVGPU_IOCTL_CHANNEL_GET_USER_DATA
         */
        PosixResult GetUserData(Out<u64> userData);

        std::shared_ptr<type::KEvent> QueryEvent(u32 eventId) override;

        PosixResult Ioctl(IoctlDescriptor cmd, span<u8> buffer) override;

        PosixResult Ioctl2(IoctlDescriptor cmd, span<u8> buffer, span<u8> inlineBuffer) override;
    };
}
