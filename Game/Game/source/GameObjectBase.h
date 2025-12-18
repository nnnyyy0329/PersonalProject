#pragma once
#include <cmath>
#include <vector>
#include <string>
#include "appframe.h"
#include "mymath.h"

// 計算用マクロ
#define PI				(3.14159265358979323846) // 円周率
#define	DEG2RAD(x)		( ((x) / 180.0f ) * PI ) // 度→ラジアン変換
#define RAD2DEG(x)		( ((x) * 180.0f ) / PI ) // ラジアン→度変換

class GameObjectBase : public ModeBase
{
public:
	virtual bool Initialize(); // 初期化
	virtual bool Terminate();  // 終了
	virtual bool Process();    // 更新
	virtual bool Render();	   // 描画

	VECTOR GetPosition() const { return _vPos; } // 位置取得
	void SetPosition(const VECTOR& v) { _vPos = v; } // 位置設定

	VECTOR GetDirection() const { return _vDir; } // 方向取得
	void SetDirection(const VECTOR& v) { _vDir = v; } // 方向設定
	
protected:
	VECTOR _vPos; // 位置
	VECTOR _vDir; // 方向
	
	// 3Dモデル描画用
	int _handle;	   // ハンドル
	int _attach_index; // アタッチインデックス
	float _total_time; // 総経過時間
	float _play_time;  // 再生時間

};

