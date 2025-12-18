
#include "AppFrame.h"
#include "ApplicationMain.h"
#include "ModeTitle.h"
#include "ModeGame.h"

bool ModeTitle::Initialize()
{
	if (!base::Initialize()) { return false; }

	_cg = LoadGraph("res/title.png");

	return true;
}

bool ModeTitle::Terminate() 
{
	DeleteGraph(_cg);

	base::Terminate();
	return true;
}

bool ModeTitle::Process()
{
	base::Process();
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// このモード中はプロセスキップ
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	if (trg & PAD_INPUT_A)	
	{
		// このモードを削除予約
		ModeServer::GetInstance()->Del(this);
		//ModeServer::GetInstance()->Add(new ModeGame(), 1, "game");
	}

	return true;
}

bool ModeTitle::Render() 
{
	base::Render();



	 //タイトルモード中はバッファーをしない
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);
	SetUseBackCulling(FALSE);

	 //タイトル画面描画
	DrawTitleScreen();

	return true;
}

// タイトル画面描画
void ModeTitle::DrawTitleScreen()
{	
	SetFontSize(48);

	//SetFontSize();
	const char* titleText = "Game Start";
	const char* text = "Press Z to Start";

	// 画面中央にタイトルテキストを描画
	DrawString((1920 - GetDrawStringWidth(titleText, strlen(titleText))) / 2, 200, titleText, GetColor(0, 255, 255));

	// 画面下部に開始テキストを描画
	DrawString((1920 - GetDrawStringWidth(text, strlen(text))) / 2, 800, text, GetColor(255, 255, 0));
}

