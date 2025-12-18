#pragma once
#include "Ability.h"

class ApplicationMain;
class ModeGame;
class Player;

class AbilityBullet : public Ability
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画
	void ShotBullet();					// 弾の発射処理
		
	// クラスセット
	void SetModeGame(ModeGame* modeGame) { _modeGame = modeGame; }
	void SetPlayer(Player* player) { _player = player; }

protected:
	float _shotcnt;

	ModeGame* _modeGame;
	Player* _player;
};

