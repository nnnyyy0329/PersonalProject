#include "Character.h"

bool Character::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if (!GameObjectBase::Initialize()) { return false; }

	_cooltime = 0;				// 無敵時間初期化
	_moveSpeed = 0.0f;			// 移動速度初期化
	_old_move_speed = 0.0f;		// 移動速度保存用初期化
	_life = 0.0f;				// 体力初期化
	_invincible_time = 0.0f;	// 無敵時間初期化
	_is_dead = false;			// 死亡フラグ初期化
	_hit = false;				// ヒットフラグ初期化

	// コリジョン用
	_collision_r = 0.0f;
	_collision_weight = 0.0f;
	_collision_pos = VGet(0.0f, 0.0f, 0.0f);

	// 攻撃用コリジョン
	{
		_attack_col_top = VGet(0.0f, 0.0f, 0.0f);
		_attack_col_bottom = VGet(0.0f, 0.0f, 0.0f);
		_attack_col_r = 0.0f;
		_attack_delay = 0.0f;
		_attack_duration = 0.0f;
		_attack_recovery = 0.0f;
		_damage = 0.0f;
		_rotation_speed = 0.0f;
		_is_attacking = false;
		_is_start_attack = false;
		_is_start_aoe_attack = false;
		_is_melee_success = false;

		_attack_timer = 0.0f;
	}

	// シールド用コリジョン
	{
		_shield_col_top = VGet(0.0f, 0.0f, 0.0f);
		_shield_col_bottom = VGet(0.0f, 0.0f, 0.0f);
		_shield_col_r = 0.0f;
		_is_shielding = false;
		_is_shield_success = false;
		_is_shield_active = false;
	}

	// カプセルコリジョン用
	{
		_capsule_top = VGet(0.0f, 0.0f, 0.0f);
		_capsule_bottom = VGet(0.0f, 0.0f, 0.0f);
		_capsule_radius = 0.0f;
	}

	// 初期化
	_oldvPos = VGet(0, 0, 0);
	_v = VGet(0, 0, 0);

	// 能力初期化
	_abilityEnum = AbilityEnum::NONE;

	return true;
}

bool Character::Terminate()
{
	return true;
}

bool Character::Process()
{
	// カプセルコリジョン座標更新をラムダ式で実装
	auto updateCapsuleCollision = [this]() 
	{
		_capsule_top = VGet(_vPos.x, _vPos.y + _capsule_top.y, _vPos.z);      // カプセルコリジョン上端更新
		_capsule_bottom = VGet(_vPos.x, _vPos.y + _capsule_bottom.y, _vPos.z); // カプセルコリジョン下端更新
	};

	return true;
}

bool Character::Render()
{
	return true;
}

// 攻撃処理
void Character::AttackMove
(
	VECTOR col_top,		   // 攻撃コリジョン上端
	VECTOR col_bottom, 	   // 攻撃コリジョン下端 
	float col_r,		   // 攻撃コリジョン半径
	float attack_delay,    // 攻撃開始までの遅延時間
	float attack_duration, // 攻撃持続時間
	float recovery,		   // 攻撃後の後隙
	float damage,		   // ダメージ量
	bool is_attacking	   // 攻撃中フラグ
)
{
	// 攻撃コリジョンをセット
	_attack_col_top = VAdd(_vPos, col_top);
	_attack_col_bottom = VAdd(_vPos, col_bottom);
	_attack_col_r = col_r;
	_attack_delay = attack_delay;
	_attack_duration = attack_duration;
	_attack_recovery = recovery;
	_damage = damage;
	_is_attacking = is_attacking;
}

// 円形攻撃処理
void Character::AttackMoveCircle
(
	VECTOR circle_center,    // 円の中心位置
	float circle_radius,     // 円の半径  
	float circle_height,     // 円の高さ
	float attack_delay,      // 攻撃開始までの遅延時間
	float attack_duration,   // 攻撃持続時間
	float recovery,          // 攻撃後の後隙
	float damage,            // ダメージ量
	bool is_attacking        // 攻撃中フラグ
)
{
	// 攻撃円形コリジョンを設定
	_attack_circle.center = circle_center;  // 円の中心位置
	_attack_circle.radius = circle_radius;  // 円の半径
	_attack_circle.height = circle_height;  // 円の高さ
	_attack_delay = attack_delay;           // 攻撃開始までの遅延時間
	_attack_duration = attack_duration;     // 攻撃持続時間
	_damage = damage;                       // ダメージ量
	_is_attacking = is_attacking;           // 攻撃中フラグ
}

// シールド処理
void Character::ShieldMove
(
	VECTOR col_top,
	VECTOR col_bottom,
	float col_r,
	float damage,
	bool is_sheilding
)
{
	// シールドコリジョン情報を保存
	_attack_col_top = col_top;
	_attack_col_bottom = col_bottom;
	_attack_col_r = col_r;
	_damage = damage;
	_is_attacking = is_sheilding;
}