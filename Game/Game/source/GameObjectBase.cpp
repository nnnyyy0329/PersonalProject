#include "GameObjectBase.h"

bool GameObjectBase::Initialize()
{
	_vPos = VGet(0, 0, 0);
	_vDir = VGet(0, 0, -1);

	_handle = -1;		// ハンドル
	_attach_index = -1; // アタッチインデックス
	_total_time = 0.0f; // 総経過時間
	_play_time = 0.0f;  // 再生時間

	return true;
}

bool GameObjectBase::Terminate()
{
	return true;
}

bool GameObjectBase::Process()
{
	return true;
}

bool GameObjectBase::Render()
{
	return true;
}
