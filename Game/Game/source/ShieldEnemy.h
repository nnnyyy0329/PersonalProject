#pragma once
#include "Enemy.h"

// 前方宣言
class ModeGame;
class Player;

class ShieldEnemy : public Enemy
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画
	void DeathEnemy()override;			// 敵の死亡処理
	void EnemyStatusMove()override;		// 敵の行動に応じた状態
	void EnemyRotate();					// 敵の回転

	virtual void ShieldMove
	(
		VECTOR col_top,
		VECTOR col_bottom,
		float col_r,
		float damage,
		bool is_sheilding
	)override;

	void ProcessShield();			// シールド処理
	void CheckShieldHit() override; // シールドヒット判定処理
	void MoveStop();				// 移動停止処理
	void CheisePlayer();			// プレイヤーを追尾する

	// デバッグ用関数
	void DrawEnemyCollision()override;	// 敵のコリジョン描画
	void DrawShieldCollision();			// シールドコリジョン描画
	void DrawStatus();					// ステータス表示関数
	void DrawCircleFloor();				// 円形床描画

	// クラス参照を設定
	void SetModeGame(ModeGame* modeGame) { _modeGame = modeGame; }
	void SetPlayer(Player* player) { _player = player; }

protected:
	CircleFloor _circleFloor;	// 円形床判定

	ModeGame* _modeGame;		// ModeGameの参照
	Player* _player;			// プレイヤーの参照
};

