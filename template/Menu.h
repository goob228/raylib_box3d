#ifndef MENU_H
#define MENU_H

#include <stdarg.h>

void MyTraceLog(int msgType, const char *text, va_list args);

void mn_resolve();
void mn_resolve_singlePlayer();
void mn_resolve_console();
void mn_drawMenu();
void mn_initMenu();
void mn_restartMenu();
void mn_openConsole();


#endif //MENU_H