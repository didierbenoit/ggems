struct VertexOutput {
  float4 position : SV_Position;
  float3 colour : COLOR0;
};

[shader("vertex")]
VertexOutput VertexMain(uint vertex_id : SV_VertexID) {
  float2 positions[6] = {
    float2(-0.85, 0.00),
    float2(0.85, 0.00),

    float2(0.00, -0.85),
    float2(0.00, 0.85),

    float2(0.00, 0.00),
    float2(-0.45, 0.45)
  };

  float3 colours[6] = {
    float3(0.95, 0.20, 0.16),
    float3(0.95, 0.20, 0.16),
    
    float3(0.20, 0.85, 0.35),
    float3(0.20, 0.85, 0.35),
    
    float3(0.25, 0.45, 1.00),
    float3(0.25, 0.45, 1.00)
  };

  VertexOutput output;
  output.position = float4(positions[vertex_id], 0.0, 1.0);
  output.colour = colours[vertex_id];
  return output;
}

[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
  return float4(input.colour, 1.0);
}
