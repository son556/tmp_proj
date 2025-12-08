#pragma once

class InputLayout;
class VertexShader;
class RasterizerState;
class PixelShader;
class SamplerState;
class Character;
class DeferredBuffer;

class Entity
{
public:
	Entity();
	~Entity() = default;
	void setRTV(ComPtr<ID3D11DepthStencilView> dsv);

	/**
	 *
	 * 
	 * \param pos 캐릭터의 중앙 위치
	 * \param radian_xyz 캐릭터 회전, 캐릭터 초기 방향(0, -1, 0)
	 */
	void setCharacter(vec3 pos, vec3 radian_xyz);
	void render(CamType type);
	void characterRenderTP( // 캐릭터는 반투명으로 렌더링
		CamType type,
		ComPtr<ID3D11ShaderResourceView> depth_srv,
		shared_ptr<SamplerState> sampler_tp,
		bool water_up_flag
	);
	void shadowRender(
		CamType type,
		Mat const& view,
		Mat const& proj,
		ComPtr<ID3D11DepthStencilView> const& dsv
	);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	ComPtr<ID3D11ShaderResourceView> getSRVPos();
	ComPtr<ID3D11ShaderResourceView> getSRVNormal();
	vec3 const& getCharacterPos();
	vec3 const& getCharacterDir();
	bool checkAABBWithEntity(vec3 const& block_pos);
	void update(vec3 const& cam_dir);
	void setCharacterLeftArmAnimation();

	/**
	 * 갱신된 현재 위치가 맞는 지 확인합니다.
	 * 만약 주위에 충돌물체가 있는 경우 entity는 이전 위치로 설정됩니다.
	 * 
	 */
	void checkEntityNowPos();

private:
	Entity(Entity const&) = delete;
	Entity& operator=(Entity const&) = delete;

private:
	void setPipe();

private:
	shared_ptr<DeferredBuffer> d_buffer;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;

private:
	ComPtr<ID3D11DepthStencilView> dsv;

private:
	shared_ptr<Character> character;
};

