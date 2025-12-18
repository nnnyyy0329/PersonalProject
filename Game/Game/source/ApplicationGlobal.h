#include "appframe.h"

class ApplicationGlobal
{
public:
	ApplicationGlobal();
	virtual ~ApplicationGlobal();

	bool Init();

public:
	int		_cgCursor;
}; 

// 他のソースでgGlobalを使えるように
extern ApplicationGlobal		gGlobal;
