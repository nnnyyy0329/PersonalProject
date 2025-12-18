#include "Enemy.h"

bool Enemy::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if (!Character::Initialize()) { return false; }
	
	// ステータスを「無し」に設定
	_status = ENEMY_STATUS::NONE;

	// 位置,向きの初期化
	_vPos = VGet(0, 0, 0);
	_vDir = VGet(0, 0, -1);

	// 初期化
	_collision_r = 30.0f;
	_collision_weight = 5.0f;
	_is_moved = false;

	// カプセルコリジョン初期化
	_capsule_top = VGet(0.0f, 0.0f, 0.0f);
	_capsule_bottom = VGet(0.0f, 0.0f, 0.0f);
	_capsule_radius = 0.0f;

	// 敵の能力初期化
	_enemy_ability = AbilityEnum::NONE; // 初期化

	return true;
}

bool Enemy::Terminate()
{
	// モデルを削除する
	MV1DeleteModel(_handle);

	return true;
}

bool Enemy::Process()
{
	return true;
}

bool Enemy::Render()
{
	return true;
}

// ダメージ処理
void Enemy::DamageEnemy(float damage)
{
	// 体力を減らす
	_life -= damage;
}

// 移動状態の更新
void Enemy::EnemyStatusMove()
{
	// 現在の位置と前回の位置を比較
	float distance = VSquareSize(VSub(_vPos, _oldvPos));

	// 移動判定
	const float movement = 0.0f;

	if(distance > movement)
	{
		_is_moved = true;
	}
	else
	{
		_is_moved = false;
	}

	// 前回の位置を更新
	_oldvPos = _vPos;
}

// 敵の死亡処理
void Enemy::DeathEnemy()
{
	// 体力が0以下なら死亡処理
	if(_life <= 0.0f)
	{
		_status = ENEMY_STATUS::DEATH; // ステータスを死亡に変更
	}
}

// 敵のコリジョン描画
void Enemy::DrawEnemyCollision()
{
	// カプセルコリジョン描画
	DrawCapsule3D
	(
		_capsule_top,
		_capsule_bottom,
		_capsule_radius,
		8,
		GetColor(25, 0, 0),   // 赤色
		GetColor(5, 55, 255), // 白色
		FALSE                // 塗りつぶし
	);
}