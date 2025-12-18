#include "AbilityBaseAttack.h"
#include "ApplicationMain.h"
#include "Player.h"
#include "Enemy.h"
#include "Ability.h"
#include "AbilityBullet.h"
#include "AbilityMelee.h"
#include "AbilityAoE.h"
#include "AbilityShield.h"

bool AbilityBaseAttack::Initialize()
{
    if(!Ability::Initialize()) { return false; }

    // 攻撃コリジョン初期化
    {
        _attack_timer = 0.0f;
        _attack_delay = 10.0f;
        _attack_duration = 15.0f;
        _attack_recovery = 5.0f;
        _damage = 1.0f;
        _is_attacking = false;
        _is_start_attack = false;

        _attack_col_top = VGet(0.0f, 50.0f, 0.0f);
        _attack_col_bottom = VGet(0.0f, 10.0f, 0.0f);
        _attack_col_r = 20.0f;
    }

    return true;
}

bool AbilityBaseAttack::Terminate()
{
    return true;
}

bool AbilityBaseAttack::Process()
{
	if(_player->GetLife() <= 0.f){ return false; } // プレイヤーが死んでいたらスキップ

	// 攻撃処理
	PlayerAttackProcess();

	// アビリティ処理
	ProcessAbilities();

	// エネミー死亡判定処理
	CheckDeathEnemy();

    return true;
}

bool AbilityBaseAttack::Render()
{
    if(_is_attacking)
    {
        DrawAttackCollision();
    }
    return true;
}

// プレイヤー攻撃処理
void AbilityBaseAttack::AttackMove
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
	if(_player == nullptr) return;

    // プレイヤーの向きを正規化
    VECTOR dir_norm = VNorm(_player->GetDirection());

    // 攻撃コリジョンを前方に配置
    VECTOR attack_offset = VScale(dir_norm, 75.0f);

    // コリジョン位置
    VECTOR col_offset = VAdd(_player->GetPosition(), attack_offset);

    // 攻撃コリジョンをセット-----------------------------
    _attack_col_top = VAdd(col_offset, col_top);	   // 攻撃コリジョン上端
    _attack_col_bottom = VAdd(col_offset, col_bottom); // 攻撃コリジョン下端
    _attack_col_r = col_r;							   // 攻撃コリジョン半径
    _attack_delay = attack_delay;					   // 攻撃開始までの遅延時間
    _attack_duration = attack_duration;				   // 攻撃持続時間
    _attack_recovery = recovery;					   // 攻撃後の後隙	
    _damage = damage;								   // ダメージ量
}

// 攻撃処理
void AbilityBaseAttack::PlayerAttackProcess()
{
	if(_player == nullptr) return;

	int trg = ApplicationMain::GetInstance()->GetTrg();
	auto player_status = _player->GetPlayerStatus();

	// 攻撃ボタンが押された場合
	if((trg & PAD_INPUT_1) && player_status != Player::PLAYER_STATUS::ATTACK)
	{
		// 攻撃開始
		_player->SetPlayerStatus(Player::PLAYER_STATUS::ATTACK);
		_attack_timer = 0.0f;
		_is_start_attack = true;

		// 攻撃開始時にヒット済みエネミー配列をクリア
		_hit_enemy.clear();

		// 攻撃コリジョンをセット
		AttackMove
		(
			VGet(0.0f, 50.0f, 0.0f),  // 攻撃コリジョン上端
			VGet(0.0f, 10.0f, 0.0f),  // 攻撃コリジョン下端
			20.0f,                    // 攻撃コリジョン半径
			10.0f,                    // 攻撃開始までの遅延時間
			15.0f,                    // 攻撃持続時間
			5.0f,                     // 攻撃後の後隙
			1.0f,                     // ダメージ量
			false					  // 攻撃中フラグ
		);
	}

	// 攻撃中の処理
	if(player_status == Player::PLAYER_STATUS::ATTACK) // 攻撃状態中
	{
		_attack_timer += 1.0f; // 攻撃タイマーを進める

		// 攻撃開始
		if(_attack_timer >= _attack_delay && _attack_timer < _attack_delay + _attack_duration)
		{
			_is_attacking = true; // 攻撃中フラグを立てる

			// 攻撃ヒット判定処理
			CheckAttackHit();
		}
		else // 攻撃終了
		{
			_is_attacking = false; // 攻撃中フラグを下ろす
		}

		// 攻撃終了
		if(_attack_timer >= _attack_delay + _attack_duration + _attack_recovery)
		{
			_player->SetPlayerStatus(Player::PLAYER_STATUS::WAIT); // ステータスを待機に戻す
			_is_attacking = false;		   // 攻撃中フラグを下ろす
			_is_start_attack = false;	   // 攻撃開始フラグを下ろす
			_attack_timer = 0.f;		   // 攻撃タイマーを初期化
			_hit_enemy.clear();			   // ヒット済みエネミー配列をクリア
		}
	}
}

// 攻撃ヒット判定処理
void AbilityBaseAttack::CheckAttackHit()
{
	if(!_is_attacking) return; // 攻撃中じゃないなら処理しない

	// 攻撃コリジョンの中心位置を計算
	VECTOR attack_center = VGet
	(
		(_attack_col_top.x + _attack_col_bottom.x) * 0.5f, // x座標
		(_attack_col_top.y + _attack_col_bottom.y) * 0.5f, // y座標
		(_attack_col_top.z + _attack_col_bottom.z) * 0.5f  // z座標
	);

	// エネミー全員に対して攻撃判定を行う
	for(auto& enemy : _enemies)
	{
		if(enemy == nullptr || !_is_attacking) continue; // エネミーがいない場合は処理しない

		// すでにヒット済みのエネミーかどうかをチェック
		if(std::find(_hit_enemy.begin(), _hit_enemy.end(), enemy) != _hit_enemy.end())
		{
			continue; // すでにヒット済みならスキップ
		}

		// エネミーとの距離チェック
		{
			VECTOR enemy_pos = enemy->GetPosition();		  // エネミーの位置
			VECTOR to_enemy = VSub(enemy_pos, attack_center); // プレイヤーからエネミーへのベクトル
			float distance = VSize(to_enemy);				  // プレイヤーとエネミーの距離

			// 攻撃コリジョンとエネミーの距離が接触しているかの判定
			if(distance <= _attack_col_r + enemy->GetColR())
			{
				enemy->DamageEnemy(3.0f);	 // エネミーにダメージを与える
				_hit_enemy.push_back(enemy); // ヒット済みエネミー配列に追加
			}
		}
	}
}

// エネミー死亡判定処理
void AbilityBaseAttack::CheckDeathEnemy()
{
	for(auto& enemy : _enemies)
	{
		if(enemy == nullptr) continue; // エネミーがいない場合は処理しない

		// エネミーの体力が0以下なら死亡処理を呼び出す
		if(enemy->GetLife() <= 0.f)
		{
			// すでに死亡済みのエネミーかどうかをチェック
			if(std::find(_death_enemies.begin(), _death_enemies.end(), enemy) == _death_enemies.end())
			{
				// アビリティインスタンスを作成して追加
				switch(enemy->GetEnemyAbility())
				{
					case AbilityEnum::MELEE:
					{
						_player->AddAbility(std::make_unique<AbilityMelee>());
						break;
					}
					case AbilityEnum::BULLET:
					{
						_player->AddAbility(std::make_unique<AbilityBullet>());
						break;
					}
					case AbilityEnum::AOE:
					{
						_player->AddAbility(std::make_unique<AbilityAoE>());
						break;
					}
					case AbilityEnum::SHIELD:
					{
						_player->AddAbility(std::make_unique<AbilityShield>());
						break;
					}
				}

				// 死亡済みエネミー配列に追加
				_death_enemies.push_back(enemy);
			}
		}
	}
}

// プレイヤーの能力に応じた処理
void AbilityBaseAttack::ProcessAbilities()
{
	if(_player == nullptr) return;

	// プレイヤーからアビリティ配列を取得
	const auto& player_abilities = _player->GetPlayerAbilities();

	for(auto& ability : player_abilities)
	{
		if(ability == nullptr){ continue; }
		if(_modeGame == nullptr){ continue; }

		// プレイヤーの位置と向きを取得
		VECTOR player_pos = _player->GetPosition();
		VECTOR player_dir = _player->GetDirection();

		// AbilityBulletの場合
		if(auto abilityBullet = dynamic_cast<AbilityBullet*>(ability.get()))
		{
			abilityBullet->SetModeGame(_modeGame);
			abilityBullet->SetPlayer(_player);

			// プレイヤーの位置と向きを設定
			abilityBullet->SetPosition(player_pos);
			abilityBullet->SetDirection(player_dir);
		}

		// AbilityMeleeの場合
		if(auto abilityMelee = dynamic_cast<AbilityMelee*>(ability.get()))
		{
			abilityMelee->SetPlayer(_player);

			// プレイヤーの位置と向きを設定
			abilityMelee->SetPosition(player_pos);
			abilityMelee->SetDirection(player_dir);
		}

		// AbilityAoEの場合
		if(auto abilityAoE = dynamic_cast<AbilityAoE*>(ability.get()))
		{
			for(auto& enemy : _enemies)
			{
				if(enemy == nullptr){ continue; }
				abilityAoE->SetEnemy(_enemies);
			}
			abilityAoE->SetPlayer(_player);

			// プレイヤーの位置と向きを設定
			abilityAoE->SetPosition(player_pos);
			abilityAoE->SetDirection(player_dir);
		}

		// AbilityShieldの場合
		if(auto abilityShield = dynamic_cast<AbilityShield*>(ability.get()))
		{
			for(auto& enemy : _enemies)
			{
				if(enemy == nullptr){ continue; }
				//abilityShield->SetEnemy(_enemies);
			}
			abilityShield->SetPlayer(_player);

			// プレイヤーの位置と向きを設定
			abilityShield->SetPosition(player_pos);
			abilityShield->SetDirection(player_dir);
		}

		ability->Process();
	}
}

void AbilityBaseAttack::DrawAttackCollision()
{
    DrawCapsule3D(
        _attack_col_top,
        _attack_col_bottom,
        _attack_col_r,
        8,
        GetColor(255, 0, 0),
        GetColor(255, 255, 255),
        FALSE
    );
}