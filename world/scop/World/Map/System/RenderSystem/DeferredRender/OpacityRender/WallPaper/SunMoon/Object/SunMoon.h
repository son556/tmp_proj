#pragma once

class DeferredGraphics;
class DeferredBuffer;
class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class Sun;
class Moon;
class ConstantBuffer;

class SunMoon
{
public:
	SunMoon(UINT width, UINT height);
	void render(CamType type, bool ccw_flag = false);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	vec3 getLightPos();
	~SunMoon() = default;

private:
	SunMoon() = delete;
	SunMoon(SunMoon const&) = delete;
	SunMoon& operator=(SunMoon const&) = delete;

private:
	void setPipe(bool ccw_flag);

private:
	shared_ptr<Sun> sun;
	vec3 sun_pos;
	vec3 moon_pos;
private:
	shared_ptr<DeferredBuffer> d_buffer;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<RasterizerState> ccw_rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<ConstantBuffer> constant_buff;
};

