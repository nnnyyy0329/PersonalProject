#include "Camera.h"
#include "ApplicationMain.h"
#include "Player.h"

bool Camera::Initialize()
{
	// カメラの設定（わかりやすい位置に）
	_vPos = VGet(0, 140.f, -280.f);
	_vTarget = VGet(0, 60, 0);	

	// カメラのクリップ距離設定
	_clipNear = 2.f;	
	_clipFar = 10000.f; 

	// クラスインスタンスの初期化
	_player = nullptr;

	return true;
}

bool Camera::Terminate()
{
	return true;
}

bool Camera::Process()
{
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// 他のクラスの
	VECTOR player_pos = _player->GetPosition();
	VECTOR player_oldPos = _player->GetOldPos();
	VECTOR player_v = _player->GetV();

	// カメラ操作
	if ((key & (PAD_INPUT_7 | PAD_INPUT_8)) == (PAD_INPUT_7 | PAD_INPUT_8)) // Q & W
	{
		// 距離、ターゲットの高さ変更
		float sx = _vPos.x - _vTarget.x;		// X距離
		float sz = _vPos.z - _vTarget.z;		// Z距離
		float rad = atan2(sz, sx); 				// 角度
		float length = sqrt(sz * sz + sx * sx); // 距離

		if (key & PAD_INPUT_LEFT) { length -= 1.f; }  // 左キー
		if (key & PAD_INPUT_RIGHT) { length += 1.f; } // 右キー

		// カメラ位置更新
		_vPos.x = _vTarget.x + cos(rad) * length; // X位置
		_vPos.z = _vTarget.z + sin(rad) * length; // Z位置

		// Y位置
		if (key & PAD_INPUT_DOWN) { _vTarget.y -= 1.f; } // 下キー
		if (key & PAD_INPUT_UP) { _vTarget.y += 1.f; }  // 上キー
	}
	else if (key & PAD_INPUT_7) // Q
	{	
		// 角度変更
		// Y軸回転
		float sx = _vPos.x - _vTarget.x;		// X距離
		float sz = _vPos.z - _vTarget.z;		// Z距離 
		float rad = atan2(sz, sx); 				// 角度
		float length = sqrt(sz * sz + sx * sx); // 距離

		if (key & PAD_INPUT_LEFT) { rad -= 0.05f; }  // 左キー
		if (key & PAD_INPUT_RIGHT) { rad += 0.05f; } // 右キー

		// カメラ位置更新
		_vPos.x = _vTarget.x + cos(rad) * length; // X位置
		_vPos.z = _vTarget.z + sin(rad) * length; // Z位置

		// Y位置
		if (key & PAD_INPUT_DOWN) { _vPos.y -= 5.f; } // 下キー
		if (key & PAD_INPUT_UP) { _vPos.y += 5.f; }   // 上キー
	}
	else if (key & PAD_INPUT_8) // W
	{	
		// カメラ位置（注目位置もXZスライド）
		float sx = _vPos.x - _vTarget.x; // X距離
		float sz = _vPos.z - _vTarget.z; // Z距離
		float camrad = atan2(sz, sx);    // カメラ角度

		// 移動方向を決める
		VECTOR v = { 0,0,0 };

		float mvSpeed = 2.f; // 移動速度
		if (key & PAD_INPUT_DOWN) { v.x = 1; }  // 下キー
		if (key & PAD_INPUT_UP) { v.x = -1; }   // 上キー
		if (key & PAD_INPUT_LEFT) { v.z = -1; } // 左キー
		if (key & PAD_INPUT_RIGHT) { v.z = 1; } // 右キー

		// vをrad分回転させる
		float length = 0.f; // 移動量

		// 移動量計算
		if (VSize(v) > 0.f) { length = mvSpeed; } // 移動量設定
		float rad = atan2(v.z, v.x);			  // 方向角度
		v.x = cos(rad + camrad) * length;		  // X移動量
		v.z = sin(rad + camrad) * length;		  // Z移動量

		// vの分移動
		_vPos = VAdd(_vPos, v);		  // カメラ位置移動
		_vTarget = VAdd(_vTarget, v); // 注視点移動
	}

	return true;
}

bool Camera::Render()
{
	// カメラの設定
	{
		// カメラの位置と注視点の設定
		SetCameraPositionAndTarget_UpVecY(_vPos, _vTarget);

		// カメラのクリップ距離の設定
		SetCameraNearFar(_clipNear, _clipFar);
	}

	// デバッグ用表示
	{
		// 0,0,0を中心に線を引く
		{
			float linelength = 1000.f;
			VECTOR v = { 0, 0, 0 };
			DrawLine3D(VAdd(v, VGet(-linelength, 0, 0)), VAdd(v, VGet(linelength, 0, 0)), GetColor(255, 0, 0));
			DrawLine3D(VAdd(v, VGet(0, -linelength, 0)), VAdd(v, VGet(0, linelength, 0)), GetColor(0, 255, 0));
			DrawLine3D(VAdd(v, VGet(0, 0, -linelength)), VAdd(v, VGet(0, 0, linelength)), GetColor(0, 0, 255));
		}

		// カメラターゲットを中心に短い線を引く
		{
			float linelength = 10.f;
			VECTOR v = _vTarget;
			DrawLine3D(VAdd(v, VGet(-linelength, 0, 0)), VAdd(v, VGet(linelength, 0, 0)), GetColor(255, 0, 0));
			DrawLine3D(VAdd(v, VGet(0, -linelength, 0)), VAdd(v, VGet(0, linelength, 0)), GetColor(0, 255, 0));
			DrawLine3D(VAdd(v, VGet(0, 0, -linelength)), VAdd(v, VGet(0, 0, linelength)), GetColor(0, 0, 255));
		}
	}

	//// カメラ情報表示
	//{
	//	int x = 0, y = 0, size = 16;
	//	SetFontSize(size);
	//	DrawFormatString(x, y, GetColor(255, 0, 0), "Camera:"); y += size;
	//	DrawFormatString(x, y, GetColor(255, 0, 0), "  target = (%5.2f, %5.2f, %5.2f)", _vTarget.x, _vTarget.y, _vTarget.z); y += size;
	//	DrawFormatString(x, y, GetColor(255, 0, 0), "  pos    = (%5.2f, %5.2f, %5.2f)", _vPos.x, _vPos.y, _vPos.z); y += size;
	//	float sx = _vPos.x - _vTarget.x;
	//	float sz = _vPos.z - _vTarget.z;
	//	float length = sqrt(sz * sz + sx * sx);
	//	float rad = atan2(sz, sx);
	//	float deg = RAD2DEG(rad);
	//	DrawFormatString(x, y, GetColor(255, 0, 0), "  len = %5.2f, rad = %5.2f, deg = %5.2f", length, rad, deg); y += size;
	//}

	return true;
}