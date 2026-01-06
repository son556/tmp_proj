#include "pch.h"
#include "StartScene.h"
#include "DeferredGraphics.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "SamplerState.h"
#include "PixelShader.h"
#include "Texture.h"
#include "Block.h"
#include "Buffer.h"
#include "ConstantBuffer.h"
#include "TestCam.h"
#include "RasterizerState.h"
#include "Letter.h"

// 1 sky box 그리기(ndc box 기준 꽉 차게) 그 위에 버튼 그리기
StartScene::StartScene()
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
	depth_stencil_desc.DepthEnable = false;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.StencilEnable = false;

	device->CreateDepthStencilState(&depth_stencil_desc,
		this->depth_stencil_state.GetAddressOf());

	vector<VertexDefer> vertices;
	vector<uint32> indices;
	Block::makeBox(1, vertices, indices);
	this->v_buffer = make_shared<Buffer<VertexDefer>>(
		device,
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->i_buffer = make_shared<Buffer<uint32>>(
		device,
		indices.data(),
		indices.size(),
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = false;
	device->CreateDepthStencilState(&desc, this->depth_stencil_state.GetAddressOf());

	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/StartSceneShader/StartSceneVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->vertex_shader->getBlob()
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/StartSceneShader/StartScenePS.hlsl",
		"main",
		"ps_5_0"
	);
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE
	);
	this->sampler_state = make_shared<SamplerState>(device);

	// texture
	this->start_button = make_shared<Texture>(
		device,
		L"./textures/start_scene/button.png"
	);
	this->select_start_button = make_shared<Texture>(
		device,
		L"./textures/start_scene/button_highlighted.png"
	);
	this->title = make_shared<Texture>(
		device,
		L"./textures/start_scene/minecraft_title.png"
	);
	this->sky_box = make_shared<Texture>(
		device,
		L"./textures/start_scene/start_cube.dds",
		true
	);
	vec4 idx;
	ComPtr<ID3D11DeviceContext> const& context = d_graphic->getContext();
	this->texture_idx = make_shared<ConstantBuffer>(
		device,
		context,
		idx
	);

	this->constant_buffer_vs = make_shared<ConstantBuffer>(
		device,
		context,
		this->c_vs
	);
	Letter::init();
}

void StartScene::render()
{
	static bool testRenderFlag = true;
	
	/*if (testRenderFlag)
	{
		if (testRenderFlag)
		{
			auto ctx = d_graphic->getContext();
			ctx->IASetPrimitiveTopology(
				D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
			);
			ctx->Draw(3, 0);
			testRenderFlag = false;
		}
		testRenderFlag = false;
	}*/
	this->setPipe();
	ComPtr<ID3D11DeviceContext> const& context = d_graphic->getContext();
	vec4 c_dir = vec4(0, 0, 1, 0);
	c_dir = vec4::Transform(c_dir, Mat::CreateRotationY(this->cam_radian));
	this->cam_radian += 0.05 * delta_time;
	Mat view = XMMatrixLookAtLH(vec3(0, 0, 0), vec3(c_dir.x, c_dir.y, c_dir.z),
		vec3(0, 1, 0));
	view = (view * this->proj).Invert().Transpose();
	this->c_vs.inv_vp = view;
	
	d_graphic->renderBegin();
	
	// sky box
	this->c_vs.world = Mat::Identity;
	this->constant_buffer_vs->update(this->c_vs);
	vec4 idx = vec4(4, 4, 4, 4);
	this->texture_idx->update(idx);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// title
	this->c_vs.world = this->title_world;
	this->constant_buffer_vs->update(this->c_vs);
	idx.x = 2;
	this->texture_idx->update(idx);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// start button
	this->c_vs.world = this->button_world;
	this->constant_buffer_vs->update(this->c_vs);
	idx.x = 0;
	this->texture_idx->update(idx);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// select_button
	if (this->checkClickStartButton()) {
		idx.x = 1;
		this->texture_idx->update(idx);
		context->DrawIndexed(this->i_buffer->getCount(), 0, 0);
	}
	this->startGameLetterRender(context);

	// exit button
	this->c_vs.world = this->button_exit_world;
	this->constant_buffer_vs->update(this->c_vs);
	idx.x = 0;
	this->texture_idx->update(idx);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// select_exit_button
	if (this->checkClickExitButton()) {
		idx.x = 1;
		this->texture_idx->update(idx);
		context->DrawIndexed(this->i_buffer->getCount(), 0, 0);
	}
	this->exitLetterRender(context);

	context->OMSetDepthStencilState(nullptr, 0);
	d_graphic->renderEnd();
}

bool StartScene::checkClickStartButton()
{
	pair<float, float> c_pos = cam->getCursorNDCPos(hWnd);
	static const float sx = -0.5;
	static const float ex = 0.5;
	static const float sy = 0.0727 + 0.1;
	static const float ey = 0.0727 - 0.1;
	if (c_pos.first > sx && c_pos.first < ex && c_pos.second < sy && c_pos.second > ey)
		return true;
	return false;
}

bool StartScene::checkClickExitButton()
{
	pair<float, float> c_pos = cam->getCursorNDCPos(hWnd);
	static const float sx = -0.5;
	static const float ex = 0.5;
	static const float sy = -0.15 + 0.1;
	static const float ey = -0.15 - 0.1;
	if (c_pos.first > sx && c_pos.first < ex && c_pos.second < sy && c_pos.second > ey)
		return true;
	return false;
}

void StartScene::setPipe()
{
	ComPtr<ID3D11DeviceContext> const& context = d_graphic->getContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	uint32 offset = this->v_buffer->getOffset();
	uint32 stride = this->v_buffer->getStride();
	context->IASetVertexBuffers(0, 1, this->v_buffer->getComPtr().GetAddressOf(),
		&stride, &offset);
	context->IASetIndexBuffer(this->i_buffer->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT, 0);

	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr, 0);
	context->VSSetConstantBuffers(0, 1,
		this->constant_buffer_vs->getComPtr().GetAddressOf());
	
	context->RSSetState(this->rasterizer_state->getComPtr().Get());
	
	context->PSSetShader(
		this->pixel_shader->getComPtr().Get(),
		nullptr, 0);
	context->PSSetConstantBuffers(0, 1, this->texture_idx->getComPtr().GetAddressOf());
	context->PSSetSamplers(0, 1, this->sampler_state->getComPtr().GetAddressOf());
	
	context->PSSetShaderResources(0, 1, this->start_button->getComPtr().GetAddressOf());
	context->PSSetShaderResources(1, 1, 
		this->select_start_button->getComPtr().GetAddressOf());
	context->PSSetShaderResources(2, 1, this->title->getComPtr().GetAddressOf());
	context->PSSetShaderResources(3, 1, Letter::getTexture()->getComPtr().GetAddressOf());
	context->PSSetShaderResources(4, 1, this->sky_box->getComPtr().GetAddressOf());
	context->OMSetDepthStencilState(this->depth_stencil_state.Get(), 0);
}

void StartScene::startGameLetterRender(ComPtr<ID3D11DeviceContext> const& context)
{
	uint32 offset;
	uint32 stride;
	float start = -0.25;
	float sh = 0.073;

	vec4 idx = vec4(3, 0, 0, 0);
	this->texture_idx->update(idx);
	
	// S
	Mat world = Mat::CreateTranslation(start, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	offset = Letter::getLetterVertexBuffer('S')->getOffset();
	stride = Letter::getLetterVertexBuffer('S')->getStride();
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('S')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);
	
	// t
	world = Mat::CreateTranslation(start + this->w, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('t')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// a
	world = Mat::CreateTranslation(start + this->w * 1.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('a')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// r
	world = Mat::CreateTranslation(start + this->w * 2.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('r')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// t
	world = Mat::CreateTranslation(start + this->w * 3.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('t')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// space
	world = Mat::CreateTranslation(start + this->w * 4.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer(' ')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// G
	world = Mat::CreateTranslation(start + this->w * 5.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('G')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// a
	world = Mat::CreateTranslation(start + this->w * 6.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('a')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// m
	world = Mat::CreateTranslation(start + this->w * 7.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('m')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// e
	world = Mat::CreateTranslation(start + this->w * 8.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('e')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	context->IASetVertexBuffers(0, 1,
		this->v_buffer->getComPtr().GetAddressOf(), &stride, &offset);
}

void StartScene::exitLetterRender(ComPtr<ID3D11DeviceContext> const& context)
{
	uint32 offset = Letter::getLetterVertexBuffer(' ')->getOffset();
	uint32 stride = Letter::getLetterVertexBuffer(' ')->getStride();
	float start = -0.08;
	float sh = -0.155;
	Mat world;
	vec4 idx = vec4(3, 0, 0, 0);
	this->texture_idx->update(idx);

	// E
	world = Mat::CreateTranslation(start, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('E')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);
	
	// x
	world = Mat::CreateTranslation(start + this->w, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('x')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// i
	world = Mat::CreateTranslation(start + this->w * 2, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('i')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);

	// t
	world = Mat::CreateTranslation(start + this->w * 2.7, sh, 0);
	this->c_vs.world = (this->letter_scale * world).Transpose();
	this->constant_buffer_vs->update(this->c_vs);
	context->IASetVertexBuffers(0, 1,
		Letter::getLetterVertexBuffer('t')->getComPtr().GetAddressOf(), &stride, &offset);
	context->DrawIndexed(this->i_buffer->getCount(), 0, 0);
	
	context->IASetVertexBuffers(0, 1,
		this->v_buffer->getComPtr().GetAddressOf(), &stride, &offset);
}
