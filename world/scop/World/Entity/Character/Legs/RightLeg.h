#pragma once

template<typename T> class Buffer;
class ConstantBuffer;
struct VertexPTN;

class RightLeg
{
public:
	RightLeg(Mat const& o_pos, Mat const& o_rot);
	~RightLeg() = default;
	shared_ptr<Buffer<VertexPTN>>& getVertexBuffer();
	Mat getWorld();
	void update(Mat const& o_pos, Mat const& o_rot);
	void setAnimationFlag();

private:
	void animationWalk();

private:
	RightLeg() = delete;
	RightLeg(RightLeg const&) = delete;
	RightLeg& operator=(RightLeg const&) = delete;

private:
	Mat world;
	Mat basic_mat;

private:
	bool animation_flag = false;
	shared_ptr<Buffer<VertexPTN>> v_buffer;
};

