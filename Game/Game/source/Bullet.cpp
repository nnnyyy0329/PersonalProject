#include "Bullet.h"
#include "map.h"

bool Bullet::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if (!Character::Initialize()) { return false; }

	_bulletType = BULLET_TYPE::ENEMY;  // デフォルトの弾

	_direction = VGet(0, 0, -1);	   // デフォルトの移動方向

	_collision_r = 5.0f;			   // 弾の当たり判定半径
	_speed = 10.0f;					   // デフォルトの移動速度
	_lifeTime = 0.0f;				   // 生存時間初期化
	_maxLifeTime = 5.0f;			   // 5秒で消滅
	_isActive = false;				   // デフォルトは無効

	return true;
}

bool Bullet::Terminate()
{
	return true;
}

bool Bullet::Process()
{
	if (!_isActive) return true;

	MoveBullet();

	MoveColBullet();

	return true;
}

bool Bullet::Render()
{
	if (!_isActive) return true;

	// 弾の色設定
	int color = 
		(_bulletType == BULLET_TYPE::PLAYER) ? GetColor(0, 255, 0) : GetColor(255, 0, 0);

	// 弾の描画
	DrawSphere3D(_vPos, _collision_r, 8, color, color, TRUE);

	return true;	
}

// 弾の初期化（発射者情報付き)
bool Bullet::Initialize(VECTOR startPos, VECTOR direction, float speed, BULLET_TYPE type)
{
	_bulletType = type;			   // 弾の種類設定

	_vPos = startPos;			   // 初期位置設定
	_direction = VNorm(direction); // 移動方向設定
								   
	_speed = speed;				   // 移動速度設定
	_lifeTime = 0.0f;			   // 生存時間初期化
	_isActive = true;			   // 有効化

	return true;
}

// 弾丸の移動処理
void Bullet::MoveBullet()
{
	// 移動処理
	_v = VScale(_direction, _speed);
	VECTOR vNormalize = VNorm(_v);
	_vPos = VAdd(_vPos, _v);

	// 生存時間の更新
	_lifeTime += 1.0f / 60.0f;
	if(_lifeTime >= _maxLifeTime)
	{
		// 生存時間が最大を超えたら無効化
		_isActive = false;
	}
}

// マップとの当たり判定時の弾丸移動
void Bullet:: MoveColBullet()
{
	// 攻撃中は位置更新をスキップ
	if(!_isActive) { return; }

	// マップステータス
	int map_handle = _map->GetHandleMap();
	int map_frameMapCol = _map->GetFrameMapCol();
	float map_colSubY = _map->GetColSubY();

	// 移動した先でコリジョン判定
	MV1_COLL_RESULT_POLY hitPoly;

	// 主人公の腰位置から下方向への直線
	hitPoly = MV1CollCheck_Line(map_handle, map_frameMapCol,
		VAdd(_vPos, VGet(0, map_colSubY, 0)),
		VAdd(_vPos, VGet(0, -99999.f, 0)));

	// 当たったかどうか
	if(!hitPoly.HitFlag) // 当たった
	{
		// 非アクティブにする
		_isActive = false;
	}
}
