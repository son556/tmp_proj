#pragma once

#include "WorldUtils.h"

class DeferredGraphics;
class VertexShader;
class SamplerState;
class PixelShader;
class InputLayout;
class RasterizerState;
class DeferredBuffer;
class Texture;
class ConstantBuffer;
template<typename T> class Buffer;

class CubeMap
{
public:
	CubeMap(UINT width, UINT height);
	void render(CamType type, bool ccw_flag = false);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	~CubeMap() = default;

private:
	CubeMap() = delete;
	CubeMap(CubeMap const&) = delete;
	CubeMap& operator=(CubeMap const&) = delete;

private:
	void setPipe(bool ccw_flag);

private:
	UINT width;
	UINT height;
	shared_ptr<Buffer<uint32>> ibuffer;
	shared_ptr<Buffer<VertexDefer>> vbuffer;

private:
	shared_ptr<DeferredBuffer> d_buffer;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<SamplerState> sampler_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<RasterizerState> ccw_rasterizer_sate;
	shared_ptr<ConstantBuffer> constant_buffer;

private:
	shared_ptr<Texture> cube_tex;
};

