#include "AbilityShield.h"
#include "ApplicationMain.h"
#include "Player.h"

bool AbilityShield::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if(!Ability::Initialize()) { return false; }

	_player = nullptr;

	return true;
}

bool AbilityShield::Terminate()
{
	return true;
}

bool AbilityShield::Process()
{
	// シールド処理
	ShieldProcess();

	// シールドコリジョン位置更新
	UpdateShieldColPos();

	return true;
}

bool AbilityShield::Render()
{
	// シールドコリジョン描画
	DrawShieldCollision();

	// コリジョン位置描画
	//DrawColPosition();

	// プレイヤースピード
	//DrawFormatString(0, 380, GetColor(0, 255, 0), " MOVE SPEED : %f", _old_move_speed);

	return true;
}

// シールド処理
void AbilityShield::ShieldMove
(
	VECTOR col_top,
	VECTOR col_bottom,
	float col_r,
	float damage,
	bool is_shielding
)
{
	// 向きを正規化
	VECTOR dir_norm = VNorm(_player->GetDirection());

	// コリジョンを前方に配置
	VECTOR shield_offset = VScale(dir_norm, 40.0f);

	// コリジョン位置
	VECTOR base_pos = _vPos;
	VECTOR col_offset = VAdd(base_pos, shield_offset);

	// シールドコリジョン情報を保存
	_shield_col_top = VAdd(col_offset, col_top);	   
	_shield_col_bottom = VAdd(col_offset, col_bottom); 
	_shield_col_r = col_r;
	_damage = damage;
	_is_shielding = is_shielding;
}

// シールド処理
void AbilityShield::ShieldProcess()
{
	if(_player == nullptr){ return; }

	int key = ApplicationMain::GetInstance()->GetKey();

	if(key & PAD_INPUT_5)
	{
		// 他のアビリティが動作中ならシールドを発動しない
		if(_player->IsAnyAbilityActive() && _player->GetPlayerStatus() != Player::PLAYER_STATUS::SHIELD)
		{
			return;
		}

		// シールド開始
		if(!_is_shield_active)
		{
			_old_move_speed = _player->GetMoveSpeed();				// 移動速度を保存
			_player->SetMoveSpeed(_player->GetMoveSpeed() * 0.5);	// シールド中は移動速度を75%に低下
			_is_shield_active = true;								// シールドアクティブフラグを立てる
		}

		// シールド開始
		_player->SetPlayerStatus(Player::PLAYER_STATUS::SHIELD);
		_is_shielding = true;

		// シールドコリジョンを設定
		ShieldMove
		(
			VGet(0.0f, 70.0f, 0.0f),  // シールドコリジョン上端
			VGet(0.0f, 40.0f, 0.0f),  // シールドコリジョン下端
			20.0f,                    // シールドコリジョン半径
			1.0f,                     // ダメージ量
			true					  // シールド中フラグ
		);
	}
	else // シールドボタンが離された場合
	{
		if(_is_shield_active)
		{
			_player->SetMoveSpeed(_old_move_speed);					// 移動速度を元に戻す
			_player->SetPlayerStatus(Player::PLAYER_STATUS::WAIT);	// プレイヤーステータスを待機に戻す
			_is_shield_active = false;								// シールドアクティブフラグを下ろす

			// シールドコリジョン情報をリセット
			_shield_col_top = VGet(0, 0, 0);
			_shield_col_bottom = VGet(0, 0, 0);
			_shield_col_r = 0.0f;
		}

		_is_shielding = false;
	}
}

// シールドコリジョン位置更新
void AbilityShield::UpdateShieldColPos()
{
	// シールドコリジョンの中心位置を更新
	VECTOR center_pos=VGet
	(
		(_shield_col_top.x + _shield_col_bottom.x) / 2.0f,
		(_shield_col_top.y + _shield_col_bottom.y) / 2.0f,
		(_shield_col_top.z + _shield_col_bottom.z) / 2.0f
	);

	_collision_pos = center_pos;
}

// デバッグ表示
void AbilityShield::DrawShieldCollision()
{
	// シールド中のみ描画
	if(_is_shielding)
	{
		// シールドコリジョン描画
		DrawCapsule3D
		(
			_shield_col_top,
			_shield_col_bottom,
			_shield_col_r,
			8,
			GetColor(0, 0, 255),
			GetColor(255, 0, 0),
			FALSE
		);
	}
}

// コリジョン位置描画
void AbilityShield::DrawColPosition()
{
	// シールドコリジョン上端
	DrawFormatString(
		10, 300,
		GetColor(0, 255, 0),
		"Shield Col Top: (%.2f, %.2f, %.2f)",
		_shield_col_top.x,
		_shield_col_top.y,
		_shield_col_top.z
	);
	// シールドコリジョン下端
	DrawFormatString(
		10, 320,
		GetColor(0, 255, 0),
		"Shield Col Bottom: (%.2f, %.2f, %.2f)",
		_shield_col_bottom.x,
		_shield_col_bottom.y,
		_shield_col_bottom.z
	);
	// シールドコリジョン座標
	DrawFormatString(
		10, 340,
		GetColor(0, 255, 0),
		"ShieldPos: %.2f, %.2f, %.2f",
		_collision_pos.x,
		_collision_pos.y,
		_collision_pos.z
	);
}