#include "pch.h"
#include "BlockItem.h"

bool BlockItem::getBlockFlag()
{
	return this->tp_flag;
}

void BlockItem::setInfo(bool tp_flag)
{
	this->tp_flag = tp_flag;
}

void BlockItem::copyFrom(BlockItem const& other)
{
	this->tp_flag = other.tp_flag;
}

BlockItem& BlockItem::operator=(BlockItem const& other)
{
	this->copyFrom(other);
	return *this;
}


