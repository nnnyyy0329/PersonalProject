#include "AppFrame.h"
#include "ApplicationMain.h"
#include "ModeGame.h"
#include "ModeEffekseer.h"

// Character
#include "Player.h"

#include "Enemy.h"
#include "BulletEnemy.h"
#include "MeleeEnemy.h"
#include "AoEEnemy.h"
#include "ShieldEnemy.h"

#include "Ability.h"
#include "AbilityBaseAttack.h"
#include "AbilityBullet.h"
#include "AbilityMelee.h"
#include "AbilityAoE.h"
#include "AbilityShield.h"

#include "Bullet.h"
#include "Camera.h"
#include "Map.h"

namespace
{
	const float SPAWN_RANGE_X = 300.0f; // エネミー生成範囲X
	const float SPAWN_RANGE_Z = 300.0f; // エネミー生成範囲Z
	const float SPAWN_HEIGHT = 0.0f;    // エネミー生成高さ
}

bool ModeGame::Initialize()
{
	if(!base::Initialize()) { return false; }

	// シャドウマップ用初期化
	{
		_handleShadowMap = MakeShadowMap(2048, 2048);
		_bViewShadowMap = false;
	}

	// 初期化処理
	{
		_gamecnt = 0;				// ゲームカウント初期化
		_spawncnt = 0;				// エネミースポーンカウント初期化
		_bullet_damage = 1.0f;		// 弾のダメージ初期化
		_attack_cooltime = 0.0f;	// 攻撃クールタイム初期化
	}

	// エネミーの生成
	{
		SpawnEnemies(3, 1, 2, 2);
	}

	// アビリティの生成
	{
		// 初期アビリティ
		{
			auto baseAbility = std::make_shared<AbilityBaseAttack>();
			//_abilities.push_back(baseAbility);
		}

		// 弾発射アビリティ
		{
			auto bulletAbility = std::make_shared<AbilityBullet>();
			_abilities.push_back(bulletAbility);
		}

		// 近接攻撃アビリティ
		{
			auto meleeAbility = std::make_shared<AbilityMelee>();
			_abilities.push_back(meleeAbility);
		}

		// 範囲攻撃アビリティ
		{
			auto aoeAbility = std::make_shared<AbilityAoE>();
			_abilities.push_back(aoeAbility);
		}

		// シールドアビリティ
		{
			auto aoeShield = std::make_shared<AbilityShield>();
			//_abilities.push_back(aoeShield);
		}
	}

	// オブジェクトのイニシャライズ読み込み
	{
		_player = std::make_shared<Player>();
		_player->Initialize();

		_cam = std::make_shared<Camera>();
		_cam->Initialize();

		_map = std::make_shared<Map>();
		_map->Initialize();

		for(auto& ability : _abilities)
		{
			ability->Initialize();
		}

		for(auto i = 0; i < BULLET_MAX; ++i)
		{
			auto bullet = std::make_shared<Bullet>();
			bullet->Initialize();
			_bullets.push_back(bullet);
		}
	}

	return true;
}

bool ModeGame::Terminate() 
{
	base::Terminate();

	// オブジェクトのターミネート読み込み
	{
		_player->Terminate(); // プレイヤーの削除
		_cam->Terminate(); // カメラの削除
		_map->Terminate(); // マップの削除

		// エネミーの削除
		for (auto& enemy : _enemies)
		{
			enemy->Terminate();
		}
		_enemies.clear();

		// アビリティの削除
		for(auto& ability : _abilities)
		{
			ability->Initialize();
		}
		_abilities.clear();

		// 弾丸の削除
		for (auto& bullet : _bullets)
		{
			bullet->Terminate();
		}
		_bullets.clear();
	}

	return true;
}

bool ModeGame::Process()
{
	base::Process();

	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// ゲームカウント更新
	{
		++_gamecnt;
	}

	// クラスをセット
	{
		// エネミーのポインタ配列を作成
		std::vector<Enemy*> enemyPtrs;
		for(auto& enemy : _enemies)
		{
			enemyPtrs.push_back(enemy.get()); // エネミーのポインタを追加
		}

		// アビリティののポインタ配列を作成
		std::vector<Ability*>abilityPtrs;
		for(auto& ability : _abilities)
		{
			abilityPtrs.push_back(ability.get()); // アビリティのポインタ追加
		}

		// 弾丸のポインタ配列を作成
		std::vector<Bullet*>bulletPtrs;
		for(auto& bullet : _bullets)
		{
			bulletPtrs.push_back(bullet.get()); // アビリティのポインタ追加
		}

		// プレイヤー
		{
			_player->SetModeGame(this);
			_player->SetEnemy(enemyPtrs);
			_player->SetCamera(_cam.get());
			_player->SetMap(_map.get());
		}
		
		// カメラ
		{
			_cam->SetPlayer(_player.get());
		}
		
		// マップ
		{
			_map->SetPlayer(_player.get());
			_map->SetEnemy(enemyPtrs);
			_map->SetBullet(bulletPtrs);
			_map->SetCamera(_cam.get());
		}
		
		// 敵にクラス参照を設定
		for(auto& enemy : _enemies)
		{
			// 敵がBulletEnemy型の場合
			if(auto bulletEnemy = dynamic_cast<BulletEnemy*>(enemy.get()))
			{
				bulletEnemy->SetModeGame(this);		   // モードゲームセット
				bulletEnemy->SetPlayer(_player.get()); // プレイヤーセット
			}
		
			// 敵がMeleeEnemy型の場合
			if(auto meleeEnemy = dynamic_cast<MeleeEnemy*>(enemy.get()))
			{
				meleeEnemy->SetModeGame(this);		  // モードゲームセット
				meleeEnemy->SetPlayer(_player.get()); // プレイヤーセット
			}
		
			// 敵がAoEEnemy型の場合
			if(auto aoeEnemy = dynamic_cast<AoEEnemy*>(enemy.get()))
			{
				aoeEnemy->SetModeGame(this);		// モードゲームセット
				aoeEnemy->SetPlayer(_player.get()); // プレイヤーセット
			}
		
			// 敵がShieldEnemy型の場合
			if(auto shieldEnemy = dynamic_cast<ShieldEnemy*>(enemy.get()))
			{
				shieldEnemy->SetModeGame(this);		   // モードゲームセット
				shieldEnemy->SetPlayer(_player.get()); // プレイヤーセット
			}
		}
		
		// アビリティにクラス参照を設定
		for(auto& ability : _abilities)
		{
			// 初期アビリティ
			if(auto abilityBase = dynamic_cast<AbilityBaseAttack*>(ability.get()))
			{
				abilityBase->SetModeGame(this);
			}
		
			// 弾発射アビリティの場合
			if(auto abilityBullet = dynamic_cast<AbilityBullet*>(ability.get()))
			{
				//abilityBullet->SetModeGame(this);		 // モードゲームセット
				//abilityBullet->SetPlayer(_player.get()); // プレイヤーセット
			}
		
			// 近接攻撃アビリティの場合
			if(auto abilityMelee = dynamic_cast<AbilityMelee*>(ability.get()))
			{
				//abilityMelee->SetModeGame(this);		// モードゲームセット
				//abilityMelee->SetPlayer(_player.get()); // プレイヤーセット
			}
		
			// 範囲攻撃アビリティの場合
			if(auto abilityAoE = dynamic_cast<AbilityAoE*>(ability.get()))
			{
				//abilityAoE->SetEnemy(enemyPtrs);	  // エネミーセット
				//abilityAoE->SetModeGame(this);		  // モードゲームセット
				//abilityAoE->SetPlayer(_player.get()); // プレイヤーセット
			}
		
			// シールドアビリティの場合
			if(auto abilityShield = dynamic_cast<AbilityShield*>(ability.get()))
			{
				//abilityShield->SetPlayer(_player.get()); // プレイヤーセット
			}
		}
		
		// 弾丸にクラス参照を設定
		for(auto& bullet : _bullets)
		{
			bullet->SetMap(_map.get());
		}
	}

	// オブジェクトのプロセス読み込み
	{
		_player->Process();
		//_cam->Process();

		for(auto& enemy : _enemies)
		{
			enemy->Process();
		}

		for(auto& bullet : _bullets)
		{
			bullet->Process();
		}
	}

	// 当たり判定
	{
		// 弾丸の当たり判定
		for (auto& bullet : _bullets)
		{
			// 敵の弾丸のみ判定
			if (bullet->IsActive() && bullet->GetBulletType() == BULLET_TYPE::ENEMY)
			{
				// 当たり判定処理
				HitPlayerCapsuleBullet(_player.get(), bullet.get());
			}

			// プレイヤーの弾丸のみ判定
			if (bullet->IsActive() && bullet->GetBulletType() == BULLET_TYPE::PLAYER)
			{
				for(auto& enemy : _enemies)
				{		
					// 当たり判定処理
					HitEnemyCapsuleBullet(enemy.get(), bullet.get());
				}
			}
		}

		// プレイヤーと敵の当たり判定
		for (auto& enemy : _enemies)
		{
			// 当たり判定処理
			HitPlayerEnemy(_player.get(), enemy.get());
		}

		// シールドと弾丸の当たり判定
		for(auto& ability : _player->GetPlayerAbilities()) // プレイヤーのアビリティ配列を取得
		{
			// シールドアビリティの場合
			if(auto abilityShield = dynamic_cast<AbilityShield*>(ability.get()))
			{
				for (auto& bullet : _bullets)
				{
					HitShieldCapsuleBullet(abilityShield, bullet.get());
				}
			}
		}

		// エネミーとシールドと弾丸の当たり判定
		for(auto& enemy : _enemies)
		{
			for (auto& bullet : _bullets)
			{
				if(enemy == nullptr || bullet == nullptr) continue;

				// 敵の弾丸のみ判定
				if (bullet->IsActive() && bullet->GetBulletType() == BULLET_TYPE::PLAYER)
				{
					HitEnemyShieldBullet(enemy.get(), bullet.get());
				}
			}
		}

		// シールドとエネミー攻撃の当たり判定
		for(auto& ability : _player->GetPlayerAbilities()) // プレイヤーのアビリティ配列を取得
		{
			// シールドアビリティの場合
			if(auto abilityShield = dynamic_cast<AbilityShield*>(ability.get()))
			{
				for(auto& enemy : _enemies)
				{
					if(ability == nullptr || enemy == nullptr) continue;	

					HitShieldColEnemyAttackCol(abilityShield, enemy.get());
				}
			}
		}

		// 近接判定とエネミーの当たり判定
		for(auto& ability : _player->GetPlayerAbilities()) // プレイヤーのアビリティ配列を取得
		{
			// 近接攻撃アビリティの場合
			if(auto abilityMelee = dynamic_cast<AbilityMelee*>(ability.get()))
			{
				for(auto& enemy : _enemies)
				{
					if(ability == nullptr || enemy == nullptr) continue;	
					HitMeleeColEnemy(abilityMelee, enemy.get());
				}
			}
		}
	}

	// その他の処理
	{
		DecCooltime();		// 攻撃クールタイム減少処理
		CheckDeathEnemy();	// 敵のライフが0になったらカウントを減らす処理
		OpenMenu();			// メニュー画面を開く処理
	}

	return true;
}

bool ModeGame::Render()
{
	base::Render();

	// 3D基本設定
	{
		SetUseZBuffer3D(TRUE);
		SetWriteZBuffer3D(TRUE);
		SetUseBackCulling(TRUE);
	}

	// オブジェクトのレンダー読み込み
	{
		_map->Render();
		_player->Render();
		_cam->Render();

		for (auto& enemy : _enemies)
		{
			enemy->Render();
		}

		for(auto& ability : _abilities)
		{
			ability->Render();
		}

		// 弾丸の描画
		for (auto& bullet : _bullets)
		{
			bullet->Render();
		}

		// シャドウマップ描画
		DrawShadowMap();
	}

	// デバッグ表示
	{
		// プレイヤー
		{
			//// プレイヤー座標
			//{
			//	// プレイヤーの座標を取得して表示
			//	VECTOR player_vPos = _player->GetPosition();
			//	std::string playerPos = "player_pos:("
			//		+ std::to_string(player_vPos.x) + ","
			//		+ std::to_string(player_vPos.y) + ","
			//		+ std::to_string(player_vPos.z) + ")";

			//	DrawString(0, 100, playerPos.c_str(), GetColor(0, 255, 0));
			//}

			//// プレイヤーのカプセルコリジョン情報を取得
			//VECTOR player_pos = _player->GetPosition();
			//VECTOR capsuleTop = VAdd(_player->GetPosition(), _player->GetCapsuleTop());
			//VECTOR capsuleBottom = VAdd(_player->GetPosition(), _player->GetCapsuleBottom());
			//float capsuleRadius = _player->GetCapsuleRadius();

			//// カプセルコリジョンの描画
			//DrawCapsule3D
			//(
			//	capsuleTop,
			//	capsuleBottom,
			//	capsuleRadius,
			//	8,
			//	GetColor(25, 0, 0),
			//	GetColor(5, 55, 255),
			//	TRUE
			//);
		}

		// 敵
		for (auto& enemy : _enemies)
		{			
			// 敵がBulletEnemy型の場合
			auto bulletEnemy = dynamic_cast<BulletEnemy*>(enemy.get());
			{
				if (bulletEnemy != nullptr) // BulletEnemyが有効な場合
				{
					std::string bEnemyLife = "bulletEnemyLife:" + std::to_string(bulletEnemy->GetLife());
					//DrawString(0, 320, bEnemyLife.c_str(), GetColor(255, 0, 0));

					const char* enemyAbility = "";
					switch(bulletEnemy->_abilityEnum)
					{
						case BulletEnemy::AbilityEnum::NONE:
						{
							enemyAbility = "None";
							break;
						}
						case BulletEnemy::AbilityEnum::MELEE:
						{
							enemyAbility = "Melee";
							break;
						}
						case BulletEnemy::AbilityEnum::BULLET:
						{
							enemyAbility = "Bullet";
							break;
						}
					}
					std::string bEnemyAbility = "bulletEnemyAbility:" + std::string(enemyAbility);
					//DrawString(0, 360, bEnemyAbility.c_str(), GetColor(255, 0, 0));
				}
			}	

			// 敵がMeleeEnemy型の場合
			auto meleeEnemy = dynamic_cast<MeleeEnemy*>(enemy.get());
			{
				if (meleeEnemy != nullptr) // MeleeEnemyが有効な場合
				{
					std::string mEnemyLife = "meleeEnemyLife:" + std::to_string(meleeEnemy->GetLife());
					//DrawString(0, 420, mEnemyLife.c_str(), GetColor(255, 0, 0));

					const char* enemyAbility = "";
					switch(meleeEnemy->_abilityEnum)
					{
						case MeleeEnemy::AbilityEnum::NONE:
						{
							enemyAbility = "None";
							break;
						}
						case MeleeEnemy::AbilityEnum::MELEE:
						{
							enemyAbility = "Melee";
							break;
						}
						case MeleeEnemy::AbilityEnum::BULLET:
						{
							enemyAbility = "Bullet";
							break;
						}
					}
					std::string mEnemyAbility = "meleeEnemyAbility:" + std::string(enemyAbility);
					//DrawString(0, 440, mEnemyAbility.c_str(), GetColor(255, 0, 0));
				}
			}

			// 敵がAoEEnemy型の場合
			auto aoeEnemy = dynamic_cast<AoEEnemy*>(enemy.get());
			{
				if (aoeEnemy != nullptr) // aoeEnemyが有効な場合
				{
					std::string aEnemyLife = "aoeEnemyLife:" + std::to_string(aoeEnemy->GetLife());
					//DrawString(0, 500, aEnemyLife.c_str(), GetColor(255, 0, 0));

					const char* enemyAbility = "";
					switch(aoeEnemy->_abilityEnum)
					{
						case AoEEnemy::AbilityEnum::NONE:
						{
							enemyAbility = "None";
							break;
						}
						case AoEEnemy::AbilityEnum::MELEE:
						{
							enemyAbility = "Melee";
							break;
						}
						case AoEEnemy::AbilityEnum::BULLET:
						{
							enemyAbility = "Bullet";
							break;
						}
					}
					std::string aEnemyAbility = "aoeEnemyAbility:" + std::string(enemyAbility);
					//DrawString(0, 520, aEnemyAbility.c_str(), GetColor(255, 0, 0));
				}
			}

			// 敵がShieldEnemy型の場合
			auto shieldEnemy = dynamic_cast<ShieldEnemy*>(enemy.get());
			{
				if (shieldEnemy != nullptr) // shieldEnemyが有効な場合
				{
					std::string sEnemyLife = "shieldEnemyLife:" + std::to_string(shieldEnemy->GetLife());
					//DrawString(0, 600, sEnemyLife.c_str(), GetColor(255, 0, 0));

					const char* enemyAbility = "";
					switch(shieldEnemy->_abilityEnum)
					{
						case ShieldEnemy::AbilityEnum::NONE:
						{
							enemyAbility = "None";
							break;
						}
						case ShieldEnemy::AbilityEnum::MELEE:
						{
							enemyAbility = "Melee";
							break;
						}
						case ShieldEnemy::AbilityEnum::BULLET:
						{
							enemyAbility = "Bullet";
							break;
						}
					}
					std::string sEnemyAbility = "shieldEnemyAbility:" + std::string(enemyAbility);
					//DrawString(0, 620, sEnemyAbility.c_str(), GetColor(255, 0, 0));
				}
			}
		}

		// 弾丸
		for (auto& bullet : _bullets)
		{			
			// アクティブな弾丸数を表示（デバッグ用）
			{
				int activeBullets = 0;

				// アクティブな弾丸数をカウント
				if (bullet->IsActive()) activeBullets++; // 弾丸が有効な場合、カウントを増やす

				// アクティブな弾丸数を表示
				std::string activeBulletNum = "activeBullets:" + std::to_string(activeBullets);
				//DrawString(0, 340, activeBulletNum.c_str(), GetColor(255, 0, 0));
			}
		}

		// その他の表示
		{
			DrawCntOther();
		}
	}

	return true;
}

/********************************メニュー項目**********************************/ 
// エフェクシア起動
class MenuItemLaunchEffekseer : public MenuItemBase
{
public:
	MenuItemLaunchEffekseer(void* param, std::string text) : MenuItemBase(param, text) {}

	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		// ModeGameより上のレイヤーに登録する
		ModeServer::GetInstance()->Add(new ModeEffekseer(), 100, "effectsample");
		return 0;
	}
};

// シャドウマップ表示切替
class MenuItemViewShadowMap : public MenuItemBase
{
public:
	MenuItemViewShadowMap(void* param, std::string text) : MenuItemBase(param, text) {}

	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		ModeGame* mdGame = static_cast<ModeGame*>(_param);
		mdGame->SetDebugViewShadowMap(!mdGame->GetDebugViewShadowMap());
		return 0;
	}
};

// クールタイム減少処理
void ModeGame::DecCooltime()
{
	if(_attack_cooltime > 0.0f)
	{
		_attack_cooltime -= 1.0f;
	}
}

// 敵生成関数
void ModeGame::SpawnEnemies(int meleeCount, int bulletCount, int aoeCount, int shieldCount)
{
	// ランダム位置生成のラム式
	auto generateRandPos = []()->VECTOR
	{
		return VGet
		(
			(rand() % static_cast<int>(SPAWN_RANGE_X * 2)) - SPAWN_RANGE_X,	// X座標
			SPAWN_HEIGHT,													// Y座標
			(rand() % static_cast<int>(SPAWN_RANGE_Z * 2)) - SPAWN_RANGE_Z	// Z座標
		);
	};

	// 近接攻撃エネミー生成
	for(int i = 0; i < meleeCount; i++)
	{
		auto meleeEnemy = std::make_shared<MeleeEnemy>();	// 近接攻撃エネミー生成
		meleeEnemy->Initialize();							// 初期化
		meleeEnemy->SetPosition(generateRandPos());			// ランダム位置に設定
		_enemies.push_back(meleeEnemy);						// エネミー配列に追加
		_spawncnt++;										// スポーンカウント増加
	}	

	// 弾攻撃エネミー生成
	for(int i = 0; i < bulletCount; i++)
	{
		auto bulletEnemy = std::make_shared<BulletEnemy>();	// 弾攻撃エネミー生成
		bulletEnemy->Initialize();							// 初期化
		bulletEnemy->SetPosition(generateRandPos());		// ランダム位置に設定
		_enemies.push_back(bulletEnemy);					// エネミー配列に追加
		_spawncnt++;										// スポーンカウント増加
	}

	// 範囲攻撃エネミー生成
	for(int i = 0; i < aoeCount; i++)
	{
		auto aoeEnemy = std::make_shared<AoEEnemy>();	// 範囲攻撃エネミー生成
		aoeEnemy->Initialize();							// 初期化
		aoeEnemy->SetPosition(generateRandPos());		// ランダム位置に設定
		_enemies.push_back(aoeEnemy);					// エネミー配列に追加
		_spawncnt++;									// スポーンカウント増加
	}

	// シールドエネミー生成
	for(int i = 0; i < shieldCount; i++)
	{
		auto shieldEnemy = std::make_shared<ShieldEnemy>();	// シールドエネミー生成
		shieldEnemy->Initialize();							// 初期化
		shieldEnemy->SetPosition(generateRandPos());		// ランダム位置に設定
		_enemies.push_back(shieldEnemy);					// エネミー配列に追加
		_spawncnt++;										// スポーンカウント増加
	}
}

// 敵のライフが0になったらカウントを減らす処理
void ModeGame::CheckDeathEnemy()
{
	// エネミー配列を走査
	for(auto it = _enemies.begin(); it != _enemies.end(); )
	{
		auto& enemy = *it; // エネミーへの参照を取得

		// 敵のステータスが死亡なら
		if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH)
		{
			// スポーンカウントを減らす
			if(_spawncnt > 0)
			{
				_spawncnt--; 
			}

			// 死亡した敵を配列から削除
			enemy->Terminate();			// 敵のターミネート処理
			it = _enemies.erase(it);	// イテレータを更新して削除
		}
		else
		{
			++it; // 次のエネミーへ進む
		}
	}
}

// メニュー画面を開く処理
void ModeGame::OpenMenu()
{
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// ESCキーでメニューを開く
	if(trg & PAD_INPUT_9)
	{
		ModeMenu* modeMenu = new ModeMenu();

		// ModeGameより上のレイヤーにメニューを登録する
		ModeServer::GetInstance()->Add(modeMenu, 99, "menu");

		// ModeMenuにメニュー項目を追加する
		modeMenu->Add(new MenuItemLaunchEffekseer(this, "Effekseer"));
		modeMenu->Add(new MenuItemViewShadowMap(this, "ShadowMapView"));
	}
}

// クリア処理
void ModeGame::ClearProcess()
{
	// マップ内の敵が全滅しているか確認
	if(AllEnemiesDeadInArea() != false) // 全滅している場合
	{

	}
}

// その他の描画
void ModeGame::DrawCntOther()
{
	// 描画オフセット
	int offsetX = 1750;
	int offsetY = 10;

	// スポーンカウント表示
	{
		std::string spawnCntStr = "残りの敵の数 : " + std::to_string(_spawncnt);
		DrawString(offsetX, offsetY, spawnCntStr.c_str(), GetColor(255, 0, 0));
		offsetY += 20;
	}

	// クールタイム表示
	{
		std::string cooltimeStr = "攻撃クールタイム : " + std::to_string(static_cast<int>(_attack_cooltime));
		DrawString(offsetX-100, offsetY, cooltimeStr.c_str(), GetColor(255, 0, 0));
		offsetY += 20;
	}
}

// シャドウマップ描画
void ModeGame::DrawShadowMap()
{
	VECTOR lightDir = _map->GetLightDir(); // ライトの方向を取得

	// シャドウマップが想定するライトの方向もセット
	SetShadowMapLightDirection(_handleShadowMap, lightDir);

	// シャドウマップに描画する範囲を設定
	SetShadowMapDrawArea(_handleShadowMap, VGet(-500.0f, -1.0f, -500.0f), VGet(500.0f, 500.0f, 500.0f));

	// 2回まわして、path=0:シャドウマップへの描画, path=1:モデルの描画
	for(int path = 0; path < 2; path++)
	{
		if(path == 0)
		{
			// シャドウマップへの描画の準備
			ShadowMap_DrawSetup(_handleShadowMap);
		}
		else if(path == 1)
		{
			// シャドウマップへの描画を終了
			ShadowMap_DrawEnd();

			// 描画に使用するシャドウマップを設定
			SetUseShadowMap(0, _handleShadowMap);
		}
	}

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);

	// シャドウマップ表示
	if(_bViewShadowMap)
	{
		TestDrawShadowMap(_handleShadowMap, 0, 0, 512, 512);
	}
}

// マップ内の敵が全滅しているか確認
bool ModeGame::AllEnemiesDeadInArea()
{
	// エネミー配列を走査
	for(auto& enemy : _enemies)
	{
		// 敵のステータスが生存中ならfalseを返す
		if(enemy->GetEnemyStatus() != Enemy::ENEMY_STATUS::DEATH)
		{
			return false; // 敵がまだ生存中
		}
	}
	return true; // すべての敵が死亡している
}

// 使用可能なダンガン取得
Bullet* ModeGame::GetAvailableBullet()
{
	// アクティブでない弾丸を探す
	for(auto& bullet : _bullets)
	{
		// アクティブでない弾丸を返す
		if(!bullet->IsActive()) // 弾丸が無効な場合
		{
			return bullet.get(); // 使用可能な弾丸を返す
		}
	}
	return nullptr; // 使用可能な弾丸がない場合
}

//// プレイヤーと敵の当たり判定
//void ModeGame::HitPlayerEnemy(Player* player, Enemy* enemy)
//{
//	if(player == nullptr){ return; }
//	if(enemy == nullptr){ return; }
//
//	// 無敵時間チェック
//	if(player->GetCooltime() > 0){ return; } // 無敵時間中
//
//	// 敵のステータスが死亡ならスキップ
//	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }
//
//	// プレイヤーと敵の距離を計算
//	VECTOR vDiff = VSub(player->GetPosition(), enemy->GetPosition()); // 位置の差分ベクトル
//	float distance = VDot(vDiff, vDiff);							  // 距離の二乗
//	float colDist = player->GetColR() + enemy->GetColR();			  // 当たり判定距離
//	float colDistSq = colDist * colDist;							  // 当たり判定距離の二乗
//
//	// 当たり判定
//	if(distance <= colDistSq)
//	{
//		// 衝突処理
//		player->DamagePlayer(); // ダメージ処理
//	}
//}
//
//// プレイヤーのカプセルと弾丸の当たり判定
//void ModeGame::HitPlayerCapsuleBullet(Player* player, Bullet* bullet)
//{
//	// 無効な弾丸は無視
//	if(!bullet->IsActive()) { return; }
//
//	// プレイヤーの体力が0ならスキップ
//	if(player->GetLife() <= 0) { return; }
//
//	// プレイヤーのカプセルコリジョン情報を取得
//	VECTOR player_pos = player->GetPosition();
//	VECTOR capsuleTop = VAdd(player_pos, player->GetCapsuleTop());
//	VECTOR capsuleBottom = VAdd(player_pos, player->GetCapsuleBottom());
//	float capsuleRadius = player->GetCapsuleRadius();
//
//	// 弾丸の位置を取得
//	VECTOR bulletPos = bullet->GetPosition();
//	float bulletRadius = bullet->GetColR();
//
//	// カプセルと球の当たり判定
//	if(IsHitCapsuleSphere(capsuleTop, capsuleBottom, capsuleRadius, bulletPos, bulletRadius))
//	{
//		// 衝突処理
//
//		bullet->Deactivate();    // 弾丸を無効化
//
//		if(player->GetCooltime() > 0) { return; } // 無敵時間中は無視
//
//		player->DamagePlayer(); // ダメージ処理
//	}
//
//	// 1.	カプセルと球の当たり判定関数: IsHitCapsuleSphere()でカプセルの中心軸（線分）と球の中心の最短距離を計算
//	// 2.	点と線分の距離計算 : GetPointSegmentSq()で3D空間での点と線分の最短距離を計算
//	// 3.	弾丸との当たり判定 : HitPlayerCapsuleBullet()でプレイヤーのカプセルと敵の弾丸の当たり判定を実行
//}
//
//// シールドのカプセルと弾丸の当たり判定
//void ModeGame::HitShieldCapsuleBullet(Ability* abilityShield, Bullet* bullet)
//{
//	if(abilityShield == nullptr){ return; }
//
//	// シールドが無効ならスキップ
//	if(!abilityShield->GetIsShielding()){ return; }
//
//	// 無効な弾丸は無視
//	if(!bullet->IsActive()) { return; }
//
//	// 球の種類がエネミー弾丸でないならスキップ
//	if(bullet->GetBulletType() != BULLET_TYPE::ENEMY) { return; }
//
//	// シールドのカプセルコリジョン情報を取得
//	VECTOR capsuleTop = abilityShield->GetShieldColTop();
//	VECTOR capsuleBottom = abilityShield->GetShieldColBottom();
//	float capsuleRadius = abilityShield->GetShieldColR();
//
//	// 弾丸の位置を取得
//	VECTOR bulletPos = bullet->GetPosition();
//	float bulletRadius = bullet->GetColR();
//
//	// カプセルと球の当たり判定
//	if(IsHitCapsuleSphere(capsuleTop, capsuleBottom, capsuleRadius, bulletPos, bulletRadius))
//	{
//		// 衝突処理
//		abilityShield->SetIsShieldSuccess(true); // シールド成功フラグを立てる
//		bullet->Deactivate();					 // 弾丸を無効化
//	}
//}
//
//// エネミーのシールドと弾丸の当たり判定
//void ModeGame::HitEnemyShieldBullet(Enemy* enemy, Bullet* bullet)
//{
//	if(enemy == nullptr){ return; }
//	if(bullet == nullptr){ return; }
//
//	// 無効な弾丸は無視
//	if(!bullet->IsActive()) { return; }
//
//	// 敵のステータスが死亡ならスキップ
//	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }
//
//	// 球の種類がプレイヤー弾丸でないならスキップ
//	if(bullet->GetBulletType() != BULLET_TYPE::PLAYER) { return; }
//
//	// 弾丸の位置を取得
//	VECTOR bulletPos = bullet->GetPosition();
//	float bulletRadius = bullet->GetColR();
//
//	// ShieldEnemyの場合の処理
//	if(auto shieeldEnemy = dynamic_cast<ShieldEnemy*>(enemy))
//	{
//		// シールドのカプセルコリジョン情報を取得
//		VECTOR shieldCapsuleTop = enemy->GetShieldColTop();
//		VECTOR shieldCapsuleBottom = enemy->GetShieldColBottom();
//		float shieldCapsuleRadius = enemy->GetShieldColR();
//
//		// カプセルと球の当たり判定
//		if(IsHitCapsuleSphere(shieldCapsuleTop, shieldCapsuleBottom, shieldCapsuleRadius, bulletPos, bulletRadius))
//		{
//			// 衝突処理
//			enemy->SetIsShieldSuccess(true);	// シールド成功フラグを立てる
//			bullet->Deactivate();				// 弾丸を無効化
//		}
//	}
//}
//
//// エネミーのカプセルと弾の当たり判定
//void ModeGame::HitEnemyCapsuleBullet(Enemy* enemy, Bullet* bullet)
//{
//	// 無効な弾丸は無視
//	if(!bullet->IsActive()) { return; }
//
//	// 敵のステータスが死亡ならスキップ
//	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }
//
//	// エネミーのカプセルコリジョン情報を取得
//	VECTOR enemy_pos = enemy->GetPosition();
//	VECTOR capsuleTop = VAdd(enemy_pos, enemy->GetCapsuleTop());
//	VECTOR capsuleBottom = VAdd(enemy_pos, enemy->GetCapsuleBottom());
//	float capsuleRadius = enemy->GetCapsuleRadius();
//
//	// 弾丸の位置を取得
//	VECTOR bulletPos = bullet->GetPosition();
//	float bulletRadius = bullet->GetColR();
//
//	// カプセルと球の当たり判定
//	if(IsHitCapsuleSphere(capsuleTop, capsuleBottom, capsuleRadius, bulletPos, bulletRadius))
//	{
//		// 衝突処理
//		enemy->DamageEnemy(_bullet_damage); // ダメージ処理
//		bullet->Deactivate();				// 弾丸を無効化
//	}
//
//	// 1.	カプセルと球の当たり判定関数: IsHitCapsuleSphere()でカプセルの中心軸（線分）と球の中心の最短距離を計算
//	// 2.	点と線分の距離計算 : GetPointSegmentSq()で3D空間での点と線分の最短距離を計算
//	// 3.	弾丸との当たり判定 : HitPlayerCapsuleBullet()でプレイヤーのカプセルと敵の弾丸の当たり判定を実行
//}
//
//// シールドとエネミー攻撃の当たり判定
//void ModeGame::HitShieldColEnemyAttackCol(Ability* abilityShield, Enemy* enemy)
//{
//	if(abilityShield == nullptr){ return; }
//	if(enemy == nullptr){ return; }
//
//	// シールドが無効ならスキップ
//	if(!abilityShield->GetIsShielding()){ return; }
//
//	// エネミーが攻撃中でないならスキップ
//	if(!enemy->GetIsAttacking()){ return; }
//
//	// 敵のステータスが死亡ならスキップ
//	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }
//
//	// 無敵時間中ならスキップ
//	if(_player->GetCooltime() > 0){ return; }
//	if(_player->GetInvincibleTime() > 0) { return; }
//
//	// MeleeEnemyの場合の処理
//	if(auto meleeEnemy = dynamic_cast<MeleeEnemy*>(enemy))
//	{
//		// 攻撃範囲のカプセルとシールドのカプセルの当たり判定
//		// 近接攻撃エネミーの攻撃コリジョン情報を取得
//		VECTOR meleeAttackTop = meleeEnemy->GetAttackColTop();
//		VECTOR meleeAttackBottom = meleeEnemy->GetAttackColBottom();
//		float meleeAttackRadius = meleeEnemy->GetAttackColR();
//
//		// シールドのカプセルコリジョン情報を取得
//		VECTOR shieldCapsuleTop = abilityShield->GetShieldColTop();
//		VECTOR shieldCapsuleBottom = abilityShield->GetShieldColBottom();
//		float shieldCapsuleRadius = abilityShield->GetShieldColR();
//
//		// カプセルとカプセルの当たり判定
//		if(HitCheck_Capsule_Capsule(
//			shieldCapsuleTop, shieldCapsuleBottom, shieldCapsuleRadius,
//			meleeAttackTop, meleeAttackBottom, meleeAttackRadius
//		) != false)
//		{
//			_player->SetInvincibleTime(230.0f);	// シールド成功フラグを立てる
//		}
//	}
//	else
//	{
//	//	// プレイヤーと敵の位置関係をチェック
//	//	VECTOR playerPos = _player->GetPosition();
//	//	VECTOR enemyPos = enemy->GetPosition();
//	//	VECTOR toPlayer = VSub(playerPos, enemyPos);
//	//	toPlayer.y = 0; // 高さ成分を無視
//
//	//	VECTOR enemyDir = enemy->GetDirection();
//	//	enemyDir.y = 0; // 高さ成分を無視
//
//	//	// 敵がプレイヤーの方向を向いているかチェック
//	//	if(VSize(toPlayer) > 0.0f && VSize(enemyDir) > 0.0f)	// ゼロ除算防止
//	//	{
//	//		VECTOR toPlayerNorm = VNorm(toPlayer);			// プレイヤー方向の正規化ベクトル
//	//		VECTOR enemyDirNorm = VNorm(enemyDir);			// 敵の向きの正規化ベクトル
//	//		float dot = VDot(toPlayerNorm, enemyDirNorm);	// 内積を計算
//
//	//		// 敵がプレイヤーから逆方向を向いている場合はスキップ
//	//		// dot < -0.5 は約120度以上反対方向を向いている場合
//	//		if(dot < -0.5f)
//	//		{
//	//			return; // シールド成功判定を行わない
//	//		}
//	//	}
//
//	//	// シールドのカプセルコリジョン情報を取得
//	//	VECTOR capsuleTop = VAdd(abilityShield->GetPosition(), abilityShield->GetShieldColTop());
//	//	VECTOR capsuleBottom = VAdd(abilityShield->GetPosition(), abilityShield->GetShieldColBottom());
//	//	float capsuleRadius = abilityShield->GetShieldColR();
//
//	//	// 敵の攻撃コリジョン情報を取得
//	//	VECTOR enemyAttackTop = enemy->GetAttackColTop();
//	//	VECTOR enemyAttackBottom = enemy->GetAttackColBottom();
//	//	float enemyAttackRadius = enemy->GetAttackColR();
//
//	//	// カプセルと球の当たり判定
//	//	if(HitCheck_Capsule_Capsule(
//	//		capsuleTop, capsuleBottom, capsuleRadius,
//	//		enemyAttackTop, enemyAttackBottom, enemyAttackRadius
//	//	) != false)
//	//	{
//	//		_player->SetIsShieldSuccess(true);	// シールド成功フラグを立てる
//	//	}
//	}
//}
//
//// メレー判定とエネミーの当たり判定
//void ModeGame::HitMeleeColEnemy(Ability* abilityMelee, Enemy* enemy)
//{
//	if(abilityMelee == nullptr){ return; }
//	if(enemy == nullptr){ return; }
//
//	// 敵のステータスが死亡ならスキップ
//	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }
//
//	// AbilityMeleeの場合の処理
//	if(auto melee = dynamic_cast<AbilityMelee*>(abilityMelee))
//	{
//		// メレー攻撃中じゃないならスキップ
//		if(!melee->GetIsAttacking()){ return; }
//
//
//		// 近接攻撃の攻撃コリジョン情報を取得
//		VECTOR meleeAttackTop = melee->GetAttackColTop();
//		VECTOR meleeAttackBottom = melee->GetAttackColBottom();
//		float meleeAttackRadius = melee->GetAttackColR();
//
//		// エネミーのカプセルコリジョン情報を取得
//		VECTOR enemy_pos = enemy->GetPosition();
//		VECTOR capsuleTop = VAdd(enemy_pos, enemy->GetCapsuleTop());
//		VECTOR capsuleBottom = VAdd(enemy_pos, enemy->GetCapsuleBottom());
//		float capsuleRadius = enemy->GetCapsuleRadius();
//
//		// カプセルとカプセルの当たり判定
//		if(HitCheck_Capsule_Capsule(
//			capsuleTop, capsuleBottom, capsuleRadius,
//			meleeAttackTop, meleeAttackBottom, meleeAttackRadius
//		) != false)
//		{
//			_player->SetIsMeleeSuccess(true);	// メレー成功
//		}
//	}
//}