#include "pch.h"
#include "TabItems.h"
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

TabItems::TabItems(float w, float h)
{
	ComPtr<ID3D11Device> const& device = d_graphic->getDevice();

	vector<VertexDefer> vertices;
	vector<uint32> indices;
	Block::makeBox(w, h, vertices, indices);
	this->v_buffer = make_shared<Buffer<VertexDefer>>(
		device, vertices.data(), vertices.size(),
		D3D11_BIND_VERTEX_BUFFER);
	this->i_buffer = make_shared<Buffer<uint32>>(
		device, indices.data(), indices.size(),
		D3D11_BIND_INDEX_BUFFER);

	Mat tmp;
	this->constant_buffer = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		tmp
	);
	this->items.resize(9 * 5 + 3);
	for (int i = 0; i < this->items.size(); i++)
		this->items[i] = nullptr;

	this->setOpacityBlocks();
	this->setTransparencyBlocks();

	// opt setting
	vertices.clear();
	indices.clear();
	Block::makeBox(this->obw_size + 0.01, this->obh_size + 0.01, vertices, indices);
	this->opt_v_buffer = make_shared<Buffer<VertexDefer>>(
		device,
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);

	this->opt_vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/GUIShader/TabItemShader/TabItemsOptVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->opt_input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->opt_vertex_shader->getBlob()
	);
	this->opt_pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/GUIShader/TabItemShader/TabItemsOptPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->opt_rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE
	);
	this->opt_sampler_state = make_shared<SamplerState>(device);
	this->constant_buffer_xflag = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		vec4(1, 1, 1, 1)
	);
}

void TabItems::setGUIBuffer(ComPtr<ID3D11DeviceContext> const& context)
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
	shared_ptr<Texture> const& texture = GUIResources::getTexture(GUITexture::TAB_ITEMS);
	context->PSSetShaderResources(0, 1, texture->getComPtr().GetAddressOf());
}

void TabItems::setOpacityItemBuffer(ComPtr<ID3D11DeviceContext> const& context, int idx)
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

	context->IASetVertexBuffers(0, 1,
		v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT, 0);

	Mat world = o_item->getWorldMatrix().Transpose();
	this->constant_buffer->update(world);
	context->VSSetConstantBuffers(0, 1, this->constant_buffer->getComPtr().GetAddressOf());
}

void TabItems::setTransParencyBuffer(ComPtr<ID3D11DeviceContext> const& context, int idx)
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
	context->VSSetConstantBuffers(0, 1, this->constant_buffer->getComPtr().GetAddressOf());
}

bool TabItems::getItemOTFlag(int idx)
{
	return this->items[idx]->getBlockFlag();
}

shared_ptr<BlockItem> const& TabItems::getItem(int idx)
{
	return this->items[idx];
}

int TabItems::getItemArraySize()
{
	return this->items.size();
}

void TabItems::moveGUIPos(vec3 const& new_pos)
{
	this->gui_world = Mat::CreateTranslation(new_pos);
}

void TabItems::moveItem(int idx, vec3 const& new_pos)
{
	if (this->items[idx] == nullptr)
		return;
	this->items[idx]->setPos(new_pos);
}

void TabItems::optRender()
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
	context->IASetInputLayout(this->opt_input_layout->getComPtr().Get());
	context->VSSetShader(
		this->opt_vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->VSSetConstantBuffers(0, 1,
		this->constant_buffer->getComPtr().GetAddressOf());
	context->RSSetState(this->opt_rasterizer_state->getComPtr().Get());
	context->PSSetShader(this->opt_pixel_shader->getComPtr().Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, this->opt_sampler_state->getComPtr().GetAddressOf());
	context->PSSetShaderResources(0, 1,
		GUIResources::getTexture(GUITexture::TAB_ITEM_X)->getComPtr().GetAddressOf());
	
	this->constant_buffer_xflag->update(vec4(1, 1, 1, 1));
	context->PSSetConstantBuffers(0, 1, 
		this->constant_buffer_xflag->getComPtr().GetAddressOf());
	
	float sx = this->items_x_0;
	for (int i = 3; i < 9; i++) {
		this->constant_buffer->
			update(Mat::CreateTranslation(vec3(this->items_x_0 + i * this->gap_width,
				this->items_y_45, 0)).Transpose());
		context->DrawIndexed(6, 0, 0);
	}
	if (this->opt_render_flag == false)
		return;
	this->constant_buffer_xflag->update(vec4(0, 0, 0, 0));
	this->constant_buffer->update(this->select_gui_world);
	context->PSSetShaderResources(0, 1,
		GUIResources::getTexture(GUITexture::SELECT_TAB_ITEM)->getComPtr().GetAddressOf());
	context->DrawIndexed(6, 0, 0);
}

int TabItems::selectSlot(float ndc_x, float ndc_y)
{
	this->opt_render_flag = false;
	if (ndc_x < -0.735 || ndc_y > 0.81)
		return -1;
	if (ndc_x > this->items_x_0 + 8 * this->gap_width + 0.075)
		return -1;
	float sx = this->items_x_0 - 0.075;
	float sy = this->items_y_0 + 0.11;
	float ex = this->items_x_0 + 0.075;
	float ey = this->items_y_0 - 0.11;
	for (int i = 0; i < 5; i++) {
		if (i) {
			sy = ey;
			ey -= 0.22;
		}
		sx = this->items_x_0 - 0.075;
		ex = this->items_x_0 + 0.075;
		for (int j = 0; j < 9; j++) {
			if (j) {
				sx = ex;
				ex += 0.15;
			}
			if (ndc_x >= sx && ndc_x < ex && ndc_y <= sy && ndc_y > ey) {
				this->opt_render_flag = true;
				this->select_gui_world = Mat::CreateTranslation(
						vec3(this->items_x_0 + j * this->gap_width, 
							this->items_y_0 - i * this->gap_height, 0)).Transpose();
				return i * 9 + j;
			}
		}
	}
	sx = this->items_x_0 - 0.075;
	sy = this->items_y_45 + 0.11;
	ex = this->items_x_0 + 0.075;
	ey = this->items_y_45 - 0.11;
	for (int j = 0; j < 3; j++) {
		if (j) {
			sx = ex;
			ex += 0.15;
		}
		if (ndc_x >= sx && ndc_x < ex && ndc_y <= sy && ndc_y > ey) {
			this->opt_render_flag = true;
			this->select_gui_world = Mat::CreateTranslation(
					vec3(this->items_x_0 + j * this->gap_width,
						this->items_y_45, 0)).Transpose();
			return 5 * 9 + j;
		}
	}
	return -1;
}

void TabItems::moveSlotItem(int src_idx, int dst_idx)
{
	this->items[dst_idx] = this->items[src_idx];
	this->items[dst_idx]->setFreeMove(false);
	int gap_w = dst_idx % 9;
	int gap_h = dst_idx / 9;
	if (dst_idx < 45) {
		this->items[dst_idx]->setPos(
			vec3(this->items_x_0 + gap_w * this->gap_width,
				this->items_y_0 - gap_h * this->gap_height, 0));
	}
	else {
		this->items[dst_idx]->setPos(
			vec3(this->items_x_0 + gap_w * this->gap_width, this->items_y_45, 0));
	}
	if (dst_idx != src_idx)
		this->items[src_idx] = nullptr;
}

void TabItems::setSlotItem(int idx, BlockType block_type)
{
	if (idx < 0 || idx >= 9 * 5 + 3)
		assert(false);
	
	int type = static_cast<int>(block_type);
	int gap_w = idx % 9;
	int gap_h = idx / 9;
	if (type < 0) {
		this->items[idx] = make_shared<TBlockItem>();
		shared_ptr<TBlockItem>&& t_item = 
			dynamic_pointer_cast<TBlockItem>(this->items[idx]);
		t_item->setInfo(block_type, TBlockItem::getColor(block_type), true,
			this->tbw_size, this->tbh_size);
		if (idx >= 45) {
			t_item->setPos(vec3(this->items_x_0 + gap_w * this->gap_width,
				this->items_y_45, 0));
		}
		else {
			t_item->setPos(vec3(this->items_x_0 + gap_w * this->gap_width,
				this->items_y_0 - gap_h * this->gap_height, 0));
		}
	}
	else if (type > 0) {
		this->items[idx] = make_shared<OBlockItem>();
		shared_ptr<OBlockItem>&& o_item = 
			dynamic_pointer_cast<OBlockItem>(this->items[idx]);
		o_item->setInfo(block_type, false, this->obw_size, this->obh_size);
		if (idx >= 45) {
			o_item->setPos(vec3(this->items_x_0 + gap_w * this->gap_width,
				this->items_y_45, 0));
		}
		else {
			o_item->setPos(vec3(this->items_x_0 + gap_w * this->gap_width,
				this->items_y_0 - gap_h * this->gap_height, 0));
		}
	}
	else {
		this->items[idx] = nullptr;
	}
}

BlockType TabItems::getSlotItem(int idx)
{
	if (idx < 0 || idx >= 48)
		assert(false);
	if (this->items[idx] == nullptr)
		return BlockType::AIR;
	return this->items[idx]->getBlockType();
}

void TabItems::deleteSlotItem(int idx)
{
	this->items[idx] = nullptr;
}

void TabItems::setOpacityBlocks()
{
	this->items[0] = make_shared<OBlockItem>();
	shared_ptr<OBlockItem>&& o_item = dynamic_pointer_cast<OBlockItem>(this->items[0]);
	o_item->setInfo(BlockType::GRASS, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0, this->items_y_0, 0));

	this->items[4] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[4]);
	o_item->setInfo(BlockType::BLACK_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 4 * this->gap_width, this->items_y_0, 0));

	this->items[5] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[5]);
	o_item->setInfo(BlockType::BLUE_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 5 * this->gap_width, this->items_y_0, 0));

	this->items[6] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[6]);
	o_item->setInfo(BlockType::BROWN_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 6 * this->gap_width, this->items_y_0, 0));

	this->items[7] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[7]);
	o_item->setInfo(BlockType::CYAN_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 7 * this->gap_width, this->items_y_0, 0));

	this->items[8] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[8]);
	o_item->setInfo(BlockType::GRAY_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 8 * this->gap_width, this->items_y_0, 0));

	this->items[9] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[9]);
	o_item->setInfo(BlockType::GREEN_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0, this->items_y_0 - this->gap_height, 0));

	this->items[10] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[10]);
	o_item->setInfo(BlockType::LIGHT_BLUE_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[11] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[11]);
	o_item->setInfo(BlockType::LIGHT_GRAY_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 2 * this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[12] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[12]);
	o_item->setInfo(BlockType::LIME_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 3 * this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[13] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[13]);
	o_item->setInfo(BlockType::MAGENTA_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 4 * this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[14] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[14]);
	o_item->setInfo(BlockType::ORANGE_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 5 * this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[15] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[15]);
	o_item->setInfo(BlockType::PINK_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 6 * this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[16] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[16]);
	o_item->setInfo(BlockType::PURPLE_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 7 * this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[17] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[17]);
	o_item->setInfo(BlockType::RED_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + 8 * this->gap_width, this->items_y_0 - this->gap_height, 0));

	this->items[18] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[18]);
	o_item->setInfo(BlockType::WHITE_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0, this->items_y_0 - 2 * this->gap_height, 0));

	this->items[19] = make_shared<OBlockItem>();
	o_item = dynamic_pointer_cast<OBlockItem>(this->items[19]);
	o_item->setInfo(BlockType::YELLOW_CONCRETE, false, this->obw_size, this->obh_size);
	o_item->setPos(vec3(this->items_x_0 + this->gap_width, this->items_y_0 - 2 * this->gap_height, 0));
}

void TabItems::setTransparencyBlocks()
{
	this->items[1] = make_shared<TBlockItem>();
	shared_ptr<TBlockItem>&& t_item = dynamic_pointer_cast<TBlockItem>(this->items[1]);
	t_item->setInfo(BlockType::TRANSPARENCY_RED,
		vec4(1, 0, 0, 0.3), true, this->tbw_size, this->tbh_size);
	t_item->setPos(vec3(this->items_x_0 + this->gap_width, this->items_y_0, 0));

	this->items[2] = make_shared<TBlockItem>();
	t_item = dynamic_pointer_cast<TBlockItem>(this->items[2]);
	t_item->setInfo(BlockType::TRANSPARENCY_BLUE, vec4(0, 0, 1, 0.3),
		true, this->tbw_size, this->tbh_size);
	t_item->setPos(vec3(this->items_x_0 + 2 * this->gap_width, this->items_y_0, 0));

	this->items[3] = make_shared<TBlockItem>();
	t_item = dynamic_pointer_cast<TBlockItem>(this->items[3]);
	t_item->setInfo(BlockType::TRANSPARENCY_GREEN, vec4(0, 1, 0, 0.3),
		true, this->tbw_size, this->tbh_size);
	t_item->setPos(vec3(this->items_x_0 + 3 * this->gap_width, this->items_y_0, 0));

	this->items[20] = make_shared<TBlockItem>();
	t_item = dynamic_pointer_cast<TBlockItem>(this->items[20]);
	t_item->setInfo(BlockType::GLASS, vec4(0.06, 0.07, 0.1, 0.6),
		true, this->tbw_size, this->tbh_size);
	t_item->setPos(vec3(this->items_x_0 + 2 * this->gap_width, this->items_y_0 - 2 * this->gap_height, 0));
}

