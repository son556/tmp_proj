#pragma once

class DeferredGraphics;
class DeferredBuffer;
class MapUtils;
class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class ConstantBuffer;

class CaveShadow
{
public:
	CaveShadow(MapUtils* m_info);
	~CaveShadow() = default;
	void render(CamType type);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	ComPtr<ID3D11RenderTargetView> getRTV();

private:
	CaveShadow() = delete;
	CaveShadow(CaveShadow const&) = delete;
	CaveShadow& operator=(CaveShadow const&) = delete;

private:
	void setPipe();

private:
	shared_ptr<DeferredBuffer> d_buffer;
	MapUtils* m_info;

private:
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;

private:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
};

