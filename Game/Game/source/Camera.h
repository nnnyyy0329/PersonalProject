#pragma once
#include "GameObjectBase.h"

class ApplicationMain;
class Player;

class Camera : public GameObjectBase
{
public:
	virtual bool Initialize() override; // 初期化
	virtual bool Terminate() override;  // 終了
	virtual bool Process() override;    // 更新
	virtual bool Render() override;		// 描画

	VECTOR GetTarget() const { return _vTarget; } // 注視点取得
	void SetTarget(const VECTOR& v) { _vTarget = v; } // 注視点設定

	void SetPlayer(Player* player){ _player = player; } // プレイヤーセット

protected:
	VECTOR _vTarget;			// 注視点
	float  _clipNear, _clipFar; // クリップ距離

	Player* _player;
};
