#include "appframe.h"
#include "ModeMenu.h"
#include "ModeGameOver.h"
//#include "ModeGameClear.h"

#define BULLET_MAX 100

// 前方宣言
class Player;
class Enemy;
class BulletEnemy;
class MeleeEnemy;
class AoEEnemy;
class ShieldEnemy;
class Ability;
class AbilityBaseAttack;
class AbilityBullet;
class AbilityMelee;
class AbilityAoE;
class AbilityShield;
class Bullet;
class Camera;
class Map;

class ModeGame : public ModeBase
{
	typedef ModeBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void DecCooltime();																	// 攻撃クールタイム減少処理
	void SpawnEnemies(int meleeCount, int bulletCount, int aoeCount, int shieldCount);	// エネミー生成関数
	void CheckDeathEnemy();																// 敵のライフが0になったらカウントを減らす処理
	void OpenMenu();																	// メニュー画面を開く処理
	void ClearProcess();																// クリア処理
	void GameOverProcess();																// ゲームオーバー処理
	void DrawCntOther();																// その他の描画	
	void DrawShadowMap();																// シャドウマップ描画処理
	bool AllEnemiesDeadInArea();														// マップ内の敵が全滅しているか確認

	// 弾丸管理用
	Bullet* GetAvailableBullet();

	// 当たり判定処理
	void HitPlayerEnemy				(Player* player, Enemy* enemy);				// プレイヤーとエネミーの当たり判定
	void HitPlayerCapsuleBullet		(Player* player, Bullet* bullet);			// プレイヤーのカプセルと弾の当たり判定
	void HitShieldCapsuleBullet		(Ability* abilityShield, Bullet* bullet);	// シールドのカプセルと弾の当たり判定
	void HitEnemyShieldBullet		(Enemy* enemy, Bullet* bullet);				// エネミーとシールドと弾の当たり判定
	void HitEnemyCapsuleBullet		(Enemy* enemy, Bullet* bullet);				// エネミーのカプセルと弾の当たり判定
	void HitShieldColEnemyAttackCol	(Ability* abilityShield, Enemy* enemy);		// シールドとエネミー攻撃の当たり判定
	void HitMeleeColEnemy			(Ability* abilityMelee, Enemy* enemy);		// メレー判定とエネミーの当たり判定

	bool GetDebugViewShadowMap() { return _bViewShadowMap; }
	void SetDebugViewShadowMap(bool b) { _bViewShadowMap = b; }

protected:

	// シャドウマップ用
	int _handleShadowMap;
	bool	_bViewShadowMap;

	int _gamecnt;
	int _spawncnt;
	float _bullet_damage;
	float _attack_cooltime;

	std::shared_ptr<Player>_player;
	std::shared_ptr<Camera>_cam;
	std::shared_ptr<Map>_map;

	std::vector<std::shared_ptr<Bullet>>_bullets;
	std::vector<std::shared_ptr<Enemy>>_enemies;
	std::vector<std::shared_ptr<Ability>>_abilities;
}; 
