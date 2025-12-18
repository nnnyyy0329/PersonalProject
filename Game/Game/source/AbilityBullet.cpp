#include "AbilityBullet.h"
#include "ApplicationMain.h"
#include "Bullet.h"
#include "ModeGame.h"
#include "Player.h"

bool AbilityBullet::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if(!Ability::Initialize()) { return false; }

	_shotcnt = 0.0f;

	// クラスインスタンス初期化
	_bullet = nullptr;
	_modeGame = nullptr;

	return true;
}

bool AbilityBullet::Terminate()
{
	return true;
}

bool AbilityBullet::Process()
{
	// 弾発射処理
	ShotBullet();

	return true;
}

bool AbilityBullet::Render()
{
	return true;
}

// 弾の発射処理
void AbilityBullet::ShotBullet()
{
	int key = ApplicationMain::GetInstance()->GetKey();

	// プレイヤーのステータス
	Player::PLAYER_STATUS player_status = _player->GetPlayerStatus();

	if(key & PAD_INPUT_2)
	{
		// 他のアビリティが動作中なら通常攻撃を発動しない
		if(_player->IsAnyAbilityActive() && player_status != Player::PLAYER_STATUS::SHOT)
		{
			return;
		}

		player_status = Player::PLAYER_STATUS::SHOT; // ステータスを射撃に変更
		_player->SetPlayerStatus(player_status);	// プレイヤーステータス設定

		// 弾の発射
		if(_shotcnt <= 0)
		{
			// 弾発射
			if(_modeGame != nullptr)
			{
				// 使用可能な弾丸を取得
				Bullet* activeBullet = _modeGame->GetAvailableBullet();
				if(activeBullet != nullptr)
				{
					activeBullet->Initialize
					(
						VGet(_vPos.x, _vPos.y + 50, _vPos.z), _vDir, 5.0f, BULLET_TYPE::PLAYER
					);
				}
			}

			// 発射タイム設定
			_shotcnt = 20;
		}
	}
	else // ステータスを元に戻す
	{
		if(player_status == Player::PLAYER_STATUS::SHOT)
		{
			player_status = Player::PLAYER_STATUS::WAIT; // ステータスを無しに変更
			_player->SetPlayerStatus(player_status);	 // プレイヤーステータス設定
		}
	}

	// 弾発射タイムを減らす
	if(_shotcnt > 0)
	{
		_shotcnt--; // カウントを減らす
	}
}