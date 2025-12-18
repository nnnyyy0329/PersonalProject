#include "ShieldEnemy.h"
#include "ModeGame.h"
#include "Player.h"

bool ShieldEnemy::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if(!Enemy::Initialize()) { return false; }

	// モデルデータのロード
	_handle = MV1LoadModel("res/SDChar/SDChar.mv1");

	// ステータス
	_status = ENEMY_STATUS::NONE;		  // ステータスを「無し」に設定
	_enemy_ability = AbilityEnum::SHIELD; // シールド能力に設定

	// 位置,向きの初期化
	//_vPos = VGet(-350, 0, 0);
	_vDir = VGet(0, 0, -1);

	// カプセルコリジョン初期化
	{
		_capsule_top = VGet(0.0f, 75.0f, 0.0f);
		_capsule_bottom = VGet(0.0f, 0.0f, 0.0f);
		_capsule_radius = 20.0f;
	}

	// 床判定初期化
	{
		_circleFloor.center = _vPos;  // 円の中心位置
		_circleFloor.radius = 150.0f; // 円の半径
		_circleFloor.height = 0.0f;	  // 円の高さ
	}

	// 他の初期化
	_moveSpeed = 2.0f;
	_life = 5.0f;
	_rotation_speed = 0.025f;

	// 弾クラスのインスタンスを生成
	_modeGame = nullptr;
	_player = nullptr;

	return true;
}

bool ShieldEnemy::Terminate()
{
	// モデルを削除する
	MV1DeleteModel(_handle);

	return true;
}

bool ShieldEnemy::Process()
{
	// 敵の死亡処理
	DeathEnemy();

	if(_life <= 0){ return false; }

	// シールド処理
	ProcessShield();

	// 敵の行動処理
	EnemyStatusMove();

	// 敵の回転処理
	EnemyRotate();

	// 移動停止処理
	MoveStop();

	// プレイヤー追尾処理
	CheisePlayer();

	return true;
}

bool ShieldEnemy::Render()
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

	// シールドコリジョン描画
	DrawShieldCollision();

	// ステータス表示
	//DrawStatus();

	// 円形床描画
	//DrawCircleFloor();

	return true;
}

// 敵の死亡処理
void ShieldEnemy::DeathEnemy()
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
void ShieldEnemy::EnemyStatusMove()
{
	// 処理前のステータスを保存しておく
	ENEMY_STATUS oldStatus = _status;

	// 移動量をそのままキャラの向きにする
	if(_is_moved) // 移動していない時は無視するため
	{
		_status = ENEMY_STATUS::WALK; // ステータスを歩行にする
	}
	else
	{
		_status = ENEMY_STATUS::SHIELD;	// ステータスをシールドにする
	}

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
			case ENEMY_STATUS::WAIT: // 待機
			{
				// アイドルアニメーションをアタッチする
				_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
				break;
			}

			case ENEMY_STATUS::WALK: // 歩行
			{
				// 歩行アニメーションをアタッチする
				_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "run"), -1, FALSE);
				break;
			}
			case ENEMY_STATUS::SHIELD:
			{
				// 攻撃アニメーションをアタッチする
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
void ShieldEnemy::EnemyRotate()
{
	// プレイヤーの参照が無ければ処理しない
	if(_player == nullptr) return;

	// 位置を取得
	VECTOR playerPos = _player->GetPosition();

	// プレイヤーの方向ベクトルを計算
	VECTOR toPlayer = VSub(playerPos, _vPos);

	// 敵が上下を向かないように
	toPlayer.y = 0;

	// 正規化
	VECTOR target_dir = VGet(0, 0, 0);
	if(VSize(toPlayer) > 0.0f) // ゼロ除算防止
	{
		target_dir = VNorm(toPlayer); // 正規化
	}

	// ターゲットの向きに徐々に回転
	float dot_product = VDot(_vDir, target_dir);

	// 内積が1に近い場合は処理をスキップ
	if(dot_product >= 0.999f)
	{
		return;
	}

	// 外積で回転方向を求める
	VECTOR cross = VCross(_vDir, target_dir);

	// 回転角度を制限
	float angle = acos(abs(dot_product));
	float max_rotation = _rotation_speed;
	if(angle > max_rotation)
	{
		angle = max_rotation;
	}

	// 回転方向を決定
	if(cross.y < 0)
	{
		angle = -angle;
	}

	// 現在の向きをy軸回転
	float current_yaw=atan2(_vDir.x, _vDir.z);
	float new_yaw = current_yaw + angle;

	// 向きを決定
	_vDir.x = sin(new_yaw);
	_vDir.z = cos(new_yaw);
	_vDir.y = 0;

	// 正規化
	_vDir = VNorm(_vDir);
}

// シールド処理
void ShieldEnemy::ShieldMove
(
	VECTOR col_top,
	VECTOR col_bottom,
	float col_r,
	float damage,
	bool is_attacking
)
{
	// 向きを正規化
	VECTOR dir_norm = VNorm(_vDir);

	// コリジョンを前方に配置
	VECTOR attack_offset = VScale(dir_norm, 40.0f);

	// コリジョン位置
	VECTOR col_offset = VAdd(_vPos, attack_offset);

	// 攻撃コリジョン情報を保存
	_shield_col_top = VAdd(col_offset, col_top);	   // シールドコリジョン上端
	_shield_col_bottom = VAdd(col_offset, col_bottom); // シールドコリジョン下端
	_shield_col_r = col_r;
	_damage = damage;
	_is_attacking = is_attacking;
}

// シールド処理
void ShieldEnemy::ProcessShield()
{
	_circleFloor.center = _vPos; // 円形床の中心位置を更新

	// シールドコリジョンを設定
	ShieldMove
	(
		VGet(0.0f, 70.0f, 0.0f),  // シールドコリジョン上端
		VGet(0.0f, 40.0f, 0.0f),  // シールドコリジョン下端
		20.0f,                    // シールドコリジョン半径
		1.0f,                     // ダメージ量
		false					  // 攻撃中フラグ
	);

	if(!_is_moved)
	{
		_status = ENEMY_STATUS::SHIELD;	// ステータスをシールドに設定
		_is_shielding = true;			// シールド中フラグを立てる
	}

	if(_status == ENEMY_STATUS::SHIELD)
	{
		CheckShieldHit(); // シールドヒット判定処理
	}
}

// シールドヒット判定処理
void ShieldEnemy::CheckShieldHit()
{
	if(_player == nullptr) return;	// プレイヤーの参照が無ければ処理しない

	bool is_player_attacking = _player->GetIsAttacking();				// 攻撃中フラグ

	// プレイヤーが攻撃中の場合に当たり判定を行う
	if(!is_player_attacking)
	{
		_is_shield_success = false; 
		return; 
	}

	if(is_player_attacking)
	{
		// プレイヤーの攻撃コリジョン情報を取得
		VECTOR player_attack_col_top = _player->GetAttackColTop();			// 攻撃コリジョン上端
		VECTOR playerr_attack_col_bottom = _player->GetAttackColBottom();	// 攻撃コリジョン下端
		float playerr_attack_col_r = _player->GetAttackColR();				// 攻撃コリジョン半径

		// カプセル同士の当たり判定
		if(HitCheck_Capsule_Capsule
		(
			_shield_col_top,			// シールドコリジョン上端
			_shield_col_bottom,			// シールドコリジョン下端
			_shield_col_r,				// シールドコリジョン半径
			player_attack_col_top,		// プレイヤー攻撃コリジョン上端
			playerr_attack_col_bottom,	// プレイヤー攻撃コリジョン下端
			playerr_attack_col_r		// プレイヤー攻撃コリジョン半径
		) != false)
		{
			_is_shield_success = true;	// シールド成功フラグを立てる
		}
	}
}

// 移動停止処理
void ShieldEnemy::MoveStop()
{
	if(_player == nullptr){ return; }

	// プレイヤーの位置を取得
	VECTOR playerPos = _player->GetPosition();

	// プレイヤーとの距離を計算
	VECTOR toPlayer = VSub(playerPos, _circleFloor.center);
	toPlayer.y = 0; // 高さ成分を無視

	float distance = VSize(toPlayer);

	// 円形床の範囲内にいる場合、移動を停止
	if(distance <= _circleFloor.radius)
	{
		_is_moved = false; // 移動フラグを下ろす
	}
	else
	{
		_is_moved = true; // 移動フラグを立てる
	}
}

// プレイヤーを追尾する
void ShieldEnemy::CheisePlayer()
{
	if(_player == nullptr){ return; }
	if(_is_moved == false){ return; }

	// 正規化
	VECTOR dir_norm = VNorm(_vDir); 

	// 移動ベクトルを計算
	VECTOR move_v = VScale(dir_norm, _moveSpeed); 

	// 位置を更新
	_vPos = VAdd(_vPos, move_v);
}

// コリジョン描画
void ShieldEnemy::DrawEnemyCollision()
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

// シールドコリジョン描画
void ShieldEnemy::DrawShieldCollision()
{
	// 攻撃コリジョンを描画
	DrawCapsule3D
	(
		_shield_col_top,		 // カプセルコリジョン上端
		_shield_col_bottom,		 // カプセルコリジョン下端
		_shield_col_r,			 // カプセルコリジョン半径
		8,						 // 分割数
		GetColor(255, 0, 0),     // 赤色
		GetColor(255, 255, 255), // 白色
		FALSE					 // 塗りつぶし
	);
}

// ステータス表示関数
void ShieldEnemy::DrawStatus()
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
		case ENEMY_STATUS::SHIELD:
		{
			enemy_status = "SHIELD";
			break;
		}
		case ENEMY_STATUS::DEATH:
		{
			enemy_status = "DEATH";
			break;
		}
	}

	DrawFormatString(0, 640, GetColor(255, 0, 0), "SHIELD_ENEMY_STATUS : %s", enemy_status);

	// シールド成功フラグ表示
	DrawFormatString(0, 660, GetColor(255, 0, 0), "SHIELD_SUCCESS : %s", _is_moved ? "TRUE" : "FALSE");
}

// 円形床描画
void ShieldEnemy::DrawCircleFloor()
{
	const int segments = 64; // 分割数
	const float angleStep = 2.0f * DX_PI_F / segments; // 角度のステップ

	for(int i = 0; i < segments; ++i)
	{
		float angle1 = i * angleStep;
		float angle2 = (i + 1) * angleStep;

		// 円周上の2点を計算
		{
			// 1点目
			VECTOR p1 = VGet
			(
				_circleFloor.center.x + cos(angle1) * _circleFloor.radius, // X座標
				_circleFloor.height,									   // 床の高さ
				_circleFloor.center.z + sin(angle1) * _circleFloor.radius  // Z座標
			);

			// 2点目
			VECTOR p2 = VGet
			(
				_circleFloor.center.x + cos(angle2) * _circleFloor.radius, // X座標
				_circleFloor.height,									   // 床の高さ
				_circleFloor.center.z + sin(angle2) * _circleFloor.radius  // Z座標
			);

			// プレイヤーが範囲内にいるかどうかで色を変える
			int color = _is_moved ? GetColor(255, 255, 0) : GetColor(255, 0, 0); // 黄色 or 赤色
			DrawLine3D(p1, p2, color);
		}
	}
}