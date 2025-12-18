#include "BulletEnemy.h"
#include "Bullet.h"
#include "ModeGame.h"
#include "Player.h"

bool BulletEnemy::Initialize()
{
	// 基底クラスの初期化を呼び出す
	if (!Enemy::Initialize()) { return false; }

	// モデルデータのロード
	_handle = MV1LoadModel("res/SDChar/SDChar.mv1");

	// ステータス
	_status = ENEMY_STATUS::NONE; // ステータスを「無し」に設定
	_enemy_ability = AbilityEnum::BULLET;   // 弾を撃つ能力に設定

	// 位置,向きの初期化
	//_vPos = VGet(100, 0, 0);
	_vDir = VGet(0, 0, -1);

	// カプセルコリジョン初期化
	_capsule_top = VGet(0.0f, 75.0f, 0.0f);
	_capsule_bottom = VGet(0.0f, 0.0f, 0.0f);
	_capsule_radius = 20.0f;

	// 他の初期化
	_life = 5.0f;
	_shotcnt = 0.0f;

	// 弾クラスのインスタンスを生成
	_bullet = nullptr;
	_modeGame = nullptr;
	_player = nullptr;

	return true;
}

bool BulletEnemy::Terminate()
{
	// モデルを削除する
	MV1DeleteModel(_handle);

	return true;
}

bool BulletEnemy::Process()
{
	// 敵の死亡処理
	DeathEnemy();

	if(_life <= 0){ return false; }

	// 敵の行動処理
	EnemyStatusMove();

	// 敵の回転処理
	EnemyRotate();

	// 弾を撃つ処理
	ShotBullet();

	return true;
}

bool BulletEnemy::Render()
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

	return true;
}

// 敵の死亡処理
void BulletEnemy::DeathEnemy()
{
	// 敵の体力が0以下なら死亡処理
	if(_life <= 0.0f)
	{
		_is_dead = true; // 死亡フラグを立てる

		// アニメーションがアタッチされていたら、デタッチする
		if (_attach_index != -1)
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
	if (_play_time >= _total_time)
	{
		_play_time = 0.0f; // 再生時間の初期化
	}
}

// 敵の行動処理
void BulletEnemy::EnemyStatusMove()
{
	// 処理前のステータスを保存しておく
	ENEMY_STATUS oldStatus = _status;

	// 移動量をそのままキャラの向きにする
	if(_is_moved) // 移動していない時は無視するため
	{
		_status = ENEMY_STATUS::WALK; // ステータスを歩行にする
	}
	else // 移動していない
	{
		_status = ENEMY_STATUS::WAIT; // ステータスを待機にする
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
		if (_attach_index != -1)
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
void BulletEnemy::EnemyRotate()
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
	if(VSize(toPlayer) > 0.0f) // ゼロ除算防止
	{
		_vDir = VNorm(toPlayer); // 正規化
	}
}

// 弾を撃つ
void BulletEnemy::ShotBullet()
{
	// 敵の動きによる弾の処理
	switch(_status)
	{
		case ENEMY_STATUS::WAIT: // 待機
		case ENEMY_STATUS::WALK: // 歩行
		{
			// 射撃カウントを進める
			_shotcnt += 1.0f;

			// 一定時間経過したら弾を発射する
			if(_shotcnt >= 120.0f) // 1秒に1回発射
			{
				// ModeGameから使用可能な弾丸を取得
				if(_modeGame != nullptr)
				{
					// 使用可能な弾丸を取得
					Bullet* activeBullet = _modeGame->GetAvailableBullet();
					if (activeBullet != nullptr) // 使用可能な弾丸がある場合
					{
						// 弾の初期化
						activeBullet->Initialize
						(
							VGet(_vPos.x, _vPos.y + 50, _vPos.z), _vDir, 5.0f, BULLET_TYPE::ENEMY
						);
					}
				}

				// 射撃カウントをリセット
				_shotcnt = 0.0f;
			}

			break;
		}
	}
}

// コリジョン描画
void BulletEnemy::DrawEnemyCollision()
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
