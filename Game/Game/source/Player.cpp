#include "Player.h"
#include "ApplicationMain.h"
#include "ModeGame.h"
#include "Enemy.h"
#include "ShieldEnemy.h"
#include "Ability.h"
//#include "AbilityBaseAttack.h"
#include "AbilityBullet.h"
#include "AbilityMelee.h"
#include "AbilityAoE.h"
#include "AbilityShield.h"
#include "Camera.h"
#include "Map.h"

bool Player::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if (!Character::Initialize()) { return false; }
	
	// モデルデータのロード
	_handle = MV1LoadModel("res/SDChar/SDChar.mv1");

	// ステータス初期化
	{
		_status = PLAYER_STATUS::NONE;
		oldStatus = PLAYER_STATUS::NONE;

		_abilityEnum = AbilityEnum::NONE;
	}

	// カプセルコリジョン初期化
	{
		_capsule_top = VGet(0.0f, 75.0f, 0.0f);
		_capsule_bottom = VGet(0.0f, 0.0f, 0.0f);
		_capsule_radius = 20.0f;
	}

	// 位置,向きの初期化
	{
		_vPos = VGet(0, 0, 0);
		_vDir = VGet(0, 0, -1);
		_oldV = VGet(0, 0, 0);
	}

	// 他の初期化
	{
		_player_key = 0;
		_player_trg = 0;
		_player_rad = 0.0f;
		_player_length = 0.0f;
		_player_camrad = 0.0f;
		_collision_r = 30.0f;
		_collision_weight = 10.0f;
		_melee_col_r = 50.0f;
		_moveSpeed = 6.0f;
		_life = 5.0f;
		_abilityCount = 0;
	}

	// クラスインスタント初期化
	{
		_modeGame = nullptr;
		_cam = nullptr;
		_map = nullptr;
	}

	// 通常攻撃アビリティを追加
	//auto baseAttack = std::make_unique<AbilityBaseAttack>();
	//baseAttack->SetPlayer(this);  // プレイヤーをセット
	//AddAbility(std::make_unique<AbilityBaseAttack>());

	return true;
}

bool Player::Terminate()
{
	// モデルを削除する
	MV1DeleteModel(_handle); 

	return true;
}

bool Player::Process()
{
	if(_life <= 0.0f){ return false; } // 死亡処理

	// キャラ移動(カメラ設定に合わせて)
	_player_key = ApplicationMain::GetInstance()->GetKey();
	_player_trg = ApplicationMain::GetInstance()->GetTrg();

	// 攻撃処理
	PlayerAttackProcess();

	// アビリティ処理
	ProcessAbilities();

	// プレイヤーの移動
	MovePlayer();

	// プレイヤーの移動に応じた回転
	MovePlayerRotate();

	// マップとの当たり判定時のプレイヤーの移動
	MoveColPlayer();

	// 移動に応じた状態変化
	MovePlayerStatus();

	// エネミー死亡判定処理
	CheckDeathEnemy();		 

	// タイマー減少処理
	DecrementTime();

	return true; 
}

bool Player::Render()
{
	if(_life <= 0.f){ return false; } // 死亡処理

	// 再生時間をセットする
	MV1SetAttachAnimTime(_handle, _attach_index, _play_time);

	// デバッグ用
	{
		// カプセルコリジョン描画
		//DrawPlayerCapsuleCol();

		// 攻撃中なら攻撃コリジョンを描画
		if (_is_attacking)
		DrawAttackCollision();

		// プレイヤーのステータス表示関数
		//DrawPlayerStatus();

		// カラーボックス描画
		DrawColorBox();

		// 能力取得描画
		DrawGetAbility();
	}

	// アビリティの描画処理を追加
	for(auto& ability : _player_abilities.GetAllComponents())
	{
		if(ability != nullptr)
		{
			ability->Render();
		}
	}

	// クールタイム描画
	DrawCooltime();

	// その他デバッグ情報描画
	DrawOther();

	// モデルを描画する
	{
		// シールド成功時の無敵時間中は点滅させる
		if(_invincible_time > 0.f)
		{
			// 点滅処理
			if(_invincible_time > 0.0f && (static_cast<int>(_invincible_time) / 4) % 2 == 0)
			{
				// 描画しない
				return false;
			}
		}

		// 通常の無敵時間中も点滅させる
		if(_cooltime > 0.f)
		{
			// 点滅処理
			if(_cooltime > 0.0f && (static_cast<int>(_cooltime) / 4) % 2 == 0)
			{
				// 描画しない
				return false;
			}
		}

		// 位置
		MV1SetPosition(_handle, _vPos);

		// 向きからY軸回転を算出
		VECTOR vRot = { 0,0,0 };

		// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)
		vRot.y = atan2(_vDir.x * -1, _vDir.z * -1);

		// 回転をセット
		MV1SetRotationXYZ(_handle, vRot);

		// 描画
		MV1DrawModel(_handle);
	}

	return true;
}

// キャラ移動
void Player:: MovePlayer()
{
	if(_is_start_attack){ _v = { 0,0,0 }; return; } // 移動ベクトルをリセット

	//// AbilityBaseAttackを取得して攻撃中かチェック
	//auto baseAttack = GetAbility<AbilityBaseAttack>();
	//if(baseAttack && baseAttack->GetIsStartAttack()){
	//	_v = { 0,0,0 };
	//	return;
	//}

	// AbilityAoEを取得して範囲攻撃中かチェック
	AbilityAoE* abilityAoE = GetAbility<AbilityAoE>();
	if(abilityAoE && abilityAoE->GetIsStartAoEAttack()){ return; } // 範囲攻撃中は移動ベクトルをリセット

	// 移動前のステータスを保存しておく
	oldStatus = _status;

	// 移動方向を決める
	_v = { 0,0,0 };
	if(_player_key & PAD_INPUT_DOWN) { _v.x = 1; }  // 後ろ
	if(_player_key & PAD_INPUT_UP) { _v.x = -1; }   // 前
	if(_player_key & PAD_INPUT_LEFT) { _v.z = -1; } // 左
	if(_player_key & PAD_INPUT_RIGHT) { _v.z = 1; } // 右
}

// プレイヤーの移動に応じた回転
void Player::MovePlayerRotate()
{
	if(_is_start_attack) { return; } // 攻撃中は回転処理をスキップ

	// AbilityAoEを取得して範囲攻撃中かチェック
	AbilityAoE* abilityAoE = GetAbility<AbilityAoE>();
	if(abilityAoE && abilityAoE->GetIsStartAoEAttack()){ return; } // 範囲攻撃中は回転処理をスキップ

	// カメラステータス
	VECTOR cam_pos = _cam->GetPosition();
	VECTOR cam_target = _cam->GetTarget();

	// カメラの向いている角度を取得
	float sx = cam_pos.x - cam_target.x;
	float sz = cam_pos.z - cam_target.z;
	_player_camrad = atan2(sz, sx);

	// vをrad分回転させる
	_player_length = 0.f;

	// 移動速度を設定
	if(VSize(_v) > 0.f) // 移動している
	{ 
		_player_length = _moveSpeed;
	} 

	_player_rad = atan2(_v.z, _v.x); // vの角度

	// 回転後のvを計算
	_v.x = cos(_player_rad + _player_camrad) * _player_length; // 回転後のx成分
	_v.z = sin(_player_rad + _player_camrad) * _player_length; // 回転後のz成分
}

// マップとの当たり判定時のプレイヤー移動
void Player:: MoveColPlayer()
{
	if(_is_start_attack) { return; } // 攻撃中は位置更新をスキップ

	// AbilityAoEを取得して範囲攻撃中かチェック
	AbilityAoE* abilityAoE = GetAbility<AbilityAoE>(); 
	if(abilityAoE && abilityAoE->GetIsStartAoEAttack()){ return; } // 範囲攻撃中は位置更新をスキップ

	// カメラステータス
	VECTOR camera_pos = _cam->GetPosition();
	VECTOR camera_target = _cam->GetTarget();

	// マップステータス
	int map_handle= _map->GetHandleMap();
	int map_frameMapCol = _map->GetFrameMapCol();
	float map_colSubY = _map->GetColSubY();

	// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
	float escapeTbl[] =
	{
		0, -10, 10, -20, 20, -30, 30, -40, 40, -50, 50, -60, 60, -70, 70, -80, 80,
	};

	for (int i = 0; i < sizeof(escapeTbl) / sizeof(escapeTbl[0]); i++)
	{
		// 移動前の位置を保存
		_oldvPos = _vPos;
		_oldV = _v;

		// escapeTbl[i]の分だけ移動量v回転
		float escape_rad = static_cast<float>(DEG2RAD(escapeTbl[i]));
		_v.x = cos(_player_rad + _player_camrad + escape_rad) * _player_length;
		_v.z = sin(_player_rad + _player_camrad + escape_rad) * _player_length;

		// vの分移動
		_vPos = VAdd(_vPos, _v);

		// 移動した先でコリジョン判定
		MV1_COLL_RESULT_POLY hitPoly;

		// 主人公の腰位置から下方向への直線
		hitPoly = MV1CollCheck_Line(map_handle, map_frameMapCol,
			VAdd(_vPos, VGet(0, map_colSubY, 0)),
			VAdd(_vPos, VGet(0, -99999.f, 0)));

		// 当たったかどうか
		if (hitPoly.HitFlag) // 当たらなかった
		{
			// 当たったY位置をキャラ座標にする
			_vPos.y = hitPoly.HitPosition.y;

			// キャラが上下に移動した量だけ、移動量を修正
			_v.y += _vPos.y - _oldvPos.y;
			
			// カメラ移動
			_cam->SetPosition(VAdd(camera_pos, _v));
			_cam->SetTarget(VAdd(camera_target, _v));

			// ループiから抜ける
			break;
		}
		else // 当たった
		{
			// 元の座標に戻す
			_vPos = _oldvPos;
			_v = _oldV;
		}
	}
}

// プレイヤーのアニメーション状態移動
void Player::MovePlayerStatus()
{
	// 待機、移動以外の状態中
	if(_status == PLAYER_STATUS::ATTACK || _status == PLAYER_STATUS::SHOT || 
	   _status == PLAYER_STATUS::AOE || _status == PLAYER_STATUS::SHIELD ||
	   _status==PLAYER_STATUS::MELEE)
	{
		// 攻撃中のアニメーション
		if(oldStatus == _status) // ステータスが変わっていない場合
		{
			// 再生時間を進める
			_play_time += 0.5f;
		}
		else // ステータスが変わった場合
		{
			// アニメーションがアタッチされていたら、デタッチする
			if (_attach_index != -1)
			{
				MV1DetachAnim(_handle, _attach_index);
				_attach_index = -1;
			}

			// ステータスに応じたアニメーションをアタッチする
			switch(_status)
			{
				case PLAYER_STATUS::ATTACK:
				{
					// 攻撃アニメーションをアタッチする
					_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "attack"), -1, FALSE);
					break;
				}
				case PLAYER_STATUS::SHOT:
				{
					// 射撃アニメーションをアタッチする（なければidleを使用）
					_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "shot"), -1, FALSE);
					if(_attach_index == -1)
					{
						_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
					}
					break;
				}
				case PLAYER_STATUS::MELEE:
				{
					// メレーアニメーションをアタッチする（なければidleを使用）
					_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "melee"), -1, FALSE);
					if(_attach_index == -1)
					{
						_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
					}
					break;
				}
				case PLAYER_STATUS::AOE:
				{
					// AoEアニメーションをアタッチする（なければidleを使用）
					_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "aoe"), -1, FALSE);
					if(_attach_index == -1)
					{
						_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
					}
					break;
				}
				case PLAYER_STATUS::SHIELD:
				{
					// シールドアニメーションをアタッチする（なければidleを使用）
					_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "shield"), -1, FALSE);
					if(_attach_index == -1)
					{
						_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
					}
					break;
				}
			}

			// 攻撃アニメーションがない場合は待機アニメーションを使用
			if (_attach_index == -1)
			{
				_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
			}

			// アニメーションの総再生時間を取得する
			_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);

			// 再生時間を初期化する
			_play_time = 0.0f;
		}

		// 再生時間がアニメーションの総再生時間に達したら再生時間を０に戻す
		if(_play_time >= _total_time)
		{
			_play_time = 0.0f;     // 再生時間の初期化
		}

		oldStatus = _status; // ステータス保存
		return;              // 攻撃状態中はここで処理終了
	}

	// 待機、移動中の処理
	if(_status == PLAYER_STATUS::NONE || 
		_status == PLAYER_STATUS::WAIT || 
		_status == PLAYER_STATUS::WALK)
	{
		// 移動量をそのままキャラの向きにする
		if(VSize(_v) > 0.f) // 移動していない時は無視するため
		{
			_vDir = _v;						// キャラの向きを移動方向にする
			_status = PLAYER_STATUS::WALK;	// ステータスを歩行にする
		}
		else // 移動していない
		{
			_status = PLAYER_STATUS::WAIT; // ステータスを待機にする
		}

		// ステータスが変わっていないか？
		if(oldStatus == _status) // 変わっていない
		{
			// 再生時間を進める
			_play_time += 0.5f;
		}
		else
		{
			// アニメーションがアタッチされていたら、デタッチする
			if(_attach_index != -1)
			{
				MV1DetachAnim(_handle, _attach_index); // アニメーションをデタッチする
				_attach_index = -1;					   // アタッチインデックスを初期化	
			}

			// ステータスに応じたアニメーションをアタッチする
			switch(_status)
			{
				case PLAYER_STATUS::WAIT: // 待機
				{
					// アイドルアニメーションをアタッチする
					_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
					break;
				}

				case PLAYER_STATUS::WALK: // 歩行
				{
					// 歩行アニメーションをアタッチする
					_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "run"), -1, FALSE);
					break;
				}
			}

			// アニメーションの総再生時間を取得する
			_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);

			// 再生時間を初期化する
			_play_time = 0.0f;
		}
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を０に戻す
	if(_play_time >= _total_time)
	{
		_play_time = 0.0f; // 再生時間の初期化
	}
}

// プレイヤーダメージ処理
void Player::DamagePlayer()
{
	if(_is_shield_success){ return; }	// シールド成功中はダメージを受けない

	// 無敵時間のカウントダウン
	if(_cooltime > 0 || _invincible_time > 0.0f)
	{
		return;
	}

	// 無敵時間設定
	_cooltime = 60; // 無敵時間を60フレームに設定	
	_life -= 1.0f;  // 体力を減らす
}

// プレイヤー攻撃処理
void Player::AttackMove
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
	// プレイヤーの向きを正規化
	VECTOR dir_norm = VNorm(_vDir);

	// 攻撃コリジョンを前方に配置
	VECTOR attack_offset = VScale(dir_norm, 50.0f);

	// コリジョン位置
	VECTOR col_offset = VAdd(_vPos, attack_offset);

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
void Player::PlayerAttackProcess()
{
	// 攻撃ボタンが押された場合
	if((_player_trg & PAD_INPUT_1) && _status != PLAYER_STATUS::ATTACK)
	{
		// 他のアビリティが動作中なら通常攻撃を発動しない
		if(IsAnyAbilityActive())
		{
			return;
		}

		// 攻撃開始
		_status = PLAYER_STATUS::ATTACK;
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
	if(_status == PLAYER_STATUS::ATTACK) // 攻撃状態中
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
			_status = PLAYER_STATUS::WAIT; // ステータスを待機に戻す
			_is_attacking = false;		   // 攻撃中フラグを下ろす
			_is_start_attack = false;	   // 攻撃開始フラグを下ろす
			_attack_timer = 0.f;		   // 攻撃タイマーを初期化
			_hit_enemy.clear();			   // ヒット済みエネミー配列をクリア
		}
	}
}

// 攻撃ヒット判定処理
void Player::CheckAttackHit()
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
				// 全ての敵でシールドチェックを実行
				enemy->CheckShieldHit();

				if(enemy->GetIsShieldSuccess())	// 敵のシールド成功時は処理しない
				{
					continue;
				}	

				enemy->DamageEnemy(3.0f);	 // エネミーにダメージを与える
				_hit_enemy.push_back(enemy); // ヒット済みエネミー配列に追加
			}

		}
	}
}

// エネミー死亡判定処理
void Player::CheckDeathEnemy()
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
						AddAbility(std::make_unique<AbilityMelee>());
						break;
					}
					case AbilityEnum::BULLET:
					{
						AddAbility(std::make_unique<AbilityBullet>());
						break;
					}
					case AbilityEnum::AOE:
					{
						AddAbility(std::make_unique<AbilityAoE>());
						break;
					}
					case AbilityEnum::SHIELD:
					{
						AddAbility(std::make_unique<AbilityShield>());
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
void Player::ProcessAbilities()
{
	for(auto& ability : _player_abilities.GetAllComponents())
	{
		if(ability == nullptr){ continue; }
		if(_modeGame == nullptr){ continue; }
			
		// AbilityBulletの場合
		if(auto abilityBullet = dynamic_cast<AbilityBullet*>(ability.get()))
		{
			abilityBullet->SetModeGame(_modeGame);
			abilityBullet->SetPlayer(this);

			// プレイヤーの位置と向きを設定
			abilityBullet->SetPosition(_vPos);
			abilityBullet->SetDirection(_vDir);
		}
			
		// AbilityMeleeの場合
		if(auto abilityMelee = dynamic_cast<AbilityMelee*>(ability.get()))
		{
			abilityMelee->SetPlayer(this);

			// プレイヤーの位置と向きを設定
			abilityMelee->SetPosition(_vPos);
			abilityMelee->SetDirection(_vDir);
		}
			
		// AbilityAoEの場合
		if(auto abilityAoE = dynamic_cast<AbilityAoE*>(ability.get()))
		{
			for(auto& enemy : _enemies)
			{
				if(enemy == nullptr){ continue; }
				abilityAoE->SetEnemy(_enemies);
			}
			abilityAoE->SetPlayer(this);

			// プレイヤーの位置と向きを設定
			abilityAoE->SetPosition(_vPos);
			abilityAoE->SetDirection(_vDir);
		}
			
		// AbilityShieldの場合
		if(auto abilityShield = dynamic_cast<AbilityShield*>(ability.get()))
		{
			for(auto& enemy : _enemies)
			{
				if(enemy == nullptr){ continue; }
				//abilityShield->SetEnemy(_enemies);
			}
			abilityShield->SetPlayer(this);

			// プレイヤーの位置と向きを設定
			abilityShield->SetPosition(_vPos);
			abilityShield->SetDirection(_vDir);
		}

		ability->Process();
	}
}

// タイマー減少処理
void Player::DecrementTime()
{
	// 無敵時間のカウントダウン
	if(_cooltime > 0)
	{
		_cooltime -= 1;
	}
	if(_invincible_time > 0.0f)
	{
		_invincible_time -= 1.0f;
	}
}

// アビリティ追加
void Player::AddAbility(std::unique_ptr<Ability> ability)
{
	if(ability == nullptr){ return; }

	bool shouldAdd = true;

	// 各アビリティタイプの重複チェック
	if(dynamic_cast<AbilityMelee*>(ability.get()))
	{
		shouldAdd = !GetAbility<AbilityMelee>();
	}
	else if(dynamic_cast<AbilityBullet*>(ability.get()))
	{
		shouldAdd = !GetAbility<AbilityBullet>();
	}
	else if(dynamic_cast<AbilityAoE*>(ability.get()))
	{
		shouldAdd = !GetAbility<AbilityAoE>();
	}
	else if(dynamic_cast<AbilityShield*>(ability.get()))
	{
		shouldAdd = !GetAbility<AbilityShield>();
	}

	// 重複していない場合のみ追加
	if(shouldAdd)
	{
		ability->Initialize();
		_player_abilities.AddComponent(std::move(ability));
	}
}

// ほかのアビリティがアクティブかどうか
bool Player::IsAnyAbilityActive()const
{
	return (_status == PLAYER_STATUS::ATTACK ||
			_status == PLAYER_STATUS::MELEE ||
			_status == PLAYER_STATUS::SHOT ||
			_status == PLAYER_STATUS::AOE ||
			_status == PLAYER_STATUS::SHIELD);
}

/*デバッグ表示-----------------------------------------------------------------------------------*/
// カプセルコリジョン描画
void Player::DrawPlayerCapsuleCol()
{
	// カプセルコリジョンの計算
	VECTOR capsuleTop = VAdd(_vPos, _capsule_top);
	VECTOR capsuleBottom = VAdd(_vPos, _capsule_bottom);

	// カプセルコリジョンを描画
	DrawCapsule3D
	(
		capsuleTop,				// カプセルコリジョン上端
		capsuleBottom,			// カプセルコリジョン下端
		_capsule_radius,		// カプセルコリジョン半径
		8,						// 分割数
		GetColor(25, 0, 0),		// 赤色
		GetColor(5, 55, 255),	// 白色
		FALSE					// 塗りつぶし
	);
}

// 攻撃コリジョン描画
void Player::DrawAttackCollision()
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

// 能力取得描画
void Player::DrawGetAbility()
{
	// 取得した能力リストを表示
	{
		// 各能力の種類を数える
		int melee_count = 0;
		int bullet_count = 0;
		int aoe_count = 0;
		int shield_count = 0;

		for(const auto& ability : _player_abilities.GetAllComponents())
		{
			// dynamic_castを使ってアビリティタイプを判定
			if(dynamic_cast<const AbilityMelee*>(ability.get()))
			{
				melee_count++;
			}
			else if(dynamic_cast<const AbilityBullet*>(ability.get()))
			{
				bullet_count++;
			}
			else if(dynamic_cast<const AbilityAoE*>(ability.get()))
			{
				aoe_count++;
			}
			else if(dynamic_cast<const AbilityShield*>(ability.get()))
			{
				shield_count++;
			}
		}

		// 能力を描画
		const char* current_ability_name = ""; // 能力名の初期化
		int x_offset = 30;
		int y_offset = 390;

		// 各能力を個別に表示
		bool hasAbility = false;

		// 背景ボックスのサイズを計算
		_abilityCount = 0;
		if(melee_count > 0) _abilityCount++;
		if(bullet_count > 0) _abilityCount++;
		if(aoe_count > 0) _abilityCount++;
		if(shield_count > 0) _abilityCount++;

		// アビリティリストのタイトル表示
		DrawFormatString(0, y_offset, GetColor(0, 255, 255), "===== Get Abilities =====");
		y_offset += 30;

		// 現在の能力名を表示
		if(melee_count > 0)
		{
			DrawFormatString(x_offset, y_offset, GetColor(0, 255, 255), "Ability Melee   Dキー");
			y_offset += 30;
			hasAbility = true;
		}
		if(bullet_count > 0)
		{
			DrawFormatString(x_offset, y_offset, GetColor(0, 255, 255), "Ability Bullet	 Xキー");
			y_offset += 30;
			hasAbility = true;
		}
		if(aoe_count > 0)
		{
			DrawFormatString(x_offset, y_offset, GetColor(0, 255, 255), "Ability AOE     Aキー");
			y_offset += 30;
			hasAbility = true;
		}
		if(shield_count > 0)
		{
			DrawFormatString(x_offset, y_offset, GetColor(0, 255, 255), "Ability Shield  Sキー");
			y_offset += 30;
			hasAbility = true;
		}
	}
}

// プレイヤーのステータス表示関数
void Player::DrawPlayerStatus()
{
	// プレイヤーのステータス
	const char* player_status = "";

	// プレイヤーのステータスに応じた表示
	switch(_status)
	{
		case PLAYER_STATUS::NONE:
		{
			player_status = "NONE";
			break;
		}
		case PLAYER_STATUS::WAIT:
		{
			player_status = "WAIT";
			break;
		}
		case PLAYER_STATUS::WALK:
		{
			player_status = "WALK";
			break;
		}
		case PLAYER_STATUS::ATTACK:
		{
			player_status = "ATTACK";
			break;
		}
		case PLAYER_STATUS::SHOT:
		{
			player_status = "SHOT";
			break;
		}
		case PLAYER_STATUS::MELEE:
		{
			player_status = "MELEE";
			break;
		}
		case PLAYER_STATUS::AOE:
		{
			player_status = "AOE";
			break;
		}
		case PLAYER_STATUS::SHIELD:
		{
			player_status = "SHIELD";
			break;
		}
		case PLAYER_STATUS::DEATH:
		{
			player_status = "DEATH";
			break;
		}
	}

	//DrawFormatString(0, 80, GetColor(0, 255, 0), "PLAYER_STATUS : %s", player_status);
}

// 値描画
void Player::DrawCooltime()
{
	// 無敵時間の表示
	if(_cooltime > 0)
	{
		//DrawFormatString(0, 260, GetColor(0, 255, 0), "COOL TIME : %d", _cooltime);
	}

	// シールド成功フラグの表示
	if(_is_shield_success)
	{
		//DrawFormatString(0, 280, GetColor(0, 255, 0), "SHIELD SUCCESS: TRUE");
	}

	// 無敵時間
	if(_invincible_time > 0.0f)
	{
		//DrawFormatString(0, 300, GetColor(0, 255, 0), "INVINCIBLE TIME : %f", _invincible_time);
	}

	// プレイヤースピード
	//DrawFormatString(0, 360, GetColor(0, 255, 0), " MOVE SPEED : %f", _moveSpeed);
}

// その他デバッグ情報描画
void Player::DrawOther()
{
	DrawFormatString(10, 10, GetColor(0, 255, 255), "-------PLAYER-------");

	// プレイヤーライフ表示
	{
		// プレイヤーのライフを取得して表示
		std::string playerLife = "Player Life : " + std::to_string(_life);
		DrawString(10, 50, playerLife.c_str(), GetColor(0, 255, 255));
	}

	// ライフ表示の隣に移動キーの表示
	{
		int x = 10;
		int y = 250;

		DrawFormatString(x, y, GetColor(0, 255, 255), "------ Move Keys ------");
		y += 20;
		DrawFormatString(x + 120, y + 10, GetColor(0, 255, 255), "↑");
		y += 20;											   
		DrawFormatString(x + 120, y + 20, GetColor(0, 255, 255), "↓");
		y += 20;											   
		DrawFormatString(x + 90, y, GetColor(0, 255, 255), "←");
		y += 20;											   
		DrawFormatString(x + 150, y - 20, GetColor(0, 255, 255), "→");
	}

	// 移動キーの隣に攻撃キーの表示
	{
		int x = 10;
		int y = 130;

		DrawFormatString(x, y, GetColor(0, 255, 255), "---- Normal Attack ----");
		y += 20;

		DrawFormatString(x + 120, y + 30, GetColor(0, 255, 255), "Zキー");
	}
}

// カラーキューブ描画
void Player::DrawColorBox()
{
	// 背景ボックスの描画（半透明）
	int box_x = 0;
	int box_y = 0;
	int box_width = 350;
	int box_height = 420;

	if(_abilityCount > 0)
	{
		box_height = box_height + (_abilityCount * 30); // タイトル + アビリティ分
	}

	// 半透明設定
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160); // 50%透明
	DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // 元に戻す
}