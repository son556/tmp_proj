#pragma once

#include "TextureArray.h"

class BlockTextureArray
{
public:
	static void Init();
	static shared_ptr<TextureArray> const& getBlocksColor();
	static shared_ptr<TextureArray> const& getBlocksNormal();
	static shared_ptr<TextureArray> const& getBlocksS();

private:
	BlockTextureArray() = delete;
	BlockTextureArray const& operator=(BlockTextureArray const&) = delete;
	BlockTextureArray(BlockTextureArray const&) = delete;
	~BlockTextureArray() = delete;

private:
	static shared_ptr<TextureArray> blocks_color;
	static shared_ptr<TextureArray> blocks_normal;
	static shared_ptr<TextureArray> blocks_s;
};

