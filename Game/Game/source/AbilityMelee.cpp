#include "AbilityMelee.h"
#include "ApplicationMain.h"
#include "Player.h"

bool AbilityMelee::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if(!Ability::Initialize()) { return false; }

	_rotation_speed = 0.1;

	_player = nullptr;

	return true;
}

bool AbilityMelee::Terminate()
{
	return true;
}

bool AbilityMelee::Process()
{
	// メレー処理
	MeleeProcess();

	// 回転処理
	RotationProcess();

	return true;
}

bool AbilityMelee::Render()
{
	// 攻撃コリジョン表示
	DrawAttackCollision();

	return true;
}

// 近接攻撃エネミー攻撃処理
void AbilityMelee::AttackMove
(
	VECTOR col_top,		   // 攻撃コリジョン上端
	VECTOR col_bottom,	   // 攻撃コリジョン下端
	float col_r,		   // 攻撃コリジョン半径
	float attack_delay,	   // 攻撃開始までの遅延時間
	float attack_duration, // 攻撃持続時間
	float recovery,		   // 攻撃後の後隙
	float damage,		   // ダメージ量
	bool is_attacking	   // 攻撃中フラグ
)
{
	// 向きを正規化
	VECTOR dir_norm = VNorm(_player->GetDirection());

	// 攻撃コリジョンを前方に配置
	VECTOR attack_offset = VScale(dir_norm, 100.0f);

	// コリジョン位置
	VECTOR col_offset = VAdd(_player->GetPosition(), attack_offset);

	// 攻撃コリジョンをセット-----------------------------
	_attack_col_top = VAdd(col_offset, col_top);		// 攻撃コリジョン上端
	_attack_col_bottom = VAdd(col_offset, col_bottom);	// 攻撃コリジョン下端
	_attack_col_r = col_r;								// 攻撃コリジョン半径
	_attack_delay = attack_delay;						// 攻撃開始までの遅延時間
	_attack_duration = attack_duration;					// 攻撃持続時間
	_attack_recovery = recovery;						// 攻撃後の後隙
	_damage = damage;									// ダメージ量
}

// メレー処理
void AbilityMelee::MeleeProcess()
{
	if(_player == nullptr) { return; }

	int key = ApplicationMain::GetInstance()->GetKey();

	// プレイヤーのステータス
	Player::PLAYER_STATUS player_status = _player->GetPlayerStatus();

	// 範囲攻撃開始フラグが立っていて、ステータスが攻撃でないなら
	if(key & PAD_INPUT_6)
	{
		// 他のアビリティが動作中なら範囲攻撃を発動しない
		if(_player->IsAnyAbilityActive() && player_status != Player::PLAYER_STATUS::MELEE)
		{
			return;
		}

		if(player_status != Player::PLAYER_STATUS::MELEE)
		{
			player_status = Player::PLAYER_STATUS::MELEE;	// ステータスを範囲攻撃に変更
			_player->SetPlayerStatus(player_status);		// プレイヤーステータス設定
			_attack_timer = 0.0f;
		}

		if(_player->GetPlayerStatus() == Player::PLAYER_STATUS::MELEE)	// 攻撃状態中
		{
			_attack_timer += 15.0f; // 攻撃タイマーを進める

			// 攻撃コリジョンをセット
			AttackMove
			(
				VGet(0.0f, 50.0f, 0.0f),  // 攻撃コリジョン上端
				VGet(0.0f, 10.0f, 0.0f),  // 攻撃コリジョン下端
				20.0f,                    // 攻撃コリジョン半径
				15.0f,					  // 攻撃開始までの遅延時間
				15.0f,					  // 攻撃持続時間
				0.0f,					  // 攻撃後の後隙
				1.0f,                     // ダメージ量
				false					  // 攻撃中フラグ
			);

			// 攻撃開始
			if(_attack_timer >= _attack_delay && _attack_timer < _attack_delay + _attack_duration)
			{
				_is_attacking = true; // 攻撃中フラグを立てる
			}
			else
			{
				_is_attacking = false; // 攻撃外では攻撃中フラグを下ろす
			}

			// 攻撃終了
			if(_attack_timer >= _attack_delay + _attack_duration)
			{
				_attack_timer = 0.f;									// 攻撃タイマーを初期化
			}
		}
	}
	else // キーが離された場合
	{
		if(_player->GetPlayerStatus() == Player::PLAYER_STATUS::MELEE)
		{
			_player->SetPlayerStatus(Player::PLAYER_STATUS::WAIT);	// ステータスを待機に戻す
			_is_attacking = false;									// 攻撃中フラグを下ろす
		}
	}
}

// 回転処理
void AbilityMelee::RotationProcess()
{
	if(_player->GetPlayerStatus() == Player::PLAYER_STATUS::MELEE)
	{
		// プレイヤーの方向取得
		VECTOR player_dir = _player->GetDirection();

		// 現在の向きから角度を計算
		float currentAngle = atan2(player_dir.x, player_dir.z);

		// 角度更新
		currentAngle += _rotation_speed;

		player_dir.x = sin(currentAngle);
		player_dir.y = 0.0f;
		player_dir.z = cos(currentAngle);

		player_dir = VNorm(player_dir);	// 方向更新

		_player->SetDirection(player_dir);
	}
}

// 攻撃コリジョン描画
void AbilityMelee::DrawAttackCollision()
{
	if(
		_is_attacking && 
		_player != nullptr &&
		_player->GetPlayerStatus() == Player::PLAYER_STATUS::MELEE
		)
	{
		// 攻撃コリジョンを描画
		DrawCapsule3D
		(
			_attack_col_top,		 // カプセルコリジョン上端
			_attack_col_bottom,		 // カプセルコリジョン下端
			_attack_col_r,			 // カプセルコリジョン半径
			8,						 // 分割数
			GetColor(255, 0, 0),     // 赤色
			GetColor(255, 255, 255), // 白色
			FALSE					 // 塗りつぶし
		);
	}
}
