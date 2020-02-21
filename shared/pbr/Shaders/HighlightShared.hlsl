////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "Shared.hlsl"

struct PSInputFlat
{
    float4 PositionProj : SV_POSITION;
    float3 PositionWorld: POSITION1;
    nointerpolation float3 NormalWorld : Normal;
};