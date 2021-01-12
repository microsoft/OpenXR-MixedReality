// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace engine {

    // Information of frame timing
    struct FrameTime {
        using clock = std::chrono::high_resolution_clock;

        uint64_t FrameIndex = 0;
        const clock::time_point StartTime = clock::now();
        clock::time_point Now = StartTime;
        clock::duration Elapsed = {};
        clock::duration TotalElapsed = {};
        float ElapsedSeconds = {};
        float TotalElapsedSeconds = {};

        XrTime PredictedDisplayTime = {};
        XrDuration PredictedDisplayPeriod = {};
        bool ShouldRender = {};
        bool IsSessionFocused = {};

        void Update(const XrFrameState& frameState, XrSessionState sessionState) {
            FrameIndex++;
            PredictedDisplayTime = frameState.predictedDisplayTime;
            PredictedDisplayPeriod = frameState.predictedDisplayPeriod;
            ShouldRender = frameState.shouldRender;
            IsSessionFocused = sessionState == XR_SESSION_STATE_FOCUSED;

            const auto now = FrameTime::clock::now();
            Elapsed = now - Now;
            Now = now;
            TotalElapsed = now - StartTime;

            ElapsedSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(Elapsed).count();
            TotalElapsedSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(TotalElapsed).count();
        }
    };
} // namespace engine
