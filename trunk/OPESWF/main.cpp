#include <iostream>
#include "opeswf.h"

unsigned int main()
{
	SWF* pSwf = new SWF;
	
	pSwf->LoadSWF("lol4.swf");
	
	delete pSwf;
	
	return 0;
}
