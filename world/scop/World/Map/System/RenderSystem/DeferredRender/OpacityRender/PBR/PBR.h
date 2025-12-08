#pragma once

#include "WorldUtils.h"

class DeferredGraphics;
class DeferredBuffer;
class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class SamplerState;
template<typename T> class Buffer;
class ConstantBuffer;
class Texture;

class PBR
{
public:
	PBR(UINT width, UINT height);
	~PBR() = default;
	void setRTV();
	void render(
		vec3 const& light_pos, 
		vec3 const& cam_pos,
		ComPtr<ID3D11ShaderResourceView> color_srv,
		D3D11_VIEWPORT* view_port = nullptr
	);
	ComPtr<ID3D11ShaderResourceView> getAmbientLight();
	ComPtr<ID3D11ShaderResourceView> getDirectLight();

private:
	PBR() = delete;
	PBR(PBR&) = delete;
	PBR& operator=(PBR&) = delete;

private:
	void setPipe();
	void setCBuffer(vec3 const& light_pos, vec3 const& cam_pos);

private:
	shared_ptr<DeferredBuffer> d_buffer;

private:
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state_linear;
	shared_ptr<SamplerState> sampler_state_clamp;

private:
	shared_ptr<Buffer<VertexDefer>> vbuffer;
	shared_ptr<Buffer<uint32>> ibuffer;
	shared_ptr<ConstantBuffer> cbuffer;

private:
	shared_ptr<Texture> irradiance_tex;
	shared_ptr<Texture> specular_tex;
	shared_ptr<Texture> brdf_tex;
};

