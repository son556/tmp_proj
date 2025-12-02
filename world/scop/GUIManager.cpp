#include "pch.h"
#include "GUIManager.h"
#include "GUIResources.h"
#include "TabItems.h"
#include "Inventory.h"
#include "TestCam.h"
//#include "FmodSound.h"

GUIManager::GUIManager()
{
	GUIResources::initialize();
	int idx = static_cast<int>(GUITexture::TAB_ITEMS);
	this->gui_arr.resize(2);
	this->gui_arr[idx] = make_shared<TabItems>(0.8, 0.8);
	this->gui_arr[idx]->moveGUIPos(vec3(0, 0.2, 0));

	idx = static_cast<int>(GUITexture::INVENTORY);
	this->gui_arr[idx] = make_shared<Inventory>(0.3, 0.1);
	this->gui_arr[idx]->moveGUIPos(vec3(0, -0.9, 0));
}

void GUIManager::render(GUITexture idx, bool click_flag)
{
	static bool flag = false;
	static int click_slot = -1;
	if (GUITexture::TAB_ITEMS == idx) {
		shared_ptr<Inventory>&& inv =
			dynamic_pointer_cast<Inventory>
			(this->gui_arr[static_cast<int>(GUITexture::INVENTORY)]);
		shared_ptr<TabItems>&& t_item =
			dynamic_pointer_cast<TabItems>(
				this->gui_arr[static_cast<int>(GUITexture::TAB_ITEMS)]);
		
		// 아이템 동기화
		for (int i = 0; i < 3; i++) {
			BlockType b_type = inv->getBlockType(i);
			t_item->setSlotItem(45 + i, b_type);
		}

		pair<float, float> ndc_xy = cam->getCursorNDCPos(hWnd);
		int s_idx = t_item->selectSlot(ndc_xy.first, ndc_xy.second);
		shared_ptr<BlockItem> block = nullptr;
		if (s_idx > -1)
			block = t_item->getItem(s_idx);
		if (click_slot > -1)
			t_item->moveItem(click_slot, vec3(ndc_xy.first, ndc_xy.second, 0));
		if (flag == false && click_flag && block) {
			//FmodSound::playSelectedSound();
			click_slot = s_idx;
			flag = true;
			t_item->moveItem(click_slot, vec3(ndc_xy.first, ndc_xy.second, 0));
			t_item->getItem(click_slot)->setFreeMove(true);
		}
		else if (flag && click_flag && s_idx > -1) {
			//FmodSound::playSelectedSound();
			if (s_idx == click_slot)
				t_item->setSlotItem(click_slot, t_item->getSlotItem(click_slot));
			else {
				if (s_idx >= 45) {
					if (click_slot >= 45) {
						BlockType b_type = t_item->getSlotItem(click_slot);
						t_item->moveSlotItem(click_slot, s_idx);
						t_item->setSlotItem(click_slot, b_type);
					}
					else {
						t_item->setSlotItem(s_idx, t_item->getSlotItem(click_slot));
						t_item->setSlotItem(click_slot, t_item->getSlotItem(click_slot));
					}
				}
				else if (click_slot >= 45)
					t_item->deleteSlotItem(click_slot);
				else if (block == nullptr)
					t_item->moveSlotItem(click_slot, s_idx);
				else
					t_item->setSlotItem(click_slot, t_item->getSlotItem(click_slot));
			}
			if (s_idx >= 45 || click_slot >= 45) {
				for (int i = 0; i < 3; i++)
					inv->setInventorySlot(i, t_item->getSlotItem(45 + i));
			}
			flag = false;
			click_slot = -1;
		}
	}

	this->gui_render.render(this->gui_arr[static_cast<int>(idx)].get(), true);
}

void GUIManager::render()
{
	this->gui_render.render(
		this->gui_arr[static_cast<int>(GUITexture::INVENTORY)].get(), true);
}

ComPtr<ID3D11ShaderResourceView> const& GUIManager::getSRV()
{
	return this->gui_render.getSRV();
}

BlockType GUIManager::getInventoryBlock(int idx)
{
	shared_ptr<Inventory>&& inven =
		dynamic_pointer_cast<Inventory>(
			this->gui_arr[static_cast<int>(GUITexture::INVENTORY)]);
	return inven->getBlockType(idx);
}

void GUIManager::selectInventoryItem(int idx)
{
	shared_ptr<Inventory>&& inven =
		dynamic_pointer_cast<Inventory>(
			this->gui_arr[static_cast<int>(GUITexture::INVENTORY)]);
	inven->selectItem(idx);
}
