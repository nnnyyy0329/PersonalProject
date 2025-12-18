#pragma once
#include "Enemy.h"

// 前方宣言
class ModeGame;
class Player;

class MeleeEnemy : public Enemy
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画
	void DeathEnemy()override;			// 敵の死亡処理
	void EnemyStatusMove()override;		// 敵の行動に応じた状態
	void EnemyRotate();					// 敵の回転
	void AttackMove();					// 攻撃中の移動処理

	// 攻撃処理
	virtual void AttackMove
	(
		VECTOR col_top,
		VECTOR col_bottom,
		float col_r,
		float attack_delay,
		float attack_duration,
		float recovery,
		float damage,
		bool is_attacking
	)override;

	void AttackProcess();				// 攻撃処理
	void CheckAttackHit();				// 攻撃ヒット判定処理
	void UpdateAttackColPos()override;	// 攻撃コリジョン座標更新

	// デバッグ用関数
	void DrawEnemyCollision()override;	// 敵のコリジョン描画
	void DrawAttackCollision();			// 攻撃コリジョン描画
	void DrawStatus();					// ステータス表示関数
	void DrawAttackColPos();			// 攻撃コリジョン位置表示

	// クラス参照を設定
	void SetModeGame(ModeGame* modeGame) { _modeGame = modeGame; }
	void SetPlayer(Player* player) { _player = player; }

protected:
	ModeGame* _modeGame; // ModeGameの参照
	Player* _player;     // プレイヤーの参照

	// 公転運動用変数
	VECTOR _orbit_center;     // 公転の中心点
	VECTOR _initial_position; // 初期座標
	VECTOR _orbit_offset;     // 公転中心のオフセット
	float _orbit_radius;      // 公転半径
	float _orbit_angle;       // 公転角度
	float _orbit_speed;       // 公転速度
	bool _orbit_initialized;  // 公転初期化フラグ
};