////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "HighlightShared.hlsl"

static const float PI = 3.141592653589793;

float HighlightFromLocation(float3 sourcePosition, float animationTime, float3 position)
{
    const float rippleSpeed = 2; // m/s
    float smoothStart = animationTime * rippleSpeed - 1.25;
    float smoothEnd = animationTime * rippleSpeed;
    return 1 - smoothstep(smoothStart, smoothEnd, distance(sourcePosition, position));
}

float4 main(PSInputFlat input) : SV_TARGET
{
    const float3 highlightColor = float3(1.0f, 1.0f, 1.0f);
    const float timemax = 15;
    const float animationTime1 = min(max(0.01, AnimationTime + 0.25), 10);
    const float animationTime2 = min(max(0.01, AnimationTime - 0.5), 10);

    const float spot1 = HighlightFromLocation(HighlightPosition, animationTime1, input.PositionWorld);
    const float spot2 = HighlightFromLocation(HighlightPosition, animationTime2, input.PositionWorld);

    return float4(highlightColor * (input.NormalWorld.y * 0.5f + 0.5f) * (spot1 - spot2), 1.0f);
}
