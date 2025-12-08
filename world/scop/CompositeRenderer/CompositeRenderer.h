#pragma once

#include "WorldUtils.h"

class InputLayout;
class VertexShader;
class PixelShader;
class RasterizerState;
class SamplerState;
template<typename T> class Buffer;


class CompositeRenderer
{
public:
	CompositeRenderer();
	~CompositeRenderer() = default;
	void render(
		ComPtr<ID3D11ShaderResourceView> const& game_srv,
		ComPtr<ID3D11ShaderResourceView> const& gui_srv
	);

private:
	void setPipe(ComPtr<ID3D11DeviceContext> const& context);

private:
	CompositeRenderer(CompositeRenderer const&) = delete;
	CompositeRenderer& operator=(CompositeRenderer const&) = delete;

private:
	shared_ptr<Buffer<VertexDefer>> v_buff;
	shared_ptr<Buffer<uint32>> i_buff;

private:
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;
	shared_ptr<RasterizerState> rasterize_state;
};

