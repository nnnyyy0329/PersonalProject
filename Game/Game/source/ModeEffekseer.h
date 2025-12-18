
#include "appframe.h"

#include <string>
#include <vector>


// モード
class ModeEffekseer : public ModeBase
{
	typedef ModeBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

protected:
	int		_effectResourceHandle;		// エフェクトファイルをロードするハンドル
	int		_playingEffectHandle;		// ロードしたエフェクトファイルから、エフェクトを生成したもの

	// エフェクト位置
	float	_position_x = 0.0f;
	float	_position_y = 0.0f;

}; 
