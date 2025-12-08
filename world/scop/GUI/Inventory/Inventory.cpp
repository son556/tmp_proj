#include "pch.h"
#include "Inventory.h"
#include "DeferredGraphics.h"
#include "ConstantBuffer.h"
#include "Buffer.h"
#include "Block.h"
#include "Texture.h"
#include "OBlockItem.h"
#include "TBlockItem.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "SamplerState.h"

Inventory::Inventory(float w, float h)
{
	ComPtr<ID3D11Device> const& device = d_graphic->getDevice();
	vector<uint32> indices;
	vector<VertexDefer> vertices;
	Block::makeBox(w, h, vertices, indices);
	this->v_buffer = make_shared<Buffer<VertexDefer>>(
		device,
		vertices.data(), vertices.size(), D3D11_BIND_VERTEX_BUFFER);
	this->i_buffer = make_shared<Buffer<uint32>>(
		device, indices.data(), indices.size(), D3D11_BIND_INDEX_BUFFER);

	Mat tmp;
	this->constant_buffer = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		tmp
	);
	this->items.resize(3);
	for (int i = 0; i < 3; i++)
		this->items[i] = nullptr;

	this->items[0] = make_shared<OBlockItem>();
	shared_ptr<OBlockItem>&& o_item = dynamic_pointer_cast<OBlockItem>(this->items[0]);
	o_item->setInfo(BlockType::GRASS, false);
	o_item->setPos(vec3(this->items_x0, this->items_y0, 0));

	this->items[1] = make_shared<TBlockItem>();
	shared_ptr<TBlockItem>&& t_item = dynamic_pointer_cast<TBlockItem>(this->items[1]);
	t_item->setInfo(BlockType::TRANSPARENCY_RED, vec4(1, 0, 0, 0.3), true);
	t_item->setPos(vec3(this->items_x0 + this->gap_width, this->items_y0, 0));

	this->items[2] = make_shared<TBlockItem>();
	t_item = dynamic_pointer_cast<TBlockItem>(this->items[2]);
	t_item->setInfo(BlockType::TRANSPARENCY_GREEN, vec4(0, 1, 0, 0.3), true);
	t_item->setPos(vec3(this->items_x0 + 2 * this->gap_width, this->items_y0, 0));

	vertices.clear();
	indices.clear();
	Block::makeBox(h, h, vertices, indices);
	this->opt_v_buffer = make_shared<Buffer<VertexDefer>>(
		device,
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->opt_vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/GUIShader/InventoryShader/InventoryOptVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->opt_vertex_shader->getBlob()
	);
	this->opt_pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/GUIShader/InventoryShader/InventoryOptPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->sampler_state = make_shared<SamplerState>(device);
	this->opt_rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE
	);

	Mat a = Mat::CreateTranslation(vec3(-0.2, -0.9, 0)).Transpose();
	this->constant_opt_buffer = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		a
	);
}

void Inventory::setGUIBuffer(ComPtr<ID3D11DeviceContext> const& context)
{
	uint32 stride = this->v_buffer->getStride();
	uint32 offset = this->v_buffer->getOffset();

	context->IASetVertexBuffers(0, 1,
		this->v_buffer->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(
		this->i_buffer->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT, 0);
	this->constant_buffer->update(this->gui_world.Transpose());
	context->VSSetConstantBuffers(0, 1, this->constant_buffer->getComPtr().GetAddressOf());
	shared_ptr<Texture> const& texture = 
		GUIResources::getTexture(GUITexture::INVENTORY);
	context->PSSetShaderResources(0, 1, texture->getComPtr().GetAddressOf());
}

void Inventory::setOpacityItemBuffer(ComPtr<ID3D11DeviceContext> const& context, int idx)
{
	if (this->items.size() <= idx || this->items[idx]->getBlockFlag())
		return;
	shared_ptr<OBlockItem> const& o_item =
		dynamic_pointer_cast<OBlockItem>(this->items[idx]);
	shared_ptr<Buffer<VertexDefer>> const& v_buff =
		o_item->getVertexBuffer();
	shared_ptr<Buffer<uint32>> const& i_buff =
		o_item->getIndexBuffer();
	uint32 stride = v_buff->getStride();
	uint32 offset = v_buff->getOffset();
	Mat world = o_item->getWorldMatrix().Transpose();
	this->constant_buffer->update(world);

	context->IASetVertexBuffers(0, 1,
		v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT, 0);
	context->VSSetConstantBuffers(0, 1, this->constant_buffer->getComPtr().GetAddressOf());
}

void Inventory::setTransParencyBuffer(
	ComPtr<ID3D11DeviceContext> const& context, 
	int idx
)
{
	if (this->items.size() <= idx || this->items[idx]->getBlockFlag() == false)
		return;

	shared_ptr<TBlockItem> const& t_item = dynamic_pointer_cast<TBlockItem>(this->items[idx]);
	shared_ptr<Buffer<VertexColor>> const& v_buff = t_item->getVertexBuffer();
	shared_ptr<Buffer<uint32>> const& i_buff = t_item->getIndexBuffer();
	uint32 stride = v_buff->getStride();
	uint32 offset = v_buff->getOffset();

	Mat world = t_item->getWorldMatrix().Transpose();
	this->constant_buffer->update(world);

	context->IASetVertexBuffers(0, 1,
		v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT, 0);
	context->VSSetConstantBuffers(0, 1, 
		this->constant_buffer->getComPtr().GetAddressOf());
}

int Inventory::getItemArraySize()
{
	return this->items.size();
}

shared_ptr<BlockItem> const& Inventory::getItem(int idx)
{
	return this->items[idx];
}

bool Inventory::getItemOTFlag(int idx)
{
	return this->items[idx]->getBlockFlag();
}

void Inventory::moveItem(int idx, vec3 const& new_pos)
{
	if (this->items[idx] == nullptr)
		return;
}

void Inventory::moveGUIPos(vec3 const& new_pos)
{
	this->gui_world = Mat::CreateTranslation(new_pos);
}

void Inventory::optRender()
{
	ComPtr<ID3D11DeviceContext> const& context = d_graphic->getContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	uint32 stride = this->opt_v_buffer->getStride();
	uint32 offset = this->opt_v_buffer->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->opt_v_buffer->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(
		this->i_buffer->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	context->VSSetShader(
		this->opt_vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->VSSetConstantBuffers(0, 1, 
		this->constant_opt_buffer->getComPtr().GetAddressOf());
	context->RSSetState(this->opt_rasterizer_state->getComPtr().Get());
	context->PSSetShader(this->opt_pixel_shader->getComPtr().Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, this->sampler_state->getComPtr().GetAddressOf());
	context->PSSetShaderResources(0, 1,
		GUIResources::getTexture(GUITexture::SELECT_ITEM)->getComPtr().GetAddressOf());
	context->DrawIndexed(6, 0, 0);
}

void Inventory::deleteSlotItem(int idx)
{
	this->items[idx] = nullptr;
}

BlockType Inventory::getBlockType(int idx)
{
	if (this->items[idx] == nullptr)
		return BlockType::AIR;
	return this->items[idx]->getBlockType();
}

void Inventory::selectItem(int idx)
{
	Mat m = Mat::Identity;
	if (idx == 0)
		m = Mat::CreateTranslation(vec3(-0.2, -0.9, 0)).Transpose();
	else if (idx == 1)
		m = Mat::CreateTranslation(vec3(0, -0.9, 0)).Transpose();
	else if (idx == 2)
		m = Mat::CreateTranslation(vec3(0.2, -0.9, 0)).Transpose();
	this->constant_opt_buffer->update(m);
}

void Inventory::setInventorySlot(int idx, BlockType block_type)
{
	if (idx < 0 || idx >= 3)
		assert(false);
	int type = static_cast<int>(block_type);
	if (type == 0)
		this->items[idx] = nullptr;
	else if (type > 0) {
		this->items[idx] = make_shared<OBlockItem>();
		shared_ptr<OBlockItem>&& o_item = 
			dynamic_pointer_cast<OBlockItem>(this->items[idx]);
		o_item->setInfo(block_type, false);
		o_item->setPos(vec3(this->items_x0 + idx * this->gap_width, this->items_y0, 0));
	}
	else {
		this->items[idx] = make_shared<TBlockItem>();
		shared_ptr<TBlockItem>&& t_item = 
			dynamic_pointer_cast<TBlockItem>(this->items[idx]);
		t_item->setInfo(block_type, TBlockItem::getColor(block_type), true);
		t_item->setPos(vec3(this->items_x0 + idx * this->gap_width, this->items_y0, 0));
	}
}
