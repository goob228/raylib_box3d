#include "Menu.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <styles/style_brick.h>

#include "SysCvar.h"
#include "SysCmd.h"
#include "Parser.h"

static Font jbmono = {0};

static int fontSize = 32;

#define LOG_BUFFER_SIZE 32768
#define LINE_SIZE 128

static char log_buffer[LOG_BUFFER_SIZE] = {0};

static size_t log_buffer_ptr = 0;

static void (*resolve)() = mn_resolve;

int count_char(char* str, char c, int max)
{
    int counter = 0;
    for (int i = 0; i < max, str[i]; i++) {
        if (str[i] == c) 
            counter++;
    }

    return counter;
}

void MyTraceLog(int msgType, const char *text, va_list args)
{
    static char str[LINE_SIZE] = {0};
    vprintf(text, args); printf("\n");

    vsnprintf(str, LINE_SIZE, text, args);
    size_t len = strnlen(str, LINE_SIZE);
    if (log_buffer_ptr + LINE_SIZE+2 >= LOG_BUFFER_SIZE) {
        memcpy(log_buffer, &log_buffer[LOG_BUFFER_SIZE/2], LOG_BUFFER_SIZE/2);
        log_buffer_ptr = strnlen(log_buffer, LOG_BUFFER_SIZE-1);
        log_buffer[log_buffer_ptr] = '\n';
        log_buffer[++log_buffer_ptr] = '\0';
    }
    strcpy(&log_buffer[log_buffer_ptr], str );
    log_buffer_ptr+=len;
    log_buffer[log_buffer_ptr] = '\n';
    log_buffer[++log_buffer_ptr] = '\0';
    
}

void ListCvars(Command* cmd)
{
    Cvar* cur = getFirstCvar();

    do {
        printCvar(cur);
    } while (cur = cur->next);
}

void ListCmds(Command* cmd)
{
    Command* cur = getFirstCommand();

    do {
        TraceLog(LOG_NONE, "%s: %s", cur->name, cur->decription);
    } while (cur = cur->next);
}

void mn_initMenu()
{
    SetTraceLogCallback(MyTraceLog);

    setCommandCallBack(&cmd_cvars, ListCvars);
    setCommandCallBack(&cmd_help, ListCmds);
    

    GuiLoadStyleBrick();
    GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
    jbmono = LoadFont("res/JetBrainsMono-Bold.ttf");
    GuiSetFont(jbmono);
    

    GuiSetStyle(DEFAULT, LISTVIEW, 0);

    
}

void mn_openConsole()
{
    resolve = mn_resolve_console;
}

void mn_restartMenu()
{
    resolve = mn_resolve;
}


void mn_drawMenu()
{
    if (resolve) resolve();
}

void mn_resolve_singlePlayer() 
{
    if (GuiButton((Rectangle){50, 100, 200, 90}, "New game")) {

    }    

    if (GuiButton((Rectangle){50, 200, 200, 90}, "Load game")) {
        
    }    
}
void mn_resolve_console()
{
    static int but = 0;
    static bool oleg = true;
    static char text[LINE_SIZE] = {0};
    static Vector2 scroll = {0};
    static Rectangle view = {0};
    static int value = 0;
    static int active = 0;
    //GuiTabBar((Rectangle){10, 10, 900, 200}, text, &but, &active);
    //GuiTextInputBox((Rectangle){10, 10, 900, 200}, "console", "type in", text, 20, "but1", &but, &oleg);
    //GuiTextBox((Rectangle){0, 0, wh_width.valuei, 200}, log_buffer, LOG_BUFFER_SIZE, false);
    //GuiScrollPanel((Rectangle){0, 0, wh_width.valuei, 200}, log_buffer,(Rectangle){0, 0, wh_width.valuei-10, 1000}, &scroll, &view);
    int statbar = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;
    Rectangle console = (Rectangle){0, -statbar, cv_width.valuei, 200+statbar };

    Vector2 sizeoftxt = MeasureTextEx(jbmono, log_buffer, fontSize, 1.0f);
    //int counter_char = count_char(log_buffer, '\n', MAX_LINE_BUFFER_SIZE);
    
    
    BeginScissorMode(console.x, console.y, console.width, console.height);
    GuiScrollPanel(console, "", (Rectangle){0, 0, sizeoftxt.x, sizeoftxt.y}, &scroll, &view);
    DrawTextEx(jbmono, log_buffer, (Vector2){console.x+10 + scroll.x, console.y + console.height - scroll.y - sizeoftxt.y + statbar}, fontSize, 1.0f, BLACK);
    EndScissorMode();
    
    int pedik = GuiTextBox((Rectangle){0, 200, cv_width.valuei, 50}, text, 128, true);
    

    size_t len = strnlen(text, LINE_SIZE);
    if (text[len-1] == '`')
        text[len-1] = 0;

    if (pedik) {
        scroll = (Vector2){0};
        view = (Rectangle){0};
        
        TraceLog(LOG_NONE, "%s", text);

        parseString(text, LINE_SIZE);

        text[0] = 0;

    }
}

void mn_resolve()
{

    if (GuiButton((Rectangle){50, 100, 200, 90}, "Singleplayer")) {
        resolve = mn_resolve_singlePlayer;
    } 

}