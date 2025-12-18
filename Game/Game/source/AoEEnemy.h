#pragma once
#include "Enemy.h"

// 前方宣言
class ModeGame;
class Player;

class AoEEnemy : public Enemy
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画
	void DeathEnemy()override;			// 敵の死亡処理
	void EnemyStatusMove()override;		// 敵の行動に応じた状態

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

	void EnemyRotate();    // 敵の回転
	void CheisePlayer();   // プレイヤーを追尾する
	void ProcessAttack();  // 攻撃処理
	void CheckAttackHit(); // 攻撃ヒット判定処理

	// 円形床判定用メソッド
	void CheckCircularFloorCol();

	// デバッグ用関数
	void DrawEnemyCollision()override; // 敵のコリジョン描画
	void DrawCircleFloor();			   // 円形床描画
	void DrawAttackCircleFloor();	   // 攻撃範囲円形床描画
	void DrawAoEAttackRange();		   // 範囲攻撃範囲描画
	void DrawEnemyStatus();			   // 敵のステータス表示関数

	// クラス参照を設定
	void SetModeGame(ModeGame* modeGame) { _modeGame = modeGame; }
	void SetPlayer(Player* player) { _player = player; }

protected:
	bool _cheise_player;	   // プレイヤー追尾フラグ

	ModeGame* _modeGame; // ModeGameの参照
	Player* _player;     // プレイヤーの参照

	CircleFloor _circleFloor; // 円形床判定
};

