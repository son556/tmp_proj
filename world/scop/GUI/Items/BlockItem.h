#pragma once

template<typename T> class Buffer;

class BlockItem
{
public:
	BlockItem() = default;
	virtual ~BlockItem() = default;

	/**
	 * 블록이 반투명인지 불투명인지 반환합니다.
	 * 
	 * \return 반투명 true, 불투명 false 
	 */
	bool getBlockFlag();
	BlockItem& operator=(BlockItem const& other);
	virtual BlockType getBlockType() = 0;
	virtual void setPos(vec3 const& pos) = 0;

	/**
	 * ndc x y 좌표를 반환합니다.
	 * 
	 * \return 
	 */
	virtual pair<float, float> const& getPosXY() = 0;

	/**
	 * 마우스에 선택되어 마우스를 따라 움직이는 상태인지 확인.
	 * 
	 * \return 마우스를 따라 움직이면 true, 아니면 false
	 */
	virtual bool checkFreeMove() = 0;

	/**
	 * 마우스 따라 움직이는 상태 설정.
	 * 마우스를 따라움직이는 상태면 true 아니면 false
	 */
	virtual void setFreeMove(bool free_move_flag) = 0;

protected:
	void setInfo(bool tp_flag);
	virtual void copyFrom(BlockItem const& other);

private:
	BlockItem(BlockItem const&) = delete;

private:
	bool tp_flag = false;
};

