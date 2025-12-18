#pragma once
#include "Character.h"

class Map;

// 弾の種類
enum class BULLET_TYPE
{
	PLAYER,     // プレイヤーの弾
	ENEMY       // 敵の弾
};

class Bullet : public Character
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画
	void MoveBullet();					// 弾丸の移動処理
	void MoveColBullet();				// マップと弾丸との当たり判定関数

	// 弾の初期化（発射者情報付き)
	bool Initialize(VECTOR startPos, VECTOR direction, float speed, BULLET_TYPE type);

	// 弾の無効化
	void Deactivate() { _isActive = false; }

	// ゲッター
	BULLET_TYPE GetBulletType() const { return _bulletType; }
	bool IsActive() const { return _isActive; }

	// クラスセット
	void SetMap(Map* map) { _map = map; }						   // マップセット

protected:
	BULLET_TYPE _bulletType;    // 弾の種類

	float _speed;               // 移動速度
	float _lifeTime;            // 弾の生存時間
	float _maxLifeTime;         // 最大生存時間
	bool _isActive;             // 弾が有効かどうか

	VECTOR _direction;          // 移動方向

	Map* _map;
};

