#pragma once

class MapUtils;
class DeferredBuffer;
class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class SamplerState;
class ConstantBuffer;
template<typename T> class Buffer;
struct VertexDefer;

class WaterRefraction
{
public:
	WaterRefraction(MapUtils* m_info);
	~WaterRefraction() = default;
	void render(
		ComPtr<ID3D11ShaderResourceView> color,
		ComPtr<ID3D11DepthStencilView> dsv
	);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	void setWaterND(
		ComPtr<ID3D11ShaderResourceView> normal,
		ComPtr<ID3D11ShaderResourceView> distortion
	);

private:
	WaterRefraction() = delete;
	WaterRefraction(WaterRefraction const&) = delete;
	WaterRefraction& operator=(WaterRefraction const&) = delete;

private:
	void setPipe();

private:
	shared_ptr<Buffer<VertexDefer>> v_buff;
	shared_ptr<Buffer<uint32>> i_buff;
	shared_ptr<DeferredBuffer> d_buff;

private:
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;
	shared_ptr<ConstantBuffer> constant_buffer;

private:
	ComPtr<ID3D11DepthStencilState> ds_state;
	ComPtr<ID3D11ShaderResourceView> water_distortion_srv;
	ComPtr<ID3D11ShaderResourceView> water_normal_srv;
};

