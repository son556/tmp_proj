#include "pch.h"
#include "BlockTextureArray.h"
#include "DeferredGraphics.h"

shared_ptr<TextureArray> BlockTextureArray::blocks_color;
shared_ptr<TextureArray> BlockTextureArray::blocks_normal;
shared_ptr<TextureArray> BlockTextureArray::blocks_s;


void BlockTextureArray::Init()
{
	vector<wstring> path_color = {
		L"./textures/pbr/test_sample/packed_mud.png", //0
		L"./textures/pbr/oak_tree/oak_log_top.png", // 1
		L"./textures/pbr/oak_tree/oak_log.png", // 2
		L"./textures/pbr/oak_tree/oak_leaves.png", // 3
		L"./textures/pbr/concrete/black_concrete.png",
		L"./textures/pbr/concrete/blue_concrete.png",
		L"./textures/pbr/concrete/brown_concrete.png",
		L"./textures/pbr/concrete/cyan_concrete.png",
		L"./textures/pbr/concrete/gray_concrete.png",
		L"./textures/pbr/concrete/green_concrete.png",
		L"./textures/pbr/concrete/light_blue_concrete.png",
		L"./textures/pbr/concrete/light_gray_concrete.png",
		L"./textures/pbr/concrete/lime_concrete.png",
		L"./textures/pbr/concrete/magenta_concrete.png",
		L"./textures/pbr/concrete/orange_concrete.png",
		L"./textures/pbr/concrete/pink_concrete.png",
		L"./textures/pbr/concrete/purple_concrete.png",
		L"./textures/pbr/concrete/red_concrete.png",
		L"./textures/pbr/concrete/white_concrete.png",
		L"./textures/pbr/concrete/yellow_concrete.png"
	};

	vector<wstring> path_normal{
		L"./textures/pbr/test_sample/packed_mud_n.png", //0
		L"./textures/pbr/oak_tree/oak_log_top_n.png", // 1
		L"./textures/pbr/oak_tree/oak_log_n.png", // 2
		L"./textures/pbr/oak_tree/oak_leaves_n.png", // 3
		L"./textures/pbr/concrete/black_concrete_n.png",
		L"./textures/pbr/concrete/blue_concrete_n.png",
		L"./textures/pbr/concrete/brown_concrete_n.png",
		L"./textures/pbr/concrete/cyan_concrete_n.png",
		L"./textures/pbr/concrete/gray_concrete_n.png",
		L"./textures/pbr/concrete/green_concrete_n.png",
		L"./textures/pbr/concrete/light_blue_concrete_n.png",
		L"./textures/pbr/concrete/light_gray_concrete_n.png",
		L"./textures/pbr/concrete/lime_concrete_n.png",
		L"./textures/pbr/concrete/magenta_concrete_n.png",
		L"./textures/pbr/concrete/orange_concrete_n.png",
		L"./textures/pbr/concrete/pink_concrete_n.png",
		L"./textures/pbr/concrete/purple_concrete_n.png",
		L"./textures/pbr/concrete/red_concrete_n.png",
		L"./textures/pbr/concrete/white_concrete_n.png",
		L"./textures/pbr/concrete/yellow_concrete_n.png"
	};

	vector<wstring> path_s = {
		L"./textures/pbr/test_sample/packed_mud_s.png", // 0
		L"./textures/pbr/oak_tree/oak_log_top_s.png", // 1
		L"./textures/pbr/oak_tree/oak_log_s.png", // 2
		L"./textures/pbr/oak_tree/oak_leaves_s.png", // 3
		L"./textures/pbr/concrete/black_concrete_s.png",
		L"./textures/pbr/concrete/blue_concrete_s.png",
		L"./textures/pbr/concrete/brown_concrete_s.png",
		L"./textures/pbr/concrete/cyan_concrete_s.png",
		L"./textures/pbr/concrete/gray_concrete_s.png",
		L"./textures/pbr/concrete/green_concrete_s.png",
		L"./textures/pbr/concrete/light_blue_concrete_s.png",
		L"./textures/pbr/concrete/light_gray_concrete_s.png",
		L"./textures/pbr/concrete/lime_concrete_s.png",
		L"./textures/pbr/concrete/magenta_concrete_s.png",
		L"./textures/pbr/concrete/orange_concrete_s.png",
		L"./textures/pbr/concrete/pink_concrete_s.png",
		L"./textures/pbr/concrete/purple_concrete_s.png",
		L"./textures/pbr/concrete/red_concrete_s.png",
		L"./textures/pbr/concrete/white_concrete_s.png",
		L"./textures/pbr/concrete/yellow_concrete_s.png"
	};

	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();

	blocks_color = make_shared<TextureArray>(
		device,
		context,
		path_color,
		0, true
	);

	blocks_normal = make_shared<TextureArray>(
		device,
		context,
		path_normal,
		0
	);

	blocks_s = make_shared<TextureArray>(
		device,
		context,
		path_s,
		0
	);
}

shared_ptr<TextureArray> const& BlockTextureArray::getBlocksColor()
{
	return blocks_color;
}

shared_ptr<TextureArray> const& BlockTextureArray::getBlocksNormal()
{
	return blocks_normal;
}

shared_ptr<TextureArray> const& BlockTextureArray::getBlocksS()
{
	return blocks_s;
}
