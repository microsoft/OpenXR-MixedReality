////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
//
// This shader is based on the vertex shader at https://github.com/KhronosGroup/glTF-WebGL-PBR
// with modifications for HLSL and stereoscopic rendering.
//
// The MIT License
// 
// Copyright(c) 2016 - 2017 Mohamad Moneimne and Contributors
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "PbrShared.hlsl"

StructuredBuffer<float4x4> Transforms : register(t0);

cbuffer ModelConstantBuffer : register(b1)
{
    float4x4 ModelToWorld  : packoffset(c0);

};

struct VSInputPbr
{
    float4      Position            : POSITION;
    float3      Normal              : NORMAL;
    float4      Tangent             : TANGENT;
    float4      Color0              : COLOR0;
    float2      TexCoord0           : TEXCOORD0;
    min16uint   ModelTransformIndex : TRANSFORMINDEX;
};

#define VSOutputPbr PSInputPbr
VSOutputPbr main(VSInputPbr input)
{
    VSOutputPbr output;

    const float4x4 modelTransform = mul(Transforms[input.ModelTransformIndex], ModelToWorld);
    const float4 transformedPosWorld = mul(input.Position, modelTransform);
    output.PositionProj = mul(transformedPosWorld, ViewProjection);
    output.PositionWorld = transformedPosWorld.xyz / transformedPosWorld.w;

    const float3 normalW = normalize(mul(float4(input.Normal, 0.0), modelTransform).xyz);
    const float3 tangentW = normalize(mul(float4(input.Tangent.xyz, 0.0), modelTransform).xyz);
    const float3 bitangentW = cross(normalW, tangentW) * input.Tangent.w;
    output.TBN = float3x3(tangentW, bitangentW, normalW);

    output.TexCoord0 = input.TexCoord0;
    output.Color0 = input.Color0;

    return output;
}
