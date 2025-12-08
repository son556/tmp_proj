#pragma once

class DeferredGraphics;
class Filter;

class Blur
{
public:
	Blur(UINT width, UINT height);
	void setStartSRV(ComPtr<ID3D11ShaderResourceView> start_srv);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	void render();
	~Blur() = default;

private:
	Blur() = delete;
	Blur(Blur const&) = delete;
	Blur& operator=(Blur const&) = delete;


private:
	void makeFilter();
	void makeFilterSetSRV();
	void makeFilterMultiLevel();
	void makeFilterSetMultiLevelSRV();
private:
	vector<shared_ptr<Filter>> down_filter;
	vector<shared_ptr<Filter>> up_blur_x_filter;
	vector<shared_ptr<Filter>> tmp_up_filter;
	vector<shared_ptr<Filter>> up_blur_y_filter;
	shared_ptr<Filter> final_filter;

private:
	UINT width;
	UINT height;
	int multi_level_target;
private:
	ComPtr<ID3D11ShaderResourceView> start_srv;
};

