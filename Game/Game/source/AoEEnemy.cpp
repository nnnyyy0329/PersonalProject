#include "AoEEnemy.h"
#include "ModeGame.h"
#include "Player.h"

bool AoEEnemy::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if(!Enemy::Initialize()) { return false; }

	// モデルデータのロード
	_handle = MV1LoadModel("res/SDChar/SDChar.mv1");

	// ステータス
	_status = ENEMY_STATUS::NONE;	   // ステータスを「無し」に設定
	_enemy_ability = AbilityEnum::AOE; // 範囲攻撃に設定

	// 位置,向きの初期化
	//_vPos = VGet(-50, 0, 240);
	_vDir = VGet(0, 0, -1);

	// カプセルコリジョン初期化
	{
		_capsule_top = VGet(0.0f, 75.0f, 0.0f);
		_capsule_bottom = VGet(0.0f, 0.0f, 0.0f);
		_capsule_radius = 20.0f;
	}

	// 円形床コリジョン初期化
	{
		_circleFloor.center = _vPos;  // 円の中心位置
		_circleFloor.radius = 150.0f; // 円の半径
		_circleFloor.height = 0.0f;	  // 円の高さ
	}

	// 他の初期化
	{
		_life = 5.0f;
		_moveSpeed = 1.0f;
		_cheise_player = false;
		_is_start_aoe_attack = false;
	}

	// 弾クラスのインスタンスを生成
	_modeGame = nullptr;
	_player = nullptr;

	return true;
}

bool AoEEnemy::Terminate()
{
	// モデルを削除する
	MV1DeleteModel(_handle);

	return true;
}

bool AoEEnemy::Process()
{
	// 敵の死亡処理
	DeathEnemy();

	if(_life <= 0){ return false; }

	// 敵の行動処理
	EnemyStatusMove();

	// 敵の回転処理
	EnemyRotate();
	
	// プレイヤー追尾処理
	CheisePlayer();

	// 円形床コリジョン判定処理
	CheckCircularFloorCol();

	// 範囲攻撃処理
	ProcessAttack();

	return true;
}

bool AoEEnemy::Render()
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

	// 円形床描画
	DrawCircleFloor();

	// 攻撃範囲円形床描画
	DrawAttackCircleFloor();

	// 範囲攻撃範囲描画
	DrawAoEAttackRange();

	// 敵のステータス表示関数
	//DrawEnemyStatus();

	return true;
}

// 敵の死亡処理
void AoEEnemy::DeathEnemy()
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
void AoEEnemy::EnemyStatusMove()
{
	// 処理前のステータスを保存しておく
	ENEMY_STATUS oldStatus = _status;

	// ステータスの更新
	if(!_is_start_aoe_attack) // 範囲攻撃開始フラグが立っていないなら
	{
		_status = ENEMY_STATUS::WALK; // ステータスを歩行にする
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

			case ENEMY_STATUS::ATTACK: // 攻撃
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
void AoEEnemy::EnemyRotate()
{
	if(_player == nullptr) return;
	if(_is_start_aoe_attack) return;

	// 位置を取得
	VECTOR playerPos = _player->GetPosition();

	// プレイヤーの方向ベクトルを計算
	VECTOR toPlayer = VSub(playerPos, _vPos);

	// 敵が上下を向かないように
	toPlayer.y = 0;

	// 正規化
	if(VSize(toPlayer) > 0.0f) // ゼロ除算防止
	{
		_vDir = VNorm(toPlayer); // 正規化
	}
}

// プレイヤーを追尾する
void AoEEnemy::CheisePlayer()
{
	if(_player == nullptr) return;
	if(_is_start_aoe_attack) return;

	// 位置を取得
	VECTOR playerPos = _player->GetPosition();

	// プレイヤーの方向ベクトルを計算
	VECTOR toPlayer = VSub(playerPos, _vPos);

	// 敵が上下を向かないように
	toPlayer.y = 0; 

	// 正規化
	if(VSize(toPlayer) > 0.0f) // ゼロ除算防止
	{
		// 正規化
		VECTOR norm = VNorm(toPlayer); 

		// 移動ベクトルを計算
		VECTOR move_v = VScale(norm, _moveSpeed);

		// 位置更新
		_vPos = VAdd(_vPos, move_v);
		_cheise_player = true;
	}
}

// 円形攻撃処理
void AoEEnemy::AttackMoveCircle
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
void AoEEnemy::ProcessAttack()
{
	// 範囲攻撃開始フラグが立っていて、ステータスが攻撃でないなら
	if(_is_start_aoe_attack && _status != ENEMY_STATUS::ATTACK) 
	{
		_status = ENEMY_STATUS::ATTACK; // ステータスを攻撃にする
		_attack_timer = 0.0f;			// 攻撃タイマー初期化

		// 円形攻撃処理セット
		AttackMoveCircle
		(
			_circleFloor.center, // 円の中心位置
			_circleFloor.radius, // 円の半径  
			_circleFloor.height, // 円の高さ
			60.0f,				 // 攻撃開始までの遅延時間
			15.0f,				 // 攻撃持続時間
			60.0f,				 // 攻撃後の後隙
			1.0f,				 // ダメージ量
			true				 // 攻撃中フラグ
		);
	}

	// 攻撃中の処理
	if(_status == ENEMY_STATUS::ATTACK) // 攻撃状態中
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
			_status = ENEMY_STATUS::WAIT; // ステータスを待機に戻す
			_is_attacking = false;		  // 攻撃中フラグを下ろす
			_is_start_aoe_attack = false; // 攻撃開始フラグを下ろす
			_attack_timer = 0.f;		  // 攻撃タイマーを初期化
		}
	}
}

// 攻撃ヒット判定処理
void AoEEnemy::CheckAttackHit()
{
	if(_player == nullptr) return;
	if(!_is_attacking) return;

	// プレイヤーの位置を取得
	VECTOR playerPos = _player->GetPosition();

	// プレイヤーが攻撃範囲内にいるかチェック
	if(_attack_circle.IsPointInside(playerPos))
	{
		// プレイヤーにダメージを与える
		_player->DamagePlayer();
	}
}

// 円形床判定用メソッド
void AoEEnemy::CheckCircularFloorCol()
{
	VECTOR playerPos = _player->GetPosition(); // プレイヤーの位置取得
	
	_circleFloor.center = _vPos; // 円の中心位置

	// プレイヤー現在位置が円内にあるかチェック
	if(_circleFloor.IsPointInside(playerPos)) // 円内にいる
	{
		_is_start_aoe_attack = true; // 範囲攻撃開始フラグを立てる
	}
}

// コリジョン描画
void AoEEnemy::DrawEnemyCollision()
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

// 円形床描画
void AoEEnemy::DrawCircleFloor()
{
	// 円形床描画
	const int segments = 64; // 円の分割数
	const float angleStep = 2.0f * DX_PI_F / segments;

	for(int i = 0; i < segments; ++i)
	{
		float angle1 = i * angleStep;
		float angle2 = (i + 1) * angleStep;

		// 円周上の2点を計算
		{
			// 1点目
			VECTOR p1 = VGet
			(
				_circleFloor.center.x + cos(angle1) * _circleFloor.radius,
				_circleFloor.height,
				_circleFloor.center.z + sin(angle1) * _circleFloor.radius
			);

			// 2点目
			VECTOR p2 = VGet
			(
				_circleFloor.center.x + cos(angle2) * _circleFloor.radius,
				_circleFloor.height,
				_circleFloor.center.z + sin(angle2) * _circleFloor.radius
			);

			DrawLine3D(p1, p2, GetColor(0, 255, 0)); // 緑色で描画
		}
	}

	// 中心点を描画
	DrawSphere3D(_circleFloor.center, 5.0f, 8, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);
}

// 攻撃範囲円形床描画
void AoEEnemy::DrawAttackCircleFloor()
{
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
				_circleFloor.center.x + cos(angle1) * _circleFloor.radius, // X座標
				_circleFloor.height + 1.0f,								   // 少し上にずらす
				_circleFloor.center.z + sin(angle1) * _circleFloor.radius  // Z座標
			);

			// 2点目
			VECTOR p2 = VGet
			(
				_circleFloor.center.x + cos(angle2) * _circleFloor.radius,
				_circleFloor.height + 1.0f, // 少し上にずらす
				_circleFloor.center.z + sin(angle2) * _circleFloor.radius
			);

			DrawLine3D(p1, p2, GetColor(255, 0, 0)); // 赤色で描画
		}
	}
	// 中心点を描画
	DrawSphere3D(_circleFloor.center, 5.0f, 8, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);
}

// 範囲攻撃範囲描画
void AoEEnemy::DrawAoEAttackRange()
{
	if(!_is_attacking) return;

	// 攻撃範囲円形床描画
	const int segments = 64; // 円の分割数
	const float angleStep = 2.0f * DX_PI_F / segments;

	// 外周を描画
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
				_attack_circle.center.x + cos(angle2) * _attack_circle.radius,
				_attack_circle.height + 1.0f, // 少し上にずらす
				_attack_circle.center.z + sin(angle2) * _attack_circle.radius
			);

			int color = _is_attacking ? GetColor(255, 100, 100) : GetColor(150, 50, 50);
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
				_attack_circle.height + 2.0f,
				_attack_circle.center.z
			);

			// 外周点
			VECTOR edgePoint = VGet
			(
				_attack_circle.center.x + cos(angle) * _attack_circle.radius,
				_attack_circle.height + 2.0f,
				_attack_circle.center.z + sin(angle) * _attack_circle.radius
			);

			DrawLine3D(centerPoint, edgePoint, GetColor(255, 100, 100));
		}

		// 攻撃中心点を描画
		//DrawSphere3D(_attack_circle.center, 8.0f, 16, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);
	}
}

// 敵のステータス表示関数
void AoEEnemy::DrawEnemyStatus()
{
	// 範囲攻撃を開始したかどうか表示
	if(_is_start_aoe_attack)
	{
		DrawString(10, 540, "AoE Attack Started", GetColor(255, 0, 0));
	}
	else
	{
		DrawString(10, 540, "AoE Attack Not Started", GetColor(255, 0, 0));
	}

	// 敵の現在のステータスを表示
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

		DrawFormatString(0, 560, GetColor(255, 0, 0), "MELEE_ENEMY_STATUS : %s", enemy_status);
	}
}