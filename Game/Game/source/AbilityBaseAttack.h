#pragma once
#include "Ability.h"

class ApplicationMain;
class ModeGame;
class Player;
class Enemy;
class Ability;
class AbilityBullet;
class AbilityMelee;
class AbilityAoE;
class AbilityShield;
class AbilityBaseAttack : public Ability
{
public:
    virtual bool Initialize() override;
    virtual bool Terminate() override;
    virtual bool Process() override;
    virtual bool Render() override;

	// プレイヤー攻撃処理
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

	void PlayerAttackProcess();						  // プレイヤー攻撃処理
	void CheckAttackHit();							  // 攻撃ヒット判定処理
	void CheckDeathEnemy();							  // エネミー死亡判定処理
	void AddAbility(std::unique_ptr<Ability>ability); // アビリティ追加メソッド
	void ProcessAbilities();						  // アビリティ処理メソッド

    //  デバッグ用関数
    void DrawAttackCollision();

	// クラスセット
	void SetModeGame(ModeGame* modeGame) { _modeGame = modeGame; } 
	void SetPlayer(Player* player) { _player = player; }
	void SetEnemy(std::vector<Enemy*>enemy){ _enemies = enemy; }   

protected:
	ModeGame* _modeGame;
	Player* _player;				   // プレイヤー
	std::vector<Enemy*> _enemies;	   // エネミーの配列
	std::vector<Enemy*>_hit_enemy;	   // 攻撃ヒットしたエネミーの配列
	std::vector<Enemy*>_death_enemies; // 死亡したエネミーの配列

};

