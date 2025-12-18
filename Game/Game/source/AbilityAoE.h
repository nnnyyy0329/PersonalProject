#pragma once
#include "Ability.h"

class ApplicationMain;
class Enemy;
class ModeGame;
class Player;

class AbilityAoE : public Ability
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画

	// 円形攻撃処理
	virtual void AttackMoveCircle
	(
		VECTOR circle_center,    // 円の中心位置
		float circle_radius,     // 円の半径  
		float circle_height,     // 円の高さ
		float attack_delay,      // 攻撃開始までの遅延時間
		float attack_duration,   // 攻撃持続時間
		float recovery,          // 攻撃後の後隙
		float damage,            // ダメージ量
		bool is_attacking        // 攻撃中フラグ
	)override;

	void ProcessAttack();  // 攻撃処理
	void CheckAttackHit(); // 攻撃ヒット判定処理

	// デバッグ用関数
	void DrawAoEAttackRange(); // 範囲攻撃範囲描画

	// クラスセット
	void SetEnemy(std::vector<Enemy*>enemy){ _enemies = enemy; } // エネミーセット
	void SetPlayer(Player* player) { _player = player; }		 // プレイヤーセット

protected:
	float _aoe_attack_damage; // 範囲攻撃ダメージ

	Player* _player;

	std::vector<Enemy*> _enemies;  // エネミーの配列
	std::vector<Enemy*>_hit_enemy; // 攻撃ヒットしたエネミーの配列

	CircleFloor _circleFloor;	   // 円形床判定
};

