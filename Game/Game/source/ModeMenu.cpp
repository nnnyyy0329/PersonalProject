
#include "AppFrame.h"
#include "ApplicationMain.h"
#include "ApplicationGlobal.h"
#include "ModeMenu.h"


bool ModeMenu::Initialize() {
	if (!base::Initialize()) { return false; }

	_curPos = 0;
	_curAnimCnt = 0;

	return true;
}

bool ModeMenu::Terminate() {
	base::Terminate();
	Clear();
	return true;
}

void ModeMenu::Clear() {
	// 登録したitemを全部削除する
	for (auto ite = _vItems.begin(); ite != _vItems.end();) {
		delete (*ite);
		ite = _vItems.erase(ite);
	}
}

void ModeMenu::Add(MenuItemBase* itemBase) {
	_vItems.push_back(itemBase);
}

bool ModeMenu::Process() {
	base::Process();
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// このモードより下のレイヤーはProcess()を呼ばない
	ModeServer::GetInstance()->SkipProcessUnderLayer();


	// ESCキーでメニューを閉じる
	bool close = false;
	if (trg & PAD_INPUT_9) {
		close = true;
	}

	// 上下でカーソルを移動する
	if (trg & PAD_INPUT_UP) { _curPos--; _curAnimCnt = 0; }
	if (trg & PAD_INPUT_DOWN) { _curPos++; _curAnimCnt = 0; }

	// カーソル位置を上下ループ
	int itemNum = _vItems.size();
	_curPos = (_curPos + itemNum) % itemNum;

	// 決定でアイテムのSelected()を呼ぶ
	if (trg & PAD_INPUT_1) {
		int ret = _vItems[_curPos]->Selected();
		if (ret == 1) {
			// メニューを閉じる
			close = true;
		}
	}


	// メニューを閉じる
	if (close) {
		// このモードを削除する
		ModeServer::GetInstance()->Del(this);
	}

	// カーソルアニメカウンタ
	_curAnimCnt++;


	return true;
}

bool ModeMenu::Render() {
	base::Render();

	// メニュー項目の確認
	int x = 128, y = 128, w = 0, h = 0, fontSize = 32, fontPitch = fontSize + 8;
	SetFontSize(fontSize);
	for (auto ite = _vItems.begin(); ite != _vItems.end();ite++) {
		int item_w = GetDrawStringWidth((*ite)->_text.c_str(), strlen((*ite)->_text.c_str()));
		if (w < item_w) {
			w = item_w;
		}
		h += fontPitch;
	}
	// カーソル, 枠分のサイズ拡張
	w += 64 + 16; h += 16;

	// 下地の描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(x, y, x + w, y + h, GetColor(0, 0, 255), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(x, y, x + w, y + h, GetColor(0, 0, 255), TRUE);

	// メニュー項目の描画
	int startY = 16 / 2;
	h = 0;
	for (auto ite = _vItems.begin(); ite != _vItems.end(); ite++) {
		DrawString(x + 64, y + startY + h, (*ite)->_text.c_str(), GetColor(255,0,0));
		h += fontPitch;
	}
	// カーソルの描画
	// x座標はアニメーションする
	DrawGraph(x + 4 + ( (_curAnimCnt/6) % 4 ) * 4, y + startY + fontPitch * _curPos, gGlobal._cgCursor, TRUE);

	return true;
}

