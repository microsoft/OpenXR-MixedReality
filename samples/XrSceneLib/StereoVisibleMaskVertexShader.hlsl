cbuffer VertexCBuffer : register(b0) {
    float4x4 ProjTransform;
}

float4 main(float2 pos : POSITION) : SV_Position {
    return mul(float4(pos, -1.0f, 1.0f), ProjTransform);
}