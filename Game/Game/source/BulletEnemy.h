#pragma once
#include "Enemy.h"

// 前方宣言
class ModeGame; 
class Player;   

class BulletEnemy : public Enemy
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画
	void DeathEnemy()override;			// 敵の死亡処理
	void EnemyStatusMove()override;		// 敵の行動
	void EnemyRotate();					// 敵の回転
	void ShotBullet();					// 弾を撃つ

	// デバッグ用関数
	void DrawEnemyCollision()override; // 敵のコリジョン描画

	// クラス参照を設定
	void SetModeGame(ModeGame* modeGame) { _modeGame = modeGame; }
	void SetPlayer(Player* player) { _player = player; }

protected:
	float _shotcnt; // 射撃カウント

	ModeGame* _modeGame; // ModeGameの参照
	Player* _player;     // プレイヤーの参照
};

