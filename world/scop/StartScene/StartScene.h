#pragma once

class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class Texture;
class SamplerState;
template<typename T> class Buffer;
class ConstantBuffer;
struct VertexDefer;

struct cbuffer_vs {
	Mat inv_vp;
	Mat world = Mat::Identity;
};


class StartScene
{
public:
	StartScene();
	~StartScene() = default;
	void render();
	bool checkClickStartButton();
	bool checkClickExitButton();

private:
	StartScene& operator=(StartScene const&) = delete;
	StartScene(StartScene const&) = delete;

private:
	void setPipe();
	void startGameLetterRender(ComPtr<ID3D11DeviceContext> const& context);
	void exitLetterRender(ComPtr<ID3D11DeviceContext> const& context);

private:
	shared_ptr<Buffer<VertexDefer>> v_buffer;
	shared_ptr<Buffer<uint32>> i_buffer;
	float cam_radian = 0;

private:
	ComPtr<ID3D11DepthStencilState> depth_stencil_state;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;

private:
	shared_ptr<Texture> start_button;
	shared_ptr<Texture> select_start_button;
	shared_ptr<Texture> title;
	shared_ptr<Texture> sky_box;

private:
	cbuffer_vs c_vs;
	shared_ptr<ConstantBuffer> constant_buffer_vs;
	shared_ptr<ConstantBuffer> texture_idx;

private:
	const Mat button_world = (Mat::CreateScale(vec3(0.5, 0.1, 1)) *
		Mat::CreateTranslation(vec3(0, 0.0727, 0))).Transpose();
	
	const Mat button_exit_world = (Mat::CreateScale(0.5, 0.1, 1) *
		Mat::CreateTranslation(vec3(0, -0.15, 0))).Transpose();
	
	const Mat title_world = (Mat::CreateScale(vec3(0.8, 0.65, 1)) *
		Mat::CreateTranslation(vec3(0, 0.7, 0))).Transpose();
	
	const float w = 0.065;
	const Mat letter_scale = Mat::CreateScale(0.065 * 0.5, 0.08 * 0.5, 1);

	const Mat proj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60),
		w_width / w_height,
		0.1, 1000
	);
};

