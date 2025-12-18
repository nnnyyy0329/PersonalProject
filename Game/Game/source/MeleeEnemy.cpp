#include "MeleeEnemy.h"
#include "ModeGame.h"
#include "Player.h"

bool MeleeEnemy::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if(!Enemy::Initialize()) { return false; }

	// モデルデータのロード
	_handle = MV1LoadModel("res/SDChar/SDChar.mv1");

	// ステータス
	_status = ENEMY_STATUS::NONE;		 // ステータスを「無し」に設定
	_enemy_ability = AbilityEnum::MELEE; // 近接攻撃能力に設定

	// 位置,向きの初期化
	//_vPos = VGet(-350, 0, 0);
	_vDir = VGet(0, 0, -1);

	// カプセルコリジョン初期化
	_capsule_top = VGet(0.0f, 75.0f, 0.0f);
	_capsule_bottom = VGet(0.0f, 0.0f, 0.0f);
	_capsule_radius = 20.0f;

	// 他の初期化
	_life = 5.0f;
	_rotation_speed = 0.05f;
	_moveSpeed = 2.5f;

	// 公転運動の初期化
	_initial_position = VGet(0.0f, 0.0f, 0.0f);
	_orbit_radius = 100.0f;     // 公転半径
	_orbit_angle = 0.0f;        // 公転角度初期値
	_orbit_speed = 0.025f;		// 公転速度（ラジアン/フレーム）
	_orbit_initialized = false; // 公転初期化フラグ

	// 公転中心のオフセットを生成時に決定（各敵で異なる値）
	_orbit_offset = VGet(
		(rand() % 201) - 100,  // -100 ~ 100
		0.0f,
		(rand() % 201) - 100   // -100 ~ 100
	);

	// 弾クラスのインスタンスを生成
	_modeGame = nullptr;
	_player = nullptr;

	return true;
}

bool MeleeEnemy::Terminate()
{
	// モデルを削除する
	MV1DeleteModel(_handle);

	return true;
}

bool MeleeEnemy::Process()
{
	// 敵の死亡処理
	DeathEnemy();

	if(_life <= 0){ return false; }

	// 攻撃処理
	AttackProcess();

	// 敵の行動処理
	EnemyStatusMove();

	// 敵の回転処理
	EnemyRotate();

	// 攻撃中の移動処理
	AttackMove();

	// コリジョン座標更新
	UpdateAttackColPos();

	return true;
}

bool MeleeEnemy::Render()
{
	if(_life <= 0){ return false; }

	// 再生時間をセットする
	MV1SetAttachAnimTime(_handle, _attach_index, _play_time);

	// モデルを描画する
	{
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

	// 敵のコリジョン描画
	//DrawEnemyCollision();

	// 攻撃コリジョン描画
	DrawAttackCollision();

	// ステータス表示
	//DrawStatus();

	// 攻撃コリジョン位置表示
	//DrawAttackColPos();

	return true;
}

// 敵の死亡処理
void MeleeEnemy::DeathEnemy()
{
	// 敵の体力が0以下なら死亡処理
	if(_life <= 0.0f)
	{
		_is_dead = true; // 死亡フラグを立てる

		// アニメーションがアタッチされていたら、デタッチする
		if(_attach_index != -1)
		{
			MV1DetachAnim(_handle, _attach_index); // アニメーションをデタッチする
			_attach_index = -1;					   // アタッチインデックスを初期化	
		}

		// 死亡アニメーションをアタッチする
		_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "death"), -1, FALSE);

		// アニメーションの総再生時間を取得する
		_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);

		// 再生時間を初期化する
		_play_time = 0.0f;

		// ステータスを死亡に設定
		_status = ENEMY_STATUS::DEATH;
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を０に戻す
	if(_play_time >= _total_time)
	{
		_play_time = 0.0f; // 再生時間の初期化
	}
}

// 敵の行動処理
void MeleeEnemy::EnemyStatusMove()
{
	// 処理前のステータスを保存しておく
	ENEMY_STATUS oldStatus = _status;

	// ステータスが変わっていないか？
	if(oldStatus == _status) // 変わっていない
	{
		// 再生時間を進める
		_play_time += 0.5f;
	}
	else // 変わっている
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
			case ENEMY_STATUS::ATTACK: // 攻撃
			{
				// アイドルアニメーションをアタッチする
				_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
				break;
			}
		}

		// アニメーションの総再生時間を取得する
		_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);

		// 再生時間を初期化する
		_play_time = 0.0f;
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を０に戻す
	if(_play_time >= _total_time)
	{
		_play_time = 0.0f; // 再生時間の初期化
	}
}

// 敵を回転させる
void MeleeEnemy::EnemyRotate()
{
	// 現在の向きから角度を計算
	float currentAngle = atan2(_vDir.x, _vDir.z);

	// 角度更新
	currentAngle += _rotation_speed;

	_vDir.x = sin(currentAngle);
	_vDir.y = 0.0f;
	_vDir.z = cos(currentAngle);

	_vDir = VNorm(_vDir);
}

// 近接攻撃エネミー攻撃処理
void MeleeEnemy::AttackMove
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
	VECTOR dir_norm = VNorm(_vDir);

	// 攻撃コリジョンを前方に配置
	VECTOR attack_offset = VScale(dir_norm, 100.0f);

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
void MeleeEnemy::AttackProcess()
{
	// 敵の状態が攻撃じゃないなら
	if(_status != ENEMY_STATUS::ATTACK)
	{
		// 攻撃開始
		_status = ENEMY_STATUS::ATTACK;
		_attack_timer = 0.0f;

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
	}

	// 攻撃中の処理
	if(_status == ENEMY_STATUS::ATTACK) // 攻撃状態中
	{
		_attack_timer += 15.0f; // 攻撃タイマーを進める

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
		if(_attack_timer >= _attack_delay + _attack_duration)
		{
			_status = ENEMY_STATUS::WAIT; // ステータスを待機に戻す
			_is_attacking = false;		   // 攻撃中フラグを下ろす
			//_attack_timer = 0.f;		   // 攻撃タイマーを初期化
		}
	}
}

// 攻撃ヒット判定処理
void MeleeEnemy::CheckAttackHit()
{
	// 攻撃中じゃないなら処理しない
	if(!_is_attacking) return;

	// 攻撃コリジョンの中心位置を計算
	VECTOR attack_center = VGet
	(
		(_attack_col_top.x + _attack_col_bottom.x) * 0.5f, // x座標
		(_attack_col_top.y + _attack_col_bottom.y) * 0.5f, // y座標
		(_attack_col_top.z + _attack_col_bottom.z) * 0.5f  // z座標
	);

	if(_player == nullptr || !_is_attacking) return; // プレイヤーがいない場合は処理しない

	// プレイヤーとの距離チェック
	{
		VECTOR player_pos = _player->GetPosition();		  // エネミーの位置
		VECTOR to_player= VSub(player_pos, attack_center); // プレイヤーからエネミーへのベクトル
		float distance = VSize(to_player);				  // プレイヤーとエネミーの距離

		// 攻撃コリジョンとエネミーの距離が接触しているかの判定
		if(distance <= _attack_col_r + _player->GetColR())
		{
			_player->DamagePlayer(); // プレイヤーにダメージを与える
		}
	}
}

// 攻撃中の移動処理
void MeleeEnemy::AttackMove()
{
	// 公転運動の初期化（初回のみ）
	if(!_orbit_initialized)
	{
		_initial_position = _vPos;	// 現在位置を初期位置として保存
		_orbit_center = VAdd(_initial_position, _orbit_offset);
		_orbit_initialized = true;
	}

	// 公転角度を更新
	_orbit_angle += _orbit_speed; 	// 公転速度分だけ角度を増加

	// 角度を0～2πの範囲に収める
	_orbit_angle = fmod(_orbit_angle, 2.0f * DX_PI_F);	// 0～2πの範囲に収める

	// 公転位置を計算
	VECTOR _orbit_position = VGet
	(
		_orbit_center.x + cos(_orbit_angle) * _orbit_radius,  // X座標
		_orbit_center.y,                                      // Y座標（高さは変わらない）
		_orbit_center.z + sin(_orbit_angle) * _orbit_radius   // Z座標
	);

	// 位置を更新
	_vPos = _orbit_position;
}

// コリジョン座標更新
void MeleeEnemy::UpdateAttackColPos()
{
	// 攻撃コリジョンの中心位置を計算
	VECTOR attack_center = VGet
	(
		(_attack_col_top.x + _attack_col_bottom.x) * 0.5f, // x座標
		(_attack_col_top.y + _attack_col_bottom.y) * 0.5f, // y座標
		(_attack_col_top.z + _attack_col_bottom.z) * 0.5f  // z座標
	);

	_collision_pos = attack_center; // コリジョン位置を更新
}

// コリジョン描画
void MeleeEnemy::DrawEnemyCollision()
{
	// 敵のコリジョン描画
	DrawCapsule3D
	(
		VAdd(_vPos, _capsule_top),
		VAdd(_vPos, _capsule_bottom),
		_capsule_radius,
		8,
		GetColor(0, 0, 255),     // 青色
		GetColor(255, 255, 255), // 白色
		FALSE
	);
}

// 攻撃コリジョン描画
void MeleeEnemy::DrawAttackCollision()
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

// ステータス表示関数
void MeleeEnemy::DrawStatus()
{
	// ステータス
	const char* enemy_status = "";

	// ステータスに応じた表示
	switch(_status)
	{
		case ENEMY_STATUS::NONE:
		{
			enemy_status = "NONE";
			break;
		}
		case ENEMY_STATUS::WAIT:
		{
			enemy_status = "WAIT";
			break;
		}
		case ENEMY_STATUS::WALK:
		{
			enemy_status = "WALK";
			break;
		}
		case ENEMY_STATUS::ATTACK:
		{
			enemy_status = "ATTACK";
			break;
		}
		case ENEMY_STATUS::DEATH:
		{
			enemy_status = "DEATH";
			break;
		}
	}

	DrawFormatString(0, 460, GetColor(255, 0, 0), "MELEE_ENEMY_STATUS : %s", enemy_status);
}

// 攻撃コリジョン位置表示
void MeleeEnemy::DrawAttackColPos()
{
	DrawFormatString(0, 480, GetColor(255, 0, 0), "ATTACK_COL_TOP : (%.2f, %.2f, %.2f)", _attack_col_top.x, _attack_col_top.y, _attack_col_top.z);
	DrawFormatString(0, 500, GetColor(255, 0, 0), "ATTACK_COL_BOTTOM : (%.2f, %.2f, %.2f)", _attack_col_bottom.x, _attack_col_bottom.y, _attack_col_bottom.z);
	DrawFormatString(0, 520, GetColor(255, 0, 0), "ATTACK_COL_POS : (%.2f, %.2f, %.2f)", _collision_pos.x, _collision_pos.y, _collision_pos.z);

	// オフセット値を表示してデバッグ
	DrawFormatString(0, 540, GetColor(0, 255, 0), "ORBIT_OFFSET : (%.1f, %.1f, %.1f)", _orbit_offset.x, _orbit_offset.y, _orbit_offset.z);
	DrawFormatString(0, 560, GetColor(0, 255, 0), "ORBIT_CENTER : (%.1f, %.1f, %.1f)", _orbit_center.x, _orbit_center.y, _orbit_center.z);
}