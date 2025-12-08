#pragma once

#include "GUI.h"

class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class SamplerState;

class TabItems : public GUI
{
public:
	TabItems(float w, float h);
	virtual void setGUIBuffer(
		ComPtr<ID3D11DeviceContext> const& context) override;
	
	virtual void setOpacityItemBuffer(
		ComPtr<ID3D11DeviceContext> const& context,
		int idx
	) override;
	
	virtual void setTransParencyBuffer(
		ComPtr<ID3D11DeviceContext> const& context,
		int idx
	) override;

	/**
	 * opacity -> false, transparency -> true.
	 * 
	 * \param idx
	 * \return 
	 */
	virtual bool getItemOTFlag(int idx) override;
	virtual shared_ptr<BlockItem> const& getItem(int idx) override;
	virtual int getItemArraySize() override;
	virtual void moveGUIPos(vec3 const& new_pos) override;
	virtual void moveItem(int idx, vec3 const& new_pos) override;
	virtual void optRender() override;

	/**
	 * 선택한 슬롯 인덱스 반환.
	 * 
	 * \param ndc_x 커서의 ndc x좌표
	 * \param ndc_y 커서의 ndc y좌표
	 * \return 슬롯의 인덱스(어떤 것도 선택되지 않으면 -2, 빈 슬롯 -1) 반환
	 *  
	 */
	int selectSlot(float ndc_x, float ndc_y);

	/**
	 * src_idx 의 아이템을 dst_idx로 옮깁니다.
	 * 
	 * \param src_idx
	 * \param dst_idx
	 */
	void moveSlotItem(int src_idx, int dst_idx);
	void setSlotItem(int idx, BlockType block_type);

	/**
	 * 인벤토리에 있는 아이템 반환.
	 * 
	 * \param idx inventory index(45 ~ 47)
	 * \return 
	 */
	BlockType getSlotItem(int idx);

	virtual void deleteSlotItem(int idx) override;

private:
	void setOpacityBlocks();
	void setTransparencyBlocks();

private:
	TabItems() = delete;
	TabItems& operator=(TabItems const&) = delete;
	TabItems(TabItems const&) = delete;

private:
	shared_ptr<ConstantBuffer> constant_buffer;
	shared_ptr<Buffer<VertexDefer>> v_buffer;
	shared_ptr<Buffer<uint32>> i_buffer;
	shared_ptr<ConstantBuffer> constant_buffer_xflag;

private:
	shared_ptr<Buffer<VertexDefer>> opt_v_buffer;
	shared_ptr<InputLayout> opt_input_layout;
	shared_ptr<VertexShader> opt_vertex_shader;
	shared_ptr<RasterizerState> opt_rasterizer_state;
	shared_ptr<PixelShader> opt_pixel_shader;
	shared_ptr<SamplerState> opt_sampler_state;

private:
	Mat gui_world = Mat::Identity;
	Mat select_gui_world = Mat::Identity;
	vector<shared_ptr<BlockItem>> items;
	const float gap_width = 0.148f;
	const float gap_height = 0.215;
	const float obh_size = w_width * 0.0001f - 0.01;
	const float obw_size = w_height * 0.0001f - 0.01;
	const float tbh_size = 2 * obh_size;
	const float tbw_size = 2 * obw_size;
	const float items_x_0 = -0.66f;
	const float items_y_0 = 0.7f;
	const float items_y_45 = -0.43f;
	bool opt_render_flag = false; // 임시
};

