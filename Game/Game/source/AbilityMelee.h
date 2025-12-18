#pragma once
#include "Ability.h"

class ApplicationMain;
class Player;

class AbilityMelee : public Ability
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画

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

	void MeleeProcess();				// メレー攻撃処理
	void RotationProcess();				// 回転処理

	// デバッグ用関数
	void DrawAttackCollision();			// 攻撃コリジョン描画

	// クラスセット
	void SetPlayer(Player* player) { _player = player; }

protected:

	Player* _player;
};

