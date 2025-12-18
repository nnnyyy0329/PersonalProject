#pragma once
#include "Ability.h"

class ApplicationMain;
class Player;

class AbilityShield : public Ability
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画

	virtual void ShieldMove
	(
		VECTOR col_top,
		VECTOR col_bottom,
		float col_r,
		float damage,
		bool is_shielding
	) override;

	void ShieldProcess();				// シールド処理
	void UpdateShieldColPos()override;	// シールドコリジョン位置更新

	// デバッグ表示
	void DrawShieldCollision(); // シールドコリジョン描画
	void DrawColPosition();		// コリジョン位置描画

	// ゲッターセッター

	// クラスセット
	void SetPlayer(Player* player) { _player = player; }

protected:

	Player* _player;
};