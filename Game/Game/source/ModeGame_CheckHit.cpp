#include "ModeGame.h"

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

// プレイヤーと敵の当たり判定
void ModeGame::HitPlayerEnemy(Player* player, Enemy* enemy)
{
	if(player == nullptr){ return; }
	if(enemy == nullptr){ return; }

	// 無敵時間チェック
	if(player->GetCooltime() > 0){ return; } // 無敵時間中

	// 敵のステータスが死亡ならスキップ
	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }

	// プレイヤーと敵の距離を計算
	VECTOR vDiff = VSub(player->GetPosition(), enemy->GetPosition()); // 位置の差分ベクトル
	float distance = VDot(vDiff, vDiff);							  // 距離の二乗
	float colDist = player->GetColR() + enemy->GetColR();			  // 当たり判定距離
	float colDistSq = colDist * colDist;							  // 当たり判定距離の二乗

	// 当たり判定
	if(distance <= colDistSq)
	{
		// 衝突処理
		player->DamagePlayer(); // ダメージ処理
	}
}

// プレイヤーのカプセルと弾丸の当たり判定
void ModeGame::HitPlayerCapsuleBullet(Player* player, Bullet* bullet)
{
	// 無効な弾丸は無視
	if(!bullet->IsActive()) { return; }

	// プレイヤーの体力が0ならスキップ
	if(player->GetLife() <= 0) { return; }

	// プレイヤーのカプセルコリジョン情報を取得
	VECTOR player_pos = player->GetPosition();
	VECTOR capsuleTop = VAdd(player_pos, player->GetCapsuleTop());
	VECTOR capsuleBottom = VAdd(player_pos, player->GetCapsuleBottom());
	float capsuleRadius = player->GetCapsuleRadius();

	// 弾丸の位置を取得
	VECTOR bulletPos = bullet->GetPosition();
	float bulletRadius = bullet->GetColR();

	// カプセルと球の当たり判定
	if(IsHitCapsuleSphere(capsuleTop, capsuleBottom, capsuleRadius, bulletPos, bulletRadius))
	{
		// 衝突処理

		bullet->Deactivate();    // 弾丸を無効化

		if(player->GetCooltime() > 0) { return; } // 無敵時間中は無視

		player->DamagePlayer(); // ダメージ処理
	}

	// 1.	カプセルと球の当たり判定関数: IsHitCapsuleSphere()でカプセルの中心軸（線分）と球の中心の最短距離を計算
	// 2.	点と線分の距離計算 : GetPointSegmentSq()で3D空間での点と線分の最短距離を計算
	// 3.	弾丸との当たり判定 : HitPlayerCapsuleBullet()でプレイヤーのカプセルと敵の弾丸の当たり判定を実行
}

// シールドのカプセルと弾丸の当たり判定
void ModeGame::HitShieldCapsuleBullet(Ability* abilityShield, Bullet* bullet)
{
	if(abilityShield == nullptr){ return; }

	// シールドが無効ならスキップ
	if(!abilityShield->GetIsShielding()){ return; }

	// 無効な弾丸は無視
	if(!bullet->IsActive()) { return; }

	// 球の種類がエネミー弾丸でないならスキップ
	if(bullet->GetBulletType() != BULLET_TYPE::ENEMY) { return; }

	// シールドのカプセルコリジョン情報を取得
	VECTOR capsuleTop = abilityShield->GetShieldColTop();
	VECTOR capsuleBottom = abilityShield->GetShieldColBottom();
	float capsuleRadius = abilityShield->GetShieldColR();

	// 弾丸の位置を取得
	VECTOR bulletPos = bullet->GetPosition();
	float bulletRadius = bullet->GetColR();

	// カプセルと球の当たり判定
	if(IsHitCapsuleSphere(capsuleTop, capsuleBottom, capsuleRadius, bulletPos, bulletRadius))
	{
		// 衝突処理
		abilityShield->SetIsShieldSuccess(true); // シールド成功フラグを立てる
		bullet->Deactivate();					 // 弾丸を無効化
	}
}

// エネミーのシールドと弾丸の当たり判定
void ModeGame::HitEnemyShieldBullet(Enemy* enemy, Bullet* bullet)
{
	if(enemy == nullptr){ return; }
	if(bullet == nullptr){ return; }

	// 無効な弾丸は無視
	if(!bullet->IsActive()) { return; }

	// 敵のステータスが死亡ならスキップ
	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }

	// 球の種類がプレイヤー弾丸でないならスキップ
	if(bullet->GetBulletType() != BULLET_TYPE::PLAYER) { return; }

	// 弾丸の位置を取得
	VECTOR bulletPos = bullet->GetPosition();
	float bulletRadius = bullet->GetColR();

	// ShieldEnemyの場合の処理
	if(auto shieeldEnemy = dynamic_cast<ShieldEnemy*>(enemy))
	{
		// シールドのカプセルコリジョン情報を取得
		VECTOR shieldCapsuleTop = enemy->GetShieldColTop();
		VECTOR shieldCapsuleBottom = enemy->GetShieldColBottom();
		float shieldCapsuleRadius = enemy->GetShieldColR();

		// カプセルと球の当たり判定
		if(IsHitCapsuleSphere(shieldCapsuleTop, shieldCapsuleBottom, shieldCapsuleRadius, bulletPos, bulletRadius))
		{
			// 衝突処理
			enemy->SetIsShieldSuccess(true);	// シールド成功フラグを立てる
			bullet->Deactivate();				// 弾丸を無効化
		}
	}
}

// エネミーのカプセルと弾の当たり判定
void ModeGame::HitEnemyCapsuleBullet(Enemy* enemy, Bullet* bullet)
{
	// 無効な弾丸は無視
	if(!bullet->IsActive()) { return; }

	// 敵のステータスが死亡ならスキップ
	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }

	// エネミーのカプセルコリジョン情報を取得
	VECTOR enemy_pos = enemy->GetPosition();
	VECTOR capsuleTop = VAdd(enemy_pos, enemy->GetCapsuleTop());
	VECTOR capsuleBottom = VAdd(enemy_pos, enemy->GetCapsuleBottom());
	float capsuleRadius = enemy->GetCapsuleRadius();

	// 弾丸の位置を取得
	VECTOR bulletPos = bullet->GetPosition();
	float bulletRadius = bullet->GetColR();

	// カプセルと球の当たり判定
	if(IsHitCapsuleSphere(capsuleTop, capsuleBottom, capsuleRadius, bulletPos, bulletRadius))
	{
		// 衝突処理
		enemy->DamageEnemy(_bullet_damage); // ダメージ処理
		bullet->Deactivate();				// 弾丸を無効化
	}

	// 1.	カプセルと球の当たり判定関数: IsHitCapsuleSphere()でカプセルの中心軸（線分）と球の中心の最短距離を計算
	// 2.	点と線分の距離計算 : GetPointSegmentSq()で3D空間での点と線分の最短距離を計算
	// 3.	弾丸との当たり判定 : HitPlayerCapsuleBullet()でプレイヤーのカプセルと敵の弾丸の当たり判定を実行
}

// シールドとエネミー攻撃の当たり判定
void ModeGame::HitShieldColEnemyAttackCol(Ability* abilityShield, Enemy* enemy)
{
	if(abilityShield == nullptr){ return; }
	if(enemy == nullptr){ return; }

	// シールドが無効ならスキップ
	if(!abilityShield->GetIsShielding()){ return; }

	// エネミーが攻撃中でないならスキップ
	if(!enemy->GetIsAttacking()){ return; }

	// 敵のステータスが死亡ならスキップ
	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }

	// 無敵時間中ならスキップ
	if(_player->GetCooltime() > 0){ return; }
	if(_player->GetInvincibleTime() > 0) { return; }

	// MeleeEnemyの場合の処理
	if(auto meleeEnemy = dynamic_cast<MeleeEnemy*>(enemy))
	{
		// 攻撃範囲のカプセルとシールドのカプセルの当たり判定
		// 近接攻撃エネミーの攻撃コリジョン情報を取得
		VECTOR meleeAttackTop = meleeEnemy->GetAttackColTop();
		VECTOR meleeAttackBottom = meleeEnemy->GetAttackColBottom();
		float meleeAttackRadius = meleeEnemy->GetAttackColR();

		// シールドのカプセルコリジョン情報を取得
		VECTOR shieldCapsuleTop = abilityShield->GetShieldColTop();
		VECTOR shieldCapsuleBottom = abilityShield->GetShieldColBottom();
		float shieldCapsuleRadius = abilityShield->GetShieldColR();

		// カプセルとカプセルの当たり判定
		if(HitCheck_Capsule_Capsule(
			shieldCapsuleTop, shieldCapsuleBottom, shieldCapsuleRadius,
			meleeAttackTop, meleeAttackBottom, meleeAttackRadius
		) != false)
		{
			_player->SetInvincibleTime(230.0f);	// シールド成功フラグを立てる
		}
	}
	else
	{
		//	// プレイヤーと敵の位置関係をチェック
		//	VECTOR playerPos = _player->GetPosition();
		//	VECTOR enemyPos = enemy->GetPosition();
		//	VECTOR toPlayer = VSub(playerPos, enemyPos);
		//	toPlayer.y = 0; // 高さ成分を無視

		//	VECTOR enemyDir = enemy->GetDirection();
		//	enemyDir.y = 0; // 高さ成分を無視

		//	// 敵がプレイヤーの方向を向いているかチェック
		//	if(VSize(toPlayer) > 0.0f && VSize(enemyDir) > 0.0f)	// ゼロ除算防止
		//	{
		//		VECTOR toPlayerNorm = VNorm(toPlayer);			// プレイヤー方向の正規化ベクトル
		//		VECTOR enemyDirNorm = VNorm(enemyDir);			// 敵の向きの正規化ベクトル
		//		float dot = VDot(toPlayerNorm, enemyDirNorm);	// 内積を計算

		//		// 敵がプレイヤーから逆方向を向いている場合はスキップ
		//		// dot < -0.5 は約120度以上反対方向を向いている場合
		//		if(dot < -0.5f)
		//		{
		//			return; // シールド成功判定を行わない
		//		}
		//	}

		//	// シールドのカプセルコリジョン情報を取得
		//	VECTOR capsuleTop = VAdd(abilityShield->GetPosition(), abilityShield->GetShieldColTop());
		//	VECTOR capsuleBottom = VAdd(abilityShield->GetPosition(), abilityShield->GetShieldColBottom());
		//	float capsuleRadius = abilityShield->GetShieldColR();

		//	// 敵の攻撃コリジョン情報を取得
		//	VECTOR enemyAttackTop = enemy->GetAttackColTop();
		//	VECTOR enemyAttackBottom = enemy->GetAttackColBottom();
		//	float enemyAttackRadius = enemy->GetAttackColR();

		//	// カプセルと球の当たり判定
		//	if(HitCheck_Capsule_Capsule(
		//		capsuleTop, capsuleBottom, capsuleRadius,
		//		enemyAttackTop, enemyAttackBottom, enemyAttackRadius
		//	) != false)
		//	{
		//		_player->SetIsShieldSuccess(true);	// シールド成功フラグを立てる
		//	}
	}
}

// メレー判定とエネミーの当たり判定
void ModeGame::HitMeleeColEnemy(Ability* abilityMelee, Enemy* enemy)
{
	if(abilityMelee == nullptr){ return; }
	if(enemy == nullptr){ return; }

	// クールタイム中ならスキップ
	if(_attack_cooltime > 0.0f){ return; }

	// 敵のステータスが死亡ならスキップ
	if(enemy->GetEnemyStatus() == Enemy::ENEMY_STATUS::DEATH){ return; }

	// AbilityMeleeの場合の処理
	if(auto melee = dynamic_cast<AbilityMelee*>(abilityMelee))
	{
		// メレー攻撃中じゃないならスキップ
		if(!melee->GetIsAttacking()){ return; }


		// 近接攻撃の攻撃コリジョン情報を取得
		VECTOR meleeAttackTop = melee->GetAttackColTop();
		VECTOR meleeAttackBottom = melee->GetAttackColBottom();
		float meleeAttackRadius = melee->GetAttackColR();

		// エネミーのカプセルコリジョン情報を取得
		VECTOR enemy_pos = enemy->GetPosition();
		VECTOR capsuleTop = VAdd(enemy_pos, enemy->GetCapsuleTop());
		VECTOR capsuleBottom = VAdd(enemy_pos, enemy->GetCapsuleBottom());
		float capsuleRadius = enemy->GetCapsuleRadius();

		// カプセルとカプセルの当たり判定
		if(HitCheck_Capsule_Capsule(
			capsuleTop, capsuleBottom, capsuleRadius,
			meleeAttackTop, meleeAttackBottom, meleeAttackRadius
		) != false)
		{
			_attack_cooltime = 20.0f;	// クールタイム減少
			enemy->DamageEnemy(1);		// ダメージ処理
		}
	}
}