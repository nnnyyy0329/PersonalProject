#pragma once
#include "GameObjectBase.h"

class Player;
class Enemy;
class Bullet;
class Camera;

class Map : public GameObjectBase
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画

	// ゲッターセッター
	int GetHandleMap() const { return _handleMap; }

	int GetFrameMapCol() const { return _frameMapCollision; }

	VECTOR GetLightDir() const { return _vLightDir; }
	void SetLightDir(const VECTOR lightDir) { this->_vLightDir = lightDir; }

	float GetColSubY() const { return _colSubY; }
	void SetColSubY(const float colSubY){ this->_colSubY = colSubY; }

	// クラスセット
	void SetPlayer(Player* player){ _player = player; }				 // プレイヤーセット
	void SetEnemy(std::vector<Enemy*>enemy){ _enemies = enemy; }	 // エネミーセット
	void SetBullet(std::vector<Bullet*>bullet){ _bullets = bullet; } // 弾丸セット
	void SetCamera(Camera* cam) { _cam = cam; }					     // カメラセット

protected:
	VECTOR _vLightDir;		// 光源方向
	float _colSubY;			// コリジョン判定時のY補正(腰位置）
	int _handleMap;		    // マップモデルハンドル
	int _handleSkySphere;   // スカイボックスモデルハンドル
	int _frameMapCollision;	// マップコリジョンフレーム

	// デバッグ用
	bool _bViewCollision;

	Player* _player;
	Camera* _cam;
	std::vector<Enemy*>_enemies;
	std::vector<Bullet*>_bullets;
};

