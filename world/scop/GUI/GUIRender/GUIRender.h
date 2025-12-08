#pragma once

class InputLayout;
class VertexShader;
class PixelShader;
class RasterizerState;
class SamplerState;
class GUI;
class DeferredBuffer;
class ConstantBuffer;

class GUIRender
{
public:
	GUIRender();
	~GUIRender() = default;
	void render(GUI* gui, bool rtv_reset);
	ComPtr<ID3D11ShaderResourceView> const& getSRV();

private:
	GUIRender(GUIRender const&) = delete;
	GUIRender& operator=(GUIRender const&) = delete;
	void setPipe(
		ComPtr<ID3D11DeviceContext> const& context,
		bool tp_flag
	);

private:
	ComPtr<ID3D11BlendState> blend_state;
	ComPtr<ID3D11DepthStencilState> depth_stencil_state;

private:
	shared_ptr<DeferredBuffer> deferred_buffer;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;
	
	shared_ptr<InputLayout> input_layout_color;
	shared_ptr<VertexShader> vertex_shader_color;
	shared_ptr<PixelShader> pixel_shader_color;

	shared_ptr<ConstantBuffer> constant_buffer;
};

