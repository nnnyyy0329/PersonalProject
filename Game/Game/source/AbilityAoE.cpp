#include "AbilityAoE.h"
#include "ApplicationMain.h"
#include "Enemy.h"
#include "ModeGame.h"
#include "Player.h"

bool AbilityAoE::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if (!Ability::Initialize()) { return false; }

	// 円形床コリジョン初期化を追加
	_circleFloor.center = VGet(0.0f, 0.0f, 0.0f); // 初期位置
	_circleFloor.radius = 150.0f;                   // 半径
	_circleFloor.height = 0.0f;                     // 高さ

	// 攻撃用円形コリジョン初期化を追加
	_attack_circle.center = VGet(0.0f, 0.0f, 0.0f); // 初期位置
	_attack_circle.radius = 150.0f;                  // 半径
	_attack_circle.height = 0.0f;                    // 高さ

	// 初期化
	_aoe_attack_damage = 2.0f; // 範囲攻撃ダメージ
	_attack_timer = 0.0f;
	_is_attacking = false;
	_is_start_aoe_attack = false;

	for(auto& enemy : _enemies)
	{
		enemy = nullptr;
	}
	_player = nullptr;

	return true;
}

bool AbilityAoE::Terminate()
{
	return true;
}

bool AbilityAoE::Process()
{
	// 攻撃処理
	ProcessAttack();

	// 攻撃ヒット判定処理
	CheckAttackHit();

	return true;
}

bool AbilityAoE::Render()
{
	// 範囲攻撃範囲描画
	DrawAoEAttackRange(); 

	return true; 
}

// 円形攻撃処理
void AbilityAoE::AttackMoveCircle
(
	VECTOR circle_center,  // 円の中心位置
	float circle_radius,   // 円の半径  
	float circle_height,   // 円の高さ
	float attack_delay,    // 攻撃開始までの遅延時間
	float attack_duration, // 攻撃持続時間
	float recovery,        // 攻撃後の後隙
	float damage,          // ダメージ量
	bool is_attacking      // 攻撃中フラグ
)
{
	_attack_circle.center = circle_center; // 円の中心位置
	_attack_circle.radius = circle_radius; // 円の半径
	_attack_circle.height = circle_height; // 円の高さ
	_attack_delay = attack_delay;		   // 攻撃開始までの遅延時間
	_attack_duration = attack_duration;    // 攻撃持続時間
	_attack_recovery = recovery;		   // 攻撃後の後隙
	_damage = damage;					   // ダメージ量
	_is_attacking = is_attacking;		   // 攻撃中フラグ
}

// 攻撃処理
void AbilityAoE::ProcessAttack()
{
	if(_player == nullptr) return;

	int trg = ApplicationMain::GetInstance()->GetTrg();

	// プレイヤーのステータス
	Player::PLAYER_STATUS player_status = _player->GetPlayerStatus();

	// 円形床判定の中心位置をプレイヤーの位置に設定
	_circleFloor.center = _player->GetPosition();

	// 範囲攻撃開始フラグが立っていて、ステータスが攻撃でないなら
	if(trg & PAD_INPUT_4 && !_is_start_aoe_attack)
	{
		// 他のアビリティが動作中なら範囲攻撃を発動しない
		if(_player->IsAnyAbilityActive() && player_status != Player::PLAYER_STATUS::AOE)
		{
			return;
		}

		player_status = Player::PLAYER_STATUS::AOE; // ステータスを範囲攻撃に変更
		_player->SetPlayerStatus(player_status);	// プレイヤーステータス設定

		_attack_timer = 0.0f;			// 攻撃タイマー初期化
		_is_start_aoe_attack = true;	// 範囲攻撃開始フラグを立てる
		_is_attacking = false;			// 初期は攻撃中ではない
		_hit_enemy.clear();				// ヒット済み敵をクリア

		// 円形攻撃処理セット
		AttackMoveCircle
		(
			_circleFloor.center, // 円の中心位置
			_circleFloor.radius, // 円の半径  
			_circleFloor.height, // 円の高さ
			20.0f,				 // 攻撃開始までの遅延時間
			15.0f,				 // 攻撃持続時間
			30.0f,				 // 攻撃後の後隙
			_aoe_attack_damage,	 // ダメージ量
			false				 // 攻撃中フラグ
		);
	}

	// 攻撃中の処理
	if(player_status == Player::PLAYER_STATUS::AOE) // 攻撃状態中
	{
		_attack_timer += 1.0f; // 攻撃タイマーを進める

		// 攻撃開始
		if(_attack_timer >= _attack_delay && _attack_timer < _attack_delay + _attack_duration)
		{
			if(!_is_attacking) // 攻撃中フラグが立っていない場合のみ
			{
				_is_attacking = true; // 攻撃中フラグを立てる
			}

			// 攻撃ヒット判定処理
			CheckAttackHit();
		}

		// 攻撃終了
		if(_attack_timer >= _attack_delay + _attack_duration + _attack_recovery)
		{
			_player->SetPlayerStatus(Player::PLAYER_STATUS::WAIT); // プレイヤーステータス設定
			_is_attacking = false;								   // 攻撃中フラグを下ろす
			_is_start_aoe_attack = false;						   // 攻撃開始フラグを下ろす
			_attack_timer = 0.f;								   // 攻撃タイマーを初期化
			_hit_enemy.clear();									   // ヒット済み敵をクリア
		}
	}
}

// 攻撃ヒット判定処理
void AbilityAoE::CheckAttackHit()
{
	if(!_is_attacking) return;

	for(auto& enemy : _enemies)
	{
		if(enemy == nullptr) continue; // エネミーがいない場合は処理しない

		// すでにヒット済みの敵かチェック
		if(std::find(_hit_enemy.begin(), _hit_enemy.end(), enemy) != _hit_enemy.end())
		{
			continue; // すでにヒット済みならスキップ
		}

		// エネミーの位置を取得
		VECTOR enemyPos = enemy->GetPosition();

		// エネミーが攻撃範囲内にいるかチェック
		if(_attack_circle.IsPointInside(enemyPos))
		{
			enemy->DamageEnemy(_aoe_attack_damage); // エネミーにダメージを与える
			_hit_enemy.push_back(enemy);			// ヒット済みリストに追加
		}
	}
}

// 範囲攻撃範囲描画
void AbilityAoE::DrawAoEAttackRange()
{
	if(!_is_start_aoe_attack) return;
	if(_player == nullptr) return;

	// 攻撃範囲円形コリジョンの中心位置をプレイヤーの位置に設定
	_attack_circle.center = _player->GetPosition();

	// 攻撃範囲円形床描画
	const int segments = 64; // 円の分割数
	const float angleStep = 2.0f * DX_PI_F / segments;

	for(int i = 0; i < segments; ++i)
	{
		float angle1 = i * angleStep;		// 現在の角度
		float angle2 = (i + 1) * angleStep; // 次の角度
		
		// 円周上の2点を計算
		{
			// 1点目
			VECTOR p1 = VGet
			(
				_attack_circle.center.x + cos(angle1) * _attack_circle.radius, // X座標
				_attack_circle.height + 1.0f,								   // 少し上にずらす
				_attack_circle.center.z + sin(angle1) * _attack_circle.radius  // Z座標
			);

			// 2点目
			VECTOR p2 = VGet
			(
				_attack_circle.center.x + cos(angle2) * _attack_circle.radius, // X座標
				_attack_circle.height + 1.0f,								   // 少し上にずらす
				_attack_circle.center.z + sin(angle2) * _attack_circle.radius  // Z座標
			);

			int color = _is_attacking ? GetColor(0, 255, 0) : GetColor(150, 50, 50);
			DrawLine3D(p1, p2, color);
		}
	}

	// 攻撃中は中心からがいっふうへのラインも描画
	if(_is_attacking)
	{
		const int radialLines = 16; // ラインの分割数
		const float radialAngleStep = 2.0f * DX_PI_F / radialLines;

		for(int i = 0; i < radialLines; ++i)
		{
			float angle = i * radialAngleStep; // 現在の角度

			// 中心点
			VECTOR centerPoint = VGet
			(
				_attack_circle.center.x,
				_attack_circle.height + 1.0f,
				_attack_circle.center.z
			);

			// 外周点
			VECTOR edgePoint = VGet
			(
				_attack_circle.center.x + cos(angle) * _attack_circle.radius,
				_attack_circle.height + 1.0f,
				_attack_circle.center.z + sin(angle) * _attack_circle.radius
			);

			DrawLine3D(centerPoint, edgePoint, GetColor(255, 0, 0));
		}

		// 攻撃中心点を描画
		//DrawSphere3D(_attack_circle.center, 8.0f, 16, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);
	}
	
	// デバッグ情報表示
	{
		//DrawFormatString(10, 900, GetColor(255, 255, 255), "AoE Timer: %.1f", _attack_timer);

		//DrawFormatString(10, 920, _is_attacking ? GetColor(0, 255, 0) : GetColor(255, 0, 0),
		//	"AoE Attack: %s", _is_attacking ? "ACTIVE" : "INACTIVE");

		//DrawFormatString(10, 940, _is_start_aoe_attack ? GetColor(0, 255, 0) : GetColor(255, 0, 0),
		//	"AoE Started: %s", _is_start_aoe_attack ? "TRUE" : "FALSE");
	}
}
