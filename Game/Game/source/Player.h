#pragma once
#include "Character.h"

class ApplicationMain;
class ModeGame;
class Enemy;
class ShieldEnemy;
class Ability;
//class AbilityBaseAttack;
class AbilityBullet;
class AbilityMelee;
class AbilityAoE;
class AbilityShield;
class Camera;
class Map;

class Player : public Character
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画

	void MovePlayer();		 // プレイヤー移動
	void MovePlayerRotate(); // プレイヤーの移動に応じた回転
	void MoveColPlayer();	 // マップとの当たり判定時のプレイヤー移動
	void MovePlayerStatus(); // プレイヤーの状態移動
	void DamagePlayer();	 // プレイヤーダメージ処理

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

	void PlayerAttackProcess();							// プレイヤー攻撃処理
	void CheckAttackHit();								// 攻撃ヒット判定処理
	void CheckDeathEnemy();								// エネミー死亡判定処理
	void AddAbility(std::unique_ptr<Ability>ability);	// アビリティ追加メソッド
	void ProcessAbilities();							// アビリティ処理メソッド
	void DecrementTime();								// 時間減少処理

	// デバッグ用関数
	void DrawPlayerCapsuleCol();	// カプセルコリジョン描画
	void DrawAttackCollision();		// 攻撃コリジョン描画
	void DrawGetAbility();			// 能力取得描画
	void DrawPlayerStatus();		// プレイヤーのステータス表示関数
	void DrawCooltime();			// クールタイム描画
	void DrawOther();				// その他デバッグ情報描画

	// 特定のアビリティを取得
	template<typename T>
	T* GetAbility();

	// いずれかのアビリティがアクティブかどうかをチェック
	bool IsAnyAbilityActive() const;

	// 状態の列挙型
	enum class PLAYER_STATUS
	{
		NONE,   // 無状態
		WAIT,   // 待機
		WALK,   // 歩行
		ATTACK, // 攻撃
		SHOT,   // 射撃
		MELEE,	// メレー
		AOE,    // 範囲攻撃
		SHIELD, // シールド
		DEATH,  // 死亡
		_EOT_,  // 終端
	};

	// ゲッターセッター
	PLAYER_STATUS GetPlayerStatus() const { return _status; }
	void SetPlayerStatus(PLAYER_STATUS status) { _status = status; }

	VECTOR GetOldV() const { return _oldV; }
	void SetOldV(const VECTOR oldV){ this->_oldV = oldV; }

	float GetPlayerRad() const { return _player_rad; }
	void SetPlayerRad(const float rad){ this->_player_rad = rad; }

	float GetPlayerLength() const { return _player_length; }
	void SetPlayerLength(const float length){ this->_player_length = length; }

	float GetPlayerCamRad() const { return _player_camrad; }
	void SetPlayerCamRad(const float camrad){ this->_player_camrad = camrad; }

	float GetMeleeColR() const { return _melee_col_r; }
	void SetMeleeColR(const float col_r){ this->_melee_col_r = col_r; }

	const std::vector<std::unique_ptr<Ability>>& GetPlayerAbilities() const { return _player_abilities; }

	// クラスセット
	void SetModeGame(ModeGame* modeGame) { _modeGame = modeGame; } // モードゲームセット
	void SetEnemy(std::vector<Enemy*>enemy){ _enemies = enemy; }   // エネミーセット
	void SetCamera(Camera* cam) { _cam = cam; }					   // カメラセット
	void SetMap(Map* map) { _map = map; }						   // マップセット
	void SetAbility(std::vector<AbilityEnum>ability_enumes){ _ability_enumes = ability_enumes; } // プレイヤーの能力配列セット

protected:

	PLAYER_STATUS _status; // 状態
	PLAYER_STATUS oldStatus;

	int _player_key;  // 現在のキー入力
	int _player_trg;  // 現在のトリガー入力

	float _player_rad;		// プレイヤーの向き角度
	float _player_length;	// プレイヤーの移動距離
	float _player_camrad;	// カメラから見たプレイヤーの角度
	float _melee_col_r;		// 近接攻撃コリジョン半径

	VECTOR _oldV;

	ModeGame* _modeGame;
	Camera* _cam;
	Map* _map;

	std::vector<Enemy*> _enemies;							 // エネミーの配列
	std::vector<Enemy*>_hit_enemy;							 // 攻撃ヒットしたエネミーの配列
	std::vector<Enemy*>_death_enemies;						 // 死亡したエネミーの配列
	std::vector<std::unique_ptr<Ability>>_player_abilities;  // プレイヤーのアビリティ配列
	std::vector<AbilityEnum>_ability_enumes;				 // プレイヤーの能力配列
};

// 特定のアビリティ取得
template<typename T> 
T* Player::GetAbility() // T型のアビリティポインタを返す
{
	// プレイヤーのアビリティ配列を走査
	for(auto& ability : _player_abilities)
	{
		// 動的キャストで特定のアビリティ型に変換を試みる
		if(auto specific_ability = dynamic_cast<T*>(ability.get())) // T型にキャストできた場合
		{
			return specific_ability; // T型のアビリティポインタを返す
		}
	}
	return nullptr; // 見つからなかった場合はnullptrを返す
}