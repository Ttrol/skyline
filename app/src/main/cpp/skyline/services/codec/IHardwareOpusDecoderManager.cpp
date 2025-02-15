// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <services/serviceman.h>

#include "IHardwareOpusDecoderManager.h"
#include "IHardwareOpusDecoder.h"

namespace skyline::service::codec {
    static size_t CalculateBufferSize(i32 sampleRate, i32 channelCount) {
        i32 requiredSize{opus_decoder_get_size(channelCount)};
        requiredSize += MaxInputBufferSize + CalculateOutBufferSize(sampleRate, channelCount, MaxFrameSizeNormal);
        return requiredSize;
    }

    Result IHardwareOpusDecoderManager::OpenHardwareOpusDecoder(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        i32 sampleRate{request.Pop<i32>()};
        i32 channelCount{request.Pop<i32>()};
        u32 workBufferSize{request.Pop<u32>()};
        KHandle workBuffer{request.copyHandles.at(0)};

        state.logger->Debug("Creating Opus decoder: Sample rate: {}, Channel count: {}, Work buffer handle: 0x{:X} (Size: 0x{:X})", sampleRate, channelCount, workBuffer, workBufferSize);

        manager.RegisterService(std::make_shared<IHardwareOpusDecoder>(state, manager, sampleRate, channelCount, workBufferSize, workBuffer), session, response);
        return {};
    }

    Result IHardwareOpusDecoderManager::GetWorkBufferSize(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        i32 sampleRate{request.Pop<i32>()};
        i32 channelCount{request.Pop<i32>()};

        response.Push<u32>(CalculateBufferSize(sampleRate, channelCount));
        return {};
    }
}
