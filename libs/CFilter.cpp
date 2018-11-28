/***
*      Author: ominenko
*/
#include "CFilter.h"
using namespace std ;
set<CFilterLoop *> CFilterLoop::filters;

void CFilterLoop::loops(){
    for(auto it:filters){
        it->loop();
    }
}

uint32_t CTimeInMs_stub::_time=0;

//eof
