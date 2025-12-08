#pragma once

#include "BlockItem.h"

struct VertexColor;

class TBlockItem : public BlockItem
{
public:
	TBlockItem() = default;
	virtual ~TBlockItem() = default;
	virtual void setPos(vec3 const& pos) override;
	virtual pair<float, float> const& getPosXY() override;
	Mat const& getWorldMatrix();
	shared_ptr<Buffer<VertexColor>> const& getVertexBuffer();
	shared_ptr<Buffer<uint32>> const& getIndexBuffer();
	/**
	 * item 정보 저장.
	 * 
	 * \param item_type 아이템 종류
	 * \param color 색(투명도 포함)
	 * \param tp_flag true
	 * \param b_size 한변의 길이가 1인 정사각형 기준에서 크기 조절
	 */
	void setInfo(BlockType item_type, vec4 const& color, 
		bool tp_flag, float w_size = -1, float h_size = -1);
	virtual BlockType getBlockType() override;
	virtual bool checkFreeMove() override;
	virtual void setFreeMove(bool free_move_flag) override;
	static vec4 const& getColor(BlockType block_type);

private:
	virtual void copyFrom(BlockItem const& othrer) override;
	TBlockItem(TBlockItem const&) = delete;
	TBlockItem& operator=(TBlockItem const&) = delete;

private:
	BlockType item_type;
	shared_ptr<Buffer<VertexColor>> v_buff;
	shared_ptr<Buffer<uint32>> i_buff;
	Mat world;
	pair<float, float> pos_xy;
	bool free_move_flag = false;

private:
	static const vec4 red;
	static const vec4 blue;
	static const vec4 green;
	static const vec4 glass;
	static const vec4 err_color;
};

