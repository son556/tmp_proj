#pragma once

#include "WorldUtils.h"

class Texture;
class LeftArm;
class RightArm;
class Head;
class Body;
class LeftLeg;
class RightLeg;
class ConstantBuffer;
template<typename T> class Buffer;
class VertexShader;
class InputLayout;
class PixelShader;
class SamplerState;
class Collision;

class Character
{
public:
	Character(Mat pos, Mat rot);
	~Character() = default;
	void render(
		CamType type,
		ComPtr<ID3D11ShaderResourceView> depth_srv,
		shared_ptr<SamplerState> sampler_tp,
		bool water_up_flag
	);
	vec3 const& getPos();
	vec3 const& getDir();
	void update(vec3 const& dir);
	void setLeftArmAnimation();
	void checkValidatePosition();

private:
	void renderHead(ComPtr<ID3D11DeviceContext>& context);
	void renderBody(ComPtr<ID3D11DeviceContext>& context);
	void renderLeftArm(ComPtr<ID3D11DeviceContext>& context);
	void renderRightArm(ComPtr<ID3D11DeviceContext>& context);
	void renderLeftLeg(ComPtr<ID3D11DeviceContext>& context);
	void renderRightLeg(ComPtr<ID3D11DeviceContext>& context);

private:
	void setPipe(ComPtr<ID3D11DeviceContext>& context);

private:
	Character() = delete;
	Character(Character const&) = delete;
	Character& operator=(Character const&) = delete;

private:
	vec3 c_pos; // 캐릭터 중앙 발 밑
	vec3 prev_pos = vec3(0, 0, 0);
	vec3 dir;
	vec3 up = vec3(0, 1, 0);
	Mat pos;
	Mat rot;
	shared_ptr<Texture> skin_tex;
	MVP mvp;
	MVP shadow_mvp;
	shared_ptr<ConstantBuffer> constant_buffer;
	shared_ptr<ConstantBuffer> constant_buffer_water;
	shared_ptr<Buffer<uint32>> i_buffer;

private:
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;

private:
	shared_ptr<LeftArm> left_arm;
	shared_ptr<RightArm> right_arm;
	shared_ptr<Head> head;
	shared_ptr<Body> body;
	shared_ptr<RightLeg> right_leg;
	shared_ptr<LeftLeg> left_leg;
	shared_ptr<Collision> aabb_collision;
};

