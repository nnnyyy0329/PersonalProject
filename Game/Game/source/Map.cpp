#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Camera.h"

bool Map::Initialize()
{
	// マップモデルのロード
	_handleSkySphere = MV1LoadModel("res/SkySphere/skysphere.mv1"); // 空

	// マップ
	#if 1
		// ダンジョン
		_handleMap = MV1LoadModel("res/Dungeon/Dungeon.mv1");
		_frameMapCollision = MV1SearchFrame(_handleMap, "dungeon_collision");
	#else
		// フィールド
		_handleMap = MV1LoadModel("res/Ground/Ground.mv1");
		_frameMapCollision = MV1SearchFrame(_handleMap, "ground_navmesh");
	#endif

	// モデルのロードが成功したかチェック
	if (_handleMap == -1)
	{
		// エラーハンドリング
		return false;
	}

	// フレームが見つからなかった場合のチェック
	if (_frameMapCollision == -1)
	{
		// エラーハンドリング
		return false;
	}

	// コリジョン情報の生成
	MV1SetupCollInfo(_handleMap, _frameMapCollision, 16, 16, 16);

	// コリジョンのフレームを描画しない設定
	MV1SetFrameVisible(_handleMap, _frameMapCollision, FALSE);

	// 光源方向の設定
	_vLightDir = VGet(0.0f, 0.0f, 0.0f);

	// 腰位置の設定
	_colSubY = 40.f;

	// コリジョン表示
	_bViewCollision = TRUE;

	// クラスインスタンスの初期化
	for(auto& enemy : _enemies){ enemy = nullptr; }
	for(auto& bullet : _bullets){ bullet = nullptr; }
	_player = nullptr;
	_cam = nullptr;

	return true;
}

bool Map::Terminate()
{
	// マップモデルの削除
	MV1DeleteModel(_handleMap);

	return true;
}

bool Map::Process()
{
	return true;
}

bool Map::Render()
{
	// ライト設定
	SetUseLighting(TRUE);
	{
		_vLightDir = VGet(-1, -1, 0.5f);
		#if 1	// 平行ライト
		{
			SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f));
			ChangeLightTypeDir(_vLightDir);
		}
		#else	// ポイントライト
		{
			SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f));
			ChangeLightTypePoint(VAdd(_vPos, VGet(0, 50.f, 0)), 1000.f, 0.f, 0.005f, 0.f);
		}
		#endif
	}

	// コリジョン判定用ラインの描画
	if(_bViewCollision)
	{
		// プレイヤー
		{
			if(_player == nullptr) { return false; } // nullptrチェック

			// プレイヤー位置とコリジョン半径取得
			VECTOR player_pos = _player->GetPosition();
			float player_col_r = _player->GetColR();
			float player_life = _player->GetLife();

			// コリジョン判定用ライン,キャラコリジョン判定用円の描画
			{
				// プレイヤー
				if(player_life > 0)
				{
					//DrawLine3D
					//(
					//	VAdd(player_pos, VGet(0, _colSubY, 0)),
					//	VAdd(player_pos, VGet(0, -99999.f, 0)),
					//	GetColor(255, 0, 0)
					//);

					//DrawSphere3D
					//(
					//	player_pos, player_col_r, 10, GetColor(255, 0, 0), GetColor(0, 0, 0), FALSE
					//);
				}
			}
		}

		// エネミー
		{
			// エネミー位置とコリジョン半径取得
			for(auto& enemy : _enemies)
			{
				if(enemy == nullptr) { continue; } // nullptrチェック

				VECTOR enemy_pos = enemy->GetPosition();
				float enemy_col_r = enemy->GetColR();
				float enemy_life = enemy->GetLife();

				// コリジョン判定用ライン,キャラコリジョン判定用円の描画
				{
					// エネミー
					if(enemy_life > 0)
					{
						//DrawLine3D
						//(
						//	VAdd(enemy_pos, VGet(0, _colSubY, 0)),
						//	VAdd(enemy_pos, VGet(0, -99999.f, 0)),
						//	GetColor(255, 0, 0)
						//);

						//DrawSphere3D
						//(
						//	enemy_pos, enemy_col_r, 10, GetColor(255, 0, 0), GetColor(0, 0, 0), FALSE
						//);
					}
				}
			}
		}

		// 弾丸
		{
			for(auto& bullet : _bullets)
			{
				if(bullet == nullptr) { continue; } // nullptrチェック

				VECTOR bullet_pos = bullet->GetPosition();
				float bullet_col_r = bullet->GetColR();

				// コリジョン判定用ライン,コリジョン判定用円の描画
				{
					// 弾丸
					if(bullet->IsActive())
					{
						//DrawLine3D
						//(
						//	VAdd(bullet_pos, VGet(0, _colSubY, 0)),
						//	VAdd(bullet_pos, VGet(0, -99999.f, 0)),
						//	GetColor(255, 0, 0)
						//);

						//DrawSphere3D
						//(
						//	bullet_pos, bullet_col_r, 10, GetColor(255, 0, 0), GetColor(0, 0, 0), FALSE
						//);
					}
				}
			}
		}
	}

	// マップモデルの描画
	{
		MV1DrawModel(_handleMap);
		MV1DrawModel(_handleSkySphere); // スカイボックス描画
	}

	return true;
}
