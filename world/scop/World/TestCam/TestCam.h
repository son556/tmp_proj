#pragma once

class ConstantBuffer;

class TestCam
{
public:
	TestCam(
		float width, 
		float height,
		float fov,
		float cam_near, 
		float cam_far
	);
	~TestCam() = default;
	void onMouseMove(HWND hWnd);
	void update(vec3 const& character_pos, vec3 const& character_dir);
	vec3 getPos();
	vec3 getDir();
	void setCursorInClient(HWND hwnd);
	float getFOV();
	MVP getMVP(CamType type);
	shared_ptr<ConstantBuffer>& getConstantBuffer(CamType type);
	pair<float, float> getCursorNDCPos(HWND hwnd);


public:
	void TestUpdate();

private:
	TestCam() = delete;
	TestCam(TestCam const&) = delete;
	TestCam& operator=(TestCam const) = delete;
	void SetMatrix();

private:
	float cam_near;
	float cam_far;
	float fov;
	vec3 pos;
	vec3 dir;
	vec3 up = vec3(0, 1, 0);
	MVP mvp;
	MVP reflection_mvp;
	Mat reflection_mat;
	Mat reflection_cmat;
	shared_ptr<ConstantBuffer> constant_buffer;
	shared_ptr<ConstantBuffer> constant_reflection_buffer;
};

