#pragma once

#include "WaterInit.h"
#include "WaterReflection.h"
#include "WaterRefraction.h"

class TestRender;

template<typename T> class Buffer;
struct VertexDefer;
struct RenderOption;

class Water
{
public:
	Water(MapUtils* m_info);
	~Water() = default;
	void setPipe();
	void render(ComPtr<ID3D11ShaderResourceView> color);
	void renderWaterInit(ComPtr<ID3D11ShaderResourceView> depth_srv);
	void renderWaterRefraction(ComPtr<ID3D11ShaderResourceView> color_srv);
	void renderWaterReflection();
	ComPtr<ID3D11ShaderResourceView> getSRV();
	ComPtr<ID3D11RenderTargetView> getRTV();

private:
	Water() = delete;
	Water(Water const&) = delete;
	Water& operator=(Water const&) = delete;

private:
	MapUtils* m_info;
	WaterInit water_init;
	WaterReflection water_reflection;
	WaterRefraction water_refraction;

private:
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;

private:
	shared_ptr<Buffer<VertexDefer>> v_buff;
	shared_ptr<Buffer<uint32>> i_buff;
	shared_ptr<DeferredBuffer> d_buff;
	ComPtr<ID3D11ShaderResourceView> water_normal_srv;
	ComPtr<ID3D11ShaderResourceView> water_distortion_srv;
	shared_ptr<ConstantBuffer> constant_buffer;

private:
	shared_ptr<TestRender> rt;
};

