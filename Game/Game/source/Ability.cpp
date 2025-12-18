#include "Ability.h"

bool Ability::Initialize()
{
	// Šî’êƒNƒ‰ƒX‚Ì‰Šú‰»‚ğŒÄ‚Ño‚·
	if(!Character::Initialize()) { return false; }

	return true;
}

bool Ability::Terminate()
{
	return true;
}

bool Ability::Process()
{
	return true;
}

bool Ability::Render()
{
	return true;
}