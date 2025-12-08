#include "pch.h"
#include "Entity.h"
#include "DeferredGraphics.h"
#include "VertexShader.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "PixelShader.h"
#include "RasterizerState.h"
#include "SamplerState.h"
#include "Character.h"
#include "DeferredBuffer.h"

Entity::Entity()
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/World/DeferredShader/Entity/EntityVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_ptn.data(),
		InputLayouts::layout_ptn.size(),
		this->vertex_shader->getBlob()
	);
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/World/DeferredShader/Entity/EntityPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->sampler_state = make_shared<SamplerState>(device);
	this->character = nullptr;
	this->d_buffer = make_shared<DeferredBuffer>(3);
	this->d_buffer->setRTVsAndSRVs(
		d_graphic->getDevice(),
		w_width,
		w_height
	);
}

void Entity::setRTV(ComPtr<ID3D11DepthStencilView> dsv)
{
	this->dsv = dsv;
}

void Entity::setCharacter(vec3 pos, vec3 radian_xyz)
{
	static int cnt = 0;
	if (cnt)
		return;
	Mat move = Mat::CreateTranslation(pos);
	Mat rot = Mat::CreateRotationX(radian_xyz.x) *
		Mat::CreateRotationY(radian_xyz.y) *
		Mat::CreateRotationZ(radian_xyz.z);
	this->character = make_shared<Character>(move, rot);
	cnt++;
}

void Entity::render(CamType type)
{
	this->setPipe();
	d_graphic->renderBegin(this->d_buffer.get(), this->dsv, true, false);
}

void Entity::characterRenderTP(
	CamType type,
	ComPtr<ID3D11ShaderResourceView> depth_srv,
	shared_ptr<SamplerState> sampler_tp,
	bool water_up_flag
)
{
	this->character->render(type, depth_srv, sampler_tp, water_up_flag);
}

void Entity::shadowRender(
	CamType type, 
	Mat const& view, 
	Mat const& proj, 
	ComPtr<ID3D11DepthStencilView> const& dsv
)
{
}

void Entity::setPipe()
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr, 0);
	context->RSSetState(this->rasterizer_state->getComPtr().Get());
	context->PSSetShader(
		this->pixel_shader->getComPtr().Get(),
		nullptr, 0);
	context->PSSetSamplers(0, 1, this->sampler_state->getComPtr().GetAddressOf());
}

ComPtr<ID3D11ShaderResourceView> Entity::getSRV()
{
	return this->d_buffer->getSRV(0);
}

ComPtr<ID3D11ShaderResourceView> Entity::getSRVPos()
{
	return this->d_buffer->getSRV(1);
}

ComPtr<ID3D11ShaderResourceView> Entity::getSRVNormal()
{
	return this->d_buffer->getSRV(2);
}

vec3 const& Entity::getCharacterPos()
{
	return this->character->getPos();
}

vec3 const& Entity::getCharacterDir()
{
	return this->character->getDir();
}

bool Entity::checkAABBWithEntity(vec3 const& block_pos)
{
	vec3 pos = this->getCharacterPos();
	vec3 max_a = pos + vec3(0.25, 2, 0.25);
	vec3 min_a = pos + vec3(-0.25, 0, -0.25);
	vec3 max_b = block_pos + vec3(1, 1, 0);
	vec3 min_b = block_pos + vec3(0, 0, -1);
	return (max_a.x > min_b.x &&
		min_a.x < max_b.x &&
		max_a.y > min_b.y &&
		min_a.y < max_b.y &&
		max_a.z > min_b.z &&
		min_a.z < max_b.z);
}

void Entity::update(vec3 const& cam_dir)
{
	this->character->update(cam_dir);
}

void Entity::setCharacterLeftArmAnimation()
{
	this->character->setLeftArmAnimation();
}

void Entity::checkEntityNowPos()
{
	this->character->checkValidatePosition();
}
