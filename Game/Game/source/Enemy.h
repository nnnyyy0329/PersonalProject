#pragma once
#include "Character.h"

class Bullet;

class Enemy : public Character
{
public:
	virtual bool Initialize() override;		// 初期化
	virtual bool Terminate() override;		// 終了
	virtual bool Process() override;		// 更新
	virtual bool Render() override;			// 描画
	virtual void DamageEnemy(float damage); // ダメージ処理
	virtual void EnemyStatusMove();			// 敵の行動
	virtual void DeathEnemy();				// 敵の死亡処理

	// デバッグ用関数
	virtual void DrawEnemyCollision(); // 敵のコリジョン描画

	// 状態の列挙型
	enum class ENEMY_STATUS
	{
		NONE,   // 無状態
		WAIT,   // 待機
		WALK,   // 歩行
		ATTACK, // 攻撃
		SHIELD, // シールド
		DEATH,  // 死亡
		_EOT_,  // 終端
	};
	ENEMY_STATUS _status; // 状態

	// ゲッターセッター
	ENEMY_STATUS GetEnemyStatus() const { return _status; }
	void SetEnemyStatus(ENEMY_STATUS status) { _status = status; }

	AbilityEnum GetEnemyAbility() const { return _enemy_ability; } // 敵の能力取得
	void SetEnemyAbility(AbilityEnum ability) { _enemy_ability = ability; } // 敵の能力設定
	
protected:
	float _enemy_rad;	 // 敵の向き角度
	float _enemy_length; // 敵の移動距離
	bool _is_moved;		 // 移動フラグ

	Bullet* _bullet;		// 弾クラスへのポインタ
	AbilityEnum _enemy_ability; // 敵の能力
};

