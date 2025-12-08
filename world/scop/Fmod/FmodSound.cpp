#include "pch.h"
//#include "FmodSound.h"
//
//FMOD::Channel* FmodSound::channel = nullptr;
//FMOD::Channel* FmodSound::bgms_channel = nullptr;
//FMOD::System* FmodSound::system = nullptr;
//FMOD::Sound* FmodSound::put_sound = nullptr;
//FMOD::Sound* FmodSound::bgms[4];
//FMOD::Sound* FmodSound::break_glass = nullptr;
//FMOD::Sound* FmodSound::break_leaf = nullptr;
//FMOD::Sound* FmodSound::selected_sound = nullptr;
//
//int bgm_idx = 0;
//bool play_next = false;
//
//FMOD_RESULT ChannelEndCallback(
//	FMOD_CHANNELCONTROL* channelControl, 
//	FMOD_CHANNELCONTROL_TYPE controlType, 
//	FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, 
//	void* commandData1, 
//	void* commandData2) {
//	if (callbackType == FMOD_CHANNELCONTROL_CALLBACK_END) {
//		bgm_idx++;
//		play_next = true;
//		if (bgm_idx == 3)
//			bgm_idx = 0;
//	}
//	return FMOD_OK;
//}
//
//void FmodSound::init()
//{
//	FMOD::System_Create(&(system));
//	system->init(32, FMOD_INIT_NORMAL, 0);
//	system->createSound("./resources/stone_effect.mp3", 
//		FMOD_LOOP_OFF, 0, &put_sound);
//	system->createSound("./resources/m_click.mp3", 
//		FMOD_LOOP_OFF, 0, &selected_sound);
//	
//	system->createSound("./resources/hal1.mp3", FMOD_LOOP_OFF, 0, &bgms[0]);
//	system->createSound("./resources/roblox.mp3", FMOD_LOOP_OFF, 0, &bgms[1]);
//	system->createSound("./resources/fishing.mp3", FMOD_LOOP_OFF, 0, &bgms[2]);
//	
//	system->createSound("./resources/minecraft-glass-break.mp3", FMOD_LOOP_OFF,
//		0, &break_glass);
//	system->createSound("./resources/Leaf_litter_break1.mp3", FMOD_LOOP_OFF, 0,
//		&break_leaf);
//}
//
//void FmodSound::playPutBlockSound(BlockType type)
//{
//	if (type == BlockType::GLASS ||
//		type == BlockType::TRANSPARENCY_BLUE ||
//		type == BlockType::TRANSPARENCY_GREEN ||
//		type == BlockType::TRANSPARENCY_RED) {
//		system->playSound(break_glass, 0, false, &channel);
//	}
//	else if (type == BlockType::OAK_LEAVES)
//		system->playSound(break_leaf, 0, false, &channel);
//	else
//		system->playSound(put_sound, 0, false, &channel);
//	system->update();
//}
//
//void FmodSound::playBGM()
//{
//	system->playSound(bgms[bgm_idx], 0, false, &bgms_channel);
//	bgms_channel->setCallback(ChannelEndCallback);
//	system->update();
//}
//
//void FmodSound::playNextBGM() // 계속 루프를 돌며 재생
//{
//	system->update();
//	if (play_next) {
//		system->playSound(bgms[bgm_idx], 0, false, &bgms_channel);
//		bgms_channel->setCallback(ChannelEndCallback);
//		play_next = false;
//	}
//}
//
//void FmodSound::playSelectedSound()
//{
//	system->playSound(selected_sound, 0, false, &channel);
//	system->update();
//}
//
//void FmodSound::release()
//{
//	system->release();
//	put_sound->release();
//	for (int i = 0; i < 4; i++)
//		bgms[i]->release();
//	selected_sound->release();
//	break_glass->release();
//	break_leaf->release();
//}
