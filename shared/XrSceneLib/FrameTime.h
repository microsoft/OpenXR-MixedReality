//*********************************************************
//    Copyright (c) Microsoft. All rights reserved.
//
//    Apache 2.0 License
//
//    You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//    implied. See the License for the specific language governing
//    permissions and limitations under the License.
//
//*********************************************************
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
