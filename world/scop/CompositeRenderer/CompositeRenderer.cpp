#include "pch.h"
#include "CompositeRenderer.h"
#include "DeferredGraphics.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "Buffer.h"
#include "Block.h"

CompositeRenderer::CompositeRenderer()
{
	ComPtr<ID3D11Device> const& device = d_graphic->getDevice();
	vector<VertexDefer> vertices;
	vector<uint32> indices;
	Block::makeBox(1, vertices, indices);
	this->v_buff = make_shared<Buffer<VertexDefer>>(
		device,
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->i_buff = make_shared<Buffer<uint32>>(
		device,
		indices.data(),
		indices.size(),
		D3D11_BIND_INDEX_BUFFER
	);
	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/AllComposite/CompositeRendererVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->vertex_shader->getBlob()
	);
	this->rasterize_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/AllComposite/CompositeRendererPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->sampler_state = make_shared<SamplerState>(device);
}

void CompositeRenderer::render(
	ComPtr<ID3D11ShaderResourceView> const& game_srv,
	ComPtr<ID3D11ShaderResourceView> const& gui_srv
)
{
	ComPtr<ID3D11DeviceContext> const& context = d_graphic->getContext();
	this->setPipe(context);
	d_graphic->renderBegin();
	context->PSSetShaderResources(0, 1, game_srv.GetAddressOf());
	context->PSSetShaderResources(1, 1, gui_srv.GetAddressOf());
	context->DrawIndexed(this->i_buff->getCount(), 0, 0);
	d_graphic->renderEnd();
}

void CompositeRenderer::setPipe(ComPtr<ID3D11DeviceContext> const& context)
{
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	uint32 stride = this->v_buff->getStride();
	uint32 offset = this->v_buff->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(this->i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT, 0);

	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);

	context->RSSetState(this->rasterize_state->getComPtr().Get());

	context->PSSetShader(
		this->pixel_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->PSSetSamplers(0, 1, this->sampler_state->getComPtr().GetAddressOf());
}
