#pragma once
#include "Character.h"
#include "Bullet.h"

class Ability : public Character
{
public:
	virtual bool Initialize() override; // ‰Šú‰»
	virtual bool Terminate() override;  // I—¹
	virtual bool Process() override;    // XV
	virtual bool Render() override;		// •`‰æ

protected:
	Bullet* _bullet;

};

