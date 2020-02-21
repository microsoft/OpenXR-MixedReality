////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
//

#include "HighlightShared.hlsl"

StructuredBuffer<float4x4> Transforms : register(t0);

cbuffer ModelConstantBuffer : register(b1)
{
    float4x4 ModelToWorld  : packoffset(c0);
};

struct VSInputFlat
{
    float4      Position            : POSITION;
    float3      Normal              : NORMAL;
    float4      Tangent             : TANGENT;
    float4      Color0              : COLOR0;
    float2      TexCoord0           : TEXCOORD0;
    min16uint   ModelTransformIndex : TRANSFORMINDEX;
};

#define VSOutputFlat PSInputFlat
VSOutputFlat main(VSInputFlat input)
{
    VSOutputFlat output;

    const float4x4 modelTransform = mul(Transforms[input.ModelTransformIndex], ModelToWorld);
    const float4 transformedPosWorld = mul(input.Position, modelTransform);
    output.PositionProj = mul(transformedPosWorld, ViewProjection);
    output.PositionWorld = transformedPosWorld.xyz / transformedPosWorld.w;
    output.NormalWorld = mul(input.Normal, (float3x3)modelTransform).xyz;

    return output;
}
