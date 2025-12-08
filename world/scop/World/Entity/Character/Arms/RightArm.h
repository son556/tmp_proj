#pragma once

template<typename T> class Buffer;
class ConstantBuffer;
struct VertexPTN;

class RightArm
{
public:
	RightArm(Mat const& o_pos, Mat const& o_rot);
	~RightArm() = default;
	shared_ptr<Buffer<VertexPTN>>& getVertexBuffer();
	Mat getWorld();
	void update(Mat const& o_pos, Mat const& o_rot);
	void setAnimationFlag();

private:
	void animationWalk();

private:
	RightArm() = delete;
	RightArm(RightArm const&) = delete;
	RightArm& operator=(RightArm const&) = delete;

private:
	Mat world;
	Mat basic_mat;

private:
	bool animation_flag;
	shared_ptr<Buffer<VertexPTN>> v_buffer;
};

