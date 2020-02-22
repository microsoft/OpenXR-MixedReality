////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "Shared.hlsl"

struct PSInputPbr
{
    float4 PositionProj : SV_POSITION;
    float3 PositionWorld: POSITION1;
    float3x3 TBN        : TANGENT;
    float2 TexCoord0    : TEXCOORD0;
    float4 Color0       : COLOR0;
};