#pragma once

#include "BlockItem.h"

template<typename T> class Buffer;
struct VertexDefer;
class Texture;

class OBlockItem : public BlockItem
{
public:
	OBlockItem() = default;
	~OBlockItem() = default;
	virtual void setPos(vec3 const& pos) override;
	virtual pair<float, float> const& getPosXY() override;
	Mat const& getWorldMatrix();
	shared_ptr<Buffer<VertexDefer>> const& getVertexBuffer();
	shared_ptr<Buffer<uint32>> const& getIndexBuffer();
	void setInfo(BlockType const& item_type, 
		bool tp_flag, float w_size = -1, float h_size = -1);
	virtual BlockType getBlockType() override;
	virtual bool checkFreeMove() override;
	virtual void setFreeMove(bool free_move_flag) override;

private:
	virtual void copyFrom(BlockItem const& othrer) override;
	OBlockItem(OBlockItem const&) = delete;
	OBlockItem& operator=(const OBlockItem&) = delete;

private:
	BlockType item_type;
	Mat world = Mat::Identity;
	shared_ptr<Buffer<VertexDefer>> v_buffer;
	shared_ptr<Buffer<uint32>> i_buffer;
	pair<float, float> pos_xy;
	bool free_move_flag = false;
};

