#pragma once

class DeferredGraphics;
class DeferredBuffer;
class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class InputLayout;
class MapUtils;
class SamplerState;
template<typename T> class Buffer;
struct VertexDefer;


class TestRender
{
public:
	TestRender(MapUtils* m_info);
	~TestRender() = default;
	void setPipe();
	void render(ComPtr<ID3D11ShaderResourceView> srv);

private:
	TestRender() = delete;
	TestRender(TestRender const&) = delete;
	TestRender& operator=(TestRender const&) = delete;

private:
	MapUtils* m_info;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;

private:
	shared_ptr<Buffer<VertexDefer>> v_buff;
	shared_ptr<Buffer<uint32>> i_buff;
};

