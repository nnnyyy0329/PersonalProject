#pragma once
#include "GameObjectBase.h"

class Character : public GameObjectBase
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画

	// 攻撃処理
	virtual void AttackMove
	(
		VECTOR col_top,		   // 攻撃コリジョン上端
		VECTOR col_bottom,	   // 攻撃コリジョン下端
		float col_r, 		   // 攻撃コリジョン半径
		float attack_delay,	   // 攻撃開始までの遅延時間
		float attack_duration, // 攻撃持続時間
		float recovery,		   // 攻撃後の後隙
		float damage,		   // ダメージ量
		bool is_attacking	   // 攻撃中フラグ
	);

	// 円形攻撃処理
	virtual void AttackMoveCircle
	(
		VECTOR circle_center,  // 円の中心位置
		float circle_radius,   // 円の半径  
		float circle_height,   // 円の高さ
		float attack_delay,    // 攻撃開始までの遅延時間
		float attack_duration, // 攻撃持続時間
		float recovery,		   // 攻撃後の後隙
		float damage,          // ダメージ量
		bool is_attacking      // 攻撃中フラグ
	);

	// シールド処理
	virtual void ShieldMove
	(
		VECTOR col_top,
		VECTOR col_bottom,
		float col_r,
		float damage,
		bool is_shielding
	);
	// シールド判定用の仮想関数
	virtual void CheckShieldHit() {}

	// スタン処理
	virtual void StunMove
	(
		float stun_time,	// スタン時間
		bool is_stunning	// スタン中フラグ
	){
	};

	virtual void UpdateAttackColPos(){};	// 攻撃コリジョン座標更新
	virtual void UpdateShieldColPos(){};	// シールドコリジョン座標更新

	// 能力用
	enum class AbilityEnum
	{
		NONE,
		MELEE,
		BULLET,
		AOE,
		SHIELD,
		_EOT_,
	};
	AbilityEnum _abilityEnum;

	// ゲッターセッター
	auto GetCooltime()const{ return _cooltime; }			// 無敵時間
	void SetCooltime(int cooltime){ _cooltime = cooltime; }	// 無敵時間設定

	float GetMoveSpeed()const{ return _moveSpeed; }					// 移動速度
	void SetMoveSpeed(float moveSpeed){ _moveSpeed = moveSpeed; }	// 移動速度設定

	float GetOldMoveSpeed()const{ return _old_move_speed; }						// 移動速度保存用
	void SetOldMoveSpeed(float oldMoveSpeed){ _old_move_speed = oldMoveSpeed; }	// 移動速度保存用設定

	float GetLife()const{ return _life; }		// 体力
	void SetLife(float life){ _life = life; }	// 体力設定

	float GetInvincibleTime()const { return _invincible_time; }							// 無敵時間
	void SetInvincibleTime(float invincibleTime) { _invincible_time = invincibleTime; } // 無敵時間設定

	bool GetIsDead()const{ return _is_dead; }			// 死亡フラグ
	void SetIsDead(bool isDead){ _is_dead = isDead; }	// 死亡フラグ設定

	bool GetHit()const{ return _hit; }		// ダメージヒットフラグ
	void SetHit(bool hit){ _hit = hit; }	// ダメージヒットフラグ設定

	float GetColR()const{ return _collision_r; }			// コリジョン半径
	void SetColR(float colR) { this->_collision_r = colR; }	// コリジョン半径設定

	float GetColWeight()const{ return _collision_weight; }						// コリジョン重さ
	void SetColWeight(float colWeight) { this->_collision_weight = colWeight; }	// コリジョン重さ設定

	VECTOR GetCollisionPos() const { return _collision_pos; }							// コリジョン位置
	void SetCollisionPos(const VECTOR collisionPos) { this->_collision_pos = collisionPos; } // コリジョン位置設定

	VECTOR GetCapsuleTop() const { return _capsule_top; }								// カプセルコリジョン上端
	void SetCapsuleTop(const VECTOR capsuleTop) { this->_capsule_top = capsuleTop; }	// カプセルコリジョン上端設定

	VECTOR GetCapsuleBottom() const { return _capsule_bottom; }										// カプセルコリジョン下端
	void SetCapsuleBottom(const VECTOR capsuleBottom) { this->_capsule_bottom = capsuleBottom; }	// カプセルコリジョン下端設定

	float GetCapsuleRadius() const { return _capsule_radius; }									// カプセルコリジョン半径
	void SetCapsuleRadius(const float capsuleRadius) { this->_capsule_radius = capsuleRadius; }	// カプセルコリジョン半径設定

	VECTOR GetOldPos() const { return _oldvPos; }						// 移動前のポジション
	void SetOldPos(const VECTOR oldvPos) { this->_oldvPos = oldvPos; }	// 移動前のポジション設定

	VECTOR GetV() const { return _v; }			// 移動ベクトル
	void SetV(const VECTOR v) { this->_v = v; }	// 移動ベクトル設定

	// 攻撃用円形コリジョン取得
	VECTOR GetAttackColTop() const { return _attack_col_top; }									// 攻撃コリジョン上端
	void SetAttackColTop(const VECTOR attackColTop) { this->_attack_col_top = attackColTop; }	// 攻撃コリジョン上端設定

	VECTOR GetAttackColBottom() const { return _attack_col_bottom; }										// 攻撃コリジョン下端
	void SetAttackColBottom(const VECTOR attackColBottom) { this->_attack_col_bottom = attackColBottom; }	// 攻撃コリジョン下端設定

	float GetAttackColR() const { return _attack_col_r; }								// 攻撃コリジョン半径
	void SetAttackColR(const float attackColR) { this->_attack_col_r = attackColR; }	// 攻撃コリジョン半径設定

	bool GetIsAttacking() const { return _is_attacking; }								// 攻撃中フラグ
	void SetIsAttacking(const bool isAttacking) { this->_is_attacking = isAttacking; }	// 攻撃中フラグ設定

	bool GetIsStartAttack() const { return _is_start_attack; }									// 攻撃開始フラグ
	void SetIsStartAttack(const bool isStartAttack) { this->_is_start_attack = isStartAttack; }	// 攻撃開始フラグ設定

	bool GetIsStartAoEAttack() const { return _is_start_aoe_attack; }											// 範囲攻撃開始フラグ
	void SetIsStartAoEAttack(const bool isStartAoEAttack) { this->_is_start_aoe_attack = isStartAoEAttack; }	// 範囲攻撃開始フラグ設定

	bool GetIsMeleeSuccess() const { return _is_melee_success; }								// メレー成功フラグ
	void SetIsMeleeSuccess(bool isMeleeSuccess){ this->_is_melee_success = isMeleeSuccess; }	// メレー成功フラグ設定

	// シールド用コリジョン取得
	VECTOR GetShieldColTop() const { return _shield_col_top; }									// シールドコリジョン上端
	void SetShieldColTop(const VECTOR shieldColTop) { this->_shield_col_top = shieldColTop; }	// シールドコリジョン上端設定

	VECTOR GetShieldColBottom() const { return _shield_col_bottom; }										// シールドコリジョン下端
	void SetShieldColBottom(const VECTOR shieldColBottom) { this->_shield_col_bottom = shieldColBottom; }	// シールドコリジョン下端設定

	float GetShieldColR() const { return _shield_col_r; }								// シールドコリジョン半径
	void SetShieldColR(const float shieldColR) { this->_shield_col_r = shieldColR; }	// シールドコリジョン半径設定

	bool GetIsShielding() const { return _is_shielding; }								// シールド中フラグ
	void SetIsShielding(const bool isShielding) { this->_is_shielding = isShielding; }	// シールド中フラグ設定

	bool GetIsShieldSuccess() const { return _is_shield_success; }										// シールドに成功したか
	void SetIsShieldSuccess(const bool isShieldSuccess) { this->_is_shield_success = isShieldSuccess; }	// シールドに成功したか設定

	bool GetIsShieldActive() const { return _is_shield_active; }									// シールドアクティブフラグ
	void SetIsShieldActive(const bool isShieldActive) { this->_is_shield_active = isShieldActive; }	// シールドアクティブフラグ設定

protected:

	// 基本ステータス
	int _cooltime;				// 無敵時間
	float _moveSpeed;			// 移動速度
	float _old_move_speed;		// 移動速度保存用
	float _life;				// 体力
	float _rotation_speed;		// 回転スピード
	float _invincible_time;		// 無敵時間
	bool _is_dead;				// 死亡フラグ
	bool _hit;					// ダメージヒットフラグ

	// 移動用
	VECTOR _oldvPos;			// 移動前のポジション
	VECTOR _v;					// 移動ベクトル

	// コリジョン用
	float	_collision_r;		// 円の半径
	float	_collision_weight;	// キャラの重さ
	VECTOR _collision_pos;		// コリジョン位置

	// 攻撃関連のメンバ変数
	CircleFloor _attack_circle;	// 攻撃用円形コリジョン
	VECTOR _attack_col_top;		// 攻撃コリジョン上端
	VECTOR _attack_col_bottom;	// 攻撃コリジョン下端
	float _attack_col_r;		// 攻撃コリジョン半径
	float _attack_delay;		// 攻撃開始までの遅延時間
	float _attack_duration;		// 攻撃持続時間
	float _attack_recovery;		// 攻撃後の後隙
	float _damage;				// ダメージ量
	float _attack_timer;		// 攻撃タイマー

	bool _is_attacking;         // 攻撃中フラグ		
	bool _is_start_attack;		// 攻撃開始フラグ
	bool _is_start_aoe_attack;  // 範囲攻撃開始フラグ
	bool _is_melee_success;		// メレー成功フラグ

	// シールド関連のメンバ変数
	VECTOR _shield_col_top;	    // シールドコリジョン上端
	VECTOR _shield_col_bottom;  // シールドコリジョン下端
	float _shield_col_r;	    // シールドコリジョン半径
	bool _is_shielding;         // シールド中フラグ
	bool _is_shield_success;	// シールドに成功したか
	bool _is_shield_active;		// シールドアクティブフラグ

	// カプセルコリジョン用
	VECTOR _capsule_top;	// カプセルコリジョン上端
	VECTOR _capsule_bottom; // カプセルコリジョン下端
	float _capsule_radius;  // カプセルコリジョン半径

};

