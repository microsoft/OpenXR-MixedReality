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

// Information of frame timing
struct FrameTime {
    using clock = std::chrono::high_resolution_clock;

    uint64_t FrameIndex = 0;
    const clock::time_point StartTime = clock::now();
    clock::time_point Now = StartTime;
    clock::duration Elapsed = {};
    clock::duration TotalElapsed = {};

    XrTime PredictedDisplayTime = {};
    XrDuration PredictedDisplayPeriod = {};
    bool ShouldRender = {};

    void Update(const XrFrameState& frameState) {
        FrameIndex++;
        PredictedDisplayTime = frameState.predictedDisplayTime;
        PredictedDisplayPeriod = frameState.predictedDisplayPeriod;
        ShouldRender = frameState.shouldRender;

        const auto now = FrameTime::clock::now();
        Elapsed = now - Now;
        TotalElapsed = now - StartTime;
        Now = now;
    }
};
