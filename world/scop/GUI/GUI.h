#pragma once

#include "OBlockItem.h"
#include "TBlockItem.h"
#include "GUIResources.h"

class ConstantBuffer;

class GUI
{
public:
	GUI() = default;
	virtual ~GUI() = default;
	virtual void setGUIBuffer(
		ComPtr<ID3D11DeviceContext> const& context) = 0;
	virtual void setOpacityItemBuffer(
		ComPtr<ID3D11DeviceContext> const& context,
		int idx
	) = 0;
	virtual void setTransParencyBuffer(
		ComPtr<ID3D11DeviceContext> const& context,
		int idx
	) = 0;
	virtual int getItemArraySize() = 0;
	virtual shared_ptr<BlockItem> const& getItem(int idx) = 0;
	virtual bool getItemOTFlag(int idx) = 0;

	/**
	 * 배열에 있는 반투명 아이템 위치를 이동합니다.
	 *
	 * \param idx 배열 인덱스
	 * \param new_pos 아이템의 새 위치
	 */
	virtual void moveItem(int idx, vec3 const& new_pos) = 0;
	virtual void moveGUIPos(vec3 const& new_pos) = 0;
	virtual void deleteSlotItem(int idx) = 0;

	/**
	 * gui 에 추가적인 랜더링이 필요한 경우 추가사항을 렌더링 해줍니다.
	 * 
	 */
	virtual void optRender() = 0;
};

