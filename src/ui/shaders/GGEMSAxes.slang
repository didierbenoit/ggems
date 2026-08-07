struct ScenePushConstants {
  float4 world_to_clip_row_0;
  float4 world_to_clip_row_1;
  float4 world_to_clip_row_2;
  float4 world_to_clip_row_3;
};

struct VertexOutput {
  float4 position : SV_Position;
  float3 color : COLOR0;
};

[[vk::push_constant]]
ConstantBuffer<ScenePushConstants> g_scene;

float4 TransformWorldPosition(float3 world_position) {
  float4 position = float4(world_position, 1.0);
  
  return float4(dot(g_scene.world_to_clip_row_0, position),
                dot(g_scene.world_to_clip_row_1, position),
                dot(g_scene.world_to_clip_row_2, position),
                dot(g_scene.world_to_clip_row_3, position));
}

[shader("vertex")]
VertexOutput VertexMain(uint vertex_id : SV_VertexID) {
  float3 positions[6] = {
      // X axis
      float3(-1.0, 0.0, 0.0),
      float3( 1.0, 0.0, 0.0),

      // Y axis
      float3(0.0, -1.0, 0.0),
      float3(0.0,  1.0, 0.0),

      // Z axis
      float3(0.0, 0.0, -1.0),
      float3(0.0, 0.0,  1.0)
  };

  float3 colors[6] = {
      // X axis
      float3(0.95, 0.20, 0.16),
      float3(0.95, 0.20, 0.16),

      // Y axis
      float3(0.20, 0.85, 0.35),
      float3(0.20, 0.85, 0.35),

      // Z axis
      float3(0.25, 0.45, 1.00),
      float3(0.25, 0.45, 1.00)
  };

  VertexOutput output;
  output.position = TransformWorldPosition(positions[vertex_id]);
  output.color = colors[vertex_id];

  return output;
}

[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
  return float4(input.color, 1.0);
}
