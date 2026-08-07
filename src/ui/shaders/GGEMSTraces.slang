struct ScenePushConstants {
  float4 world_to_clip_row_0;
  float4 world_to_clip_row_1;
  float4 world_to_clip_row_2;
  float4 world_to_clip_row_3;
};

struct VertexInput {
  float3 position : POSITION0;
  float4 color : COLOR0;
};

struct VertexOutput {
  float4 position : SV_Position;
  float4 color : COLOR0;
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
VertexOutput VertexMain(VertexInput input) {
  VertexOutput output;
  output.position = TransformWorldPosition(input.position);
  output.color = input.color;
  return output;
}

[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
  return input.color;
}
