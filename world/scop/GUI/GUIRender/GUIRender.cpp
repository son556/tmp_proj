#include "pch.h"
#include "GUIRender.h"
#include "DeferredGraphics.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "TabItems.h"
#include "DeferredBuffer.h"
#include "ConstantBuffer.h"
#include "BlockTextureArray.h"

GUIRender::GUIRender()
{
	ComPtr<ID3D11Device> const& device = d_graphic->getDevice();
	
	this->deferred_buffer = make_shared<DeferredBuffer>(1);
	this->deferred_buffer->setRTVsAndSRVs(device, w_width, w_height);

	D3D11_BLEND_DESC blend_desc;
	ZeroMemory(&blend_desc, sizeof(blend_desc));
	blend_desc.AlphaToCoverageEnable = false;
	blend_desc.IndependentBlendEnable = false;

	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend_desc, this->blend_state.GetAddressOf());

	this->vertex_shader = make_shared<VertexShader>(device, L"shader/GUIShader/GUIRenderVS.hlsl",
		"main", "vs_5_0");
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->vertex_shader->getBlob()
	);
	this->pixel_shader = make_shared<PixelShader>(device, L"shader/GUIShader/GUIRenderPS.hlsl",
		"main", "ps_5_0");
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE
	);

	this->vertex_shader_color = make_shared<VertexShader>(device, L"shader/GUIShader/GUIRenderColorVS.hlsl",
		"main", "vs_5_0");
	this->pixel_shader_color = make_shared<PixelShader>(device, L"shader/GUIShader/GUIRenderColorPS.hlsl",
		"main", "ps_5_0");
	this->input_layout_color = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pc.data(),
		InputLayouts::layout_pc.size(),
		this->vertex_shader_color->getBlob()
	);

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
	depth_stencil_desc.DepthEnable = false;

	device->CreateDepthStencilState(&depth_stencil_desc,
		this->depth_stencil_state.GetAddressOf());

	this->sampler_state = make_shared<SamplerState>(device);
	vec4 tmp = { -1, -1, -1, -1 };
	this->constant_buffer = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		tmp
	);
}

void GUIRender::render(GUI* gui, bool rtv_reset)
{
	ComPtr<ID3D11DeviceContext> const& context = d_graphic->getContext();

	d_graphic->renderBegin(this->deferred_buffer.get(), nullptr, rtv_reset);
	
	vec4 idx = { -1, -1, -1, -1 };

	// gui render
	this->setPipe(context, false);
	this->constant_buffer->update(idx);
	context->PSSetConstantBuffers(0, 1, 
		this->constant_buffer->getComPtr().GetAddressOf());

	gui->setGUIBuffer(context);
	context->DrawIndexed(6, 0, 0);
	
	// gui 추가사항 render
	gui->optRender();

	// gui opacity item render
	context->PSSetConstantBuffers(0, 1,
		this->constant_buffer->getComPtr().GetAddressOf());

	int free_move_item_idx = -1;
	this->setPipe(context, false);
	int cnt = gui->getItemArraySize();
	for (int i = 0; i < cnt; i++) {
		shared_ptr<BlockItem> const& block = gui->getItem(i);
		if (block == nullptr)
			continue;
		if (block->checkFreeMove()) {
			free_move_item_idx = i;
			continue;
		}
		if (block->getBlockFlag() == false) {
			idx.x = blockOffset[static_cast<int>(block->getBlockType()) - 1][0];
			this->constant_buffer->update(idx);
			gui->setOpacityItemBuffer(context, i);
			context->DrawIndexed(6, 0, 0);
		}
	}

	// gui transparent item render
	this->setPipe(context, true);
	for (int i = 0; i < cnt; i++) {
		shared_ptr<BlockItem> const& block = gui->getItem(i);
		if (block == nullptr || i == free_move_item_idx)
			continue;
		if (block->getBlockFlag()) {
			gui->setTransParencyBuffer(context, i);
			context->DrawIndexed(6, 0, 0);
		}
	}

	if (free_move_item_idx > -1) {
		if (gui->getItem(free_move_item_idx)->getBlockFlag() == false) {
			this->setPipe(context, false);
			context->OMSetBlendState(
				nullptr,
				nullptr,
				0xFFFFFFFF
			);
			idx.x = 
				blockOffset[static_cast<int>(gui->getItem(free_move_item_idx)->getBlockType()) - 1][0];
			this->constant_buffer->update(idx);
			gui->setOpacityItemBuffer(context, free_move_item_idx);
		}
		else
			gui->setTransParencyBuffer(context, free_move_item_idx);
		context->DrawIndexed(6, 0, 0);
	}
	context->OMSetBlendState(
		nullptr,
		nullptr,
		0xFFFFFFFF
	);
	context->OMSetDepthStencilState(nullptr, 0);
}

void GUIRender::setPipe(
	ComPtr<ID3D11DeviceContext> const& context,
	bool tp_flag
)
{
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);
	context->RSSetState(this->rasterizer_state->getComPtr().Get());

	if (tp_flag) {
		context->IASetInputLayout(this->input_layout_color->getComPtr().Get());
		context->VSSetShader(
			this->vertex_shader_color->getComPtr().Get(),
			nullptr,
			0
		);
		context->PSSetShader(
			this->pixel_shader_color->getComPtr().Get(),
			nullptr,
			0
		);
		context->OMSetBlendState(
			this->blend_state.Get(),
			nullptr,
			0xFFFFFFFF
		);
	}
	else {
		context->IASetInputLayout(this->input_layout->getComPtr().Get());
		context->VSSetShader(
			this->vertex_shader->getComPtr().Get(),
			nullptr,
			0
		);
		context->PSSetShader(
			this->pixel_shader->getComPtr().Get(),
			nullptr,
			0
		);
		context->PSSetShaderResources(1, 1,
			BlockTextureArray::getBlocksColor()->getComPtr().GetAddressOf());
		context->PSSetSamplers(0, 1, this->sampler_state->getComPtr().GetAddressOf());
	}
	context->OMSetDepthStencilState(this->depth_stencil_state.Get(), 0);
}

ComPtr<ID3D11ShaderResourceView> const& GUIRender::getSRV()
{
	return this->deferred_buffer->getSRV(0);
}
