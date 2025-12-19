#include "ModeGameOver.h"
#include "AppFrame.h"
#include "ApplicationMain.h"
#include "ModeGame.h"
#include "ModeTitle.h"

bool ModeGameOver::Initialize()
{
	if (!base::Initialize()) { return false; }
	return true;
}

bool ModeGameOver::Terminate()
{
	base::Terminate();
	return true;
}

bool ModeGameOver::Process()
{
	base::Process();

	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// このモード中はプロセススキップ
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	if (trg & PAD_INPUT_B)
	{
		// このモードを削除予約
		ModeServer::GetInstance()->Del(this);

		// タイトルモードを追加
		ModeServer::GetInstance()->Add(new ModeTitle(), 5, "title");

		//ModeServer::GetInstance()->Add(new ModeGame(), 1, "game");

	}
	return true;
}

bool ModeGameOver::Render()
{
	base::Render();
	// タイトルモード中はバッファーをしない
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);
	SetUseBackCulling(FALSE);

	SetFontSize(48);

	const char* text = "GAME OVER";

	// ゲームオーバー画面描画
	DrawString((1920 - GetDrawStringWidth(text, strlen(text))) / 2, 200, text, GetColor(255, 0, 0));

	// 画面下部に再開テキストを描画
	DrawString((1920 - GetDrawStringWidth("Press X to Title", strlen("Press X to Title"))) / 2, 800, "Press X to Title", GetColor(255, 255, 0));

	return true;
}