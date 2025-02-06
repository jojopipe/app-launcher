#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

static int WINDOW_WIDTH = 1000;
static int WINDOW_HEIGHT = 600;

void enter_pressed(char *file_name) {
    //ENTER PRESSED
    pid_t child_pid;
    child_pid = fork();
    if (child_pid < 0) {
        fprintf(stderr, "bug :(\n");
        exit(-1);
    }
    else if (child_pid == 0) {
        //child process
        int exec_result = execlp(file_name, file_name);
        exit(exec_result);
    }
    else {
        //parent process
        exit(0);
    }
}

int main(void) {
    //GuiLoadStyle("theme_light.rgs");
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "app launcher");
    SetTargetFPS(60);
    char textboxtext[4096] = "";
    char list_entries[4096] = "ONE;TWO;THREE";
    int scroll_value = 0;

    float padding = 10.0f;

    float main_rect_width = 500;
    float main_rect_height = 50;
    Rectangle main_text_rect= {WINDOW_WIDTH/2 - main_rect_width/2,
                                 WINDOW_HEIGHT/2 - main_rect_height/2,
                                 main_rect_width, main_rect_height};

    float multi_rect_width = main_rect_width;
    float multi_rect_height = 200;
    Rectangle multi_rect = {WINDOW_WIDTH/2 - multi_rect_width/2,
                            WINDOW_HEIGHT/2 + main_rect_height/2 + padding,
                            multi_rect_width, multi_rect_height};

    char info[64];

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        //int GuiTextBox(Rectangle bounds, char *text, int bufferSize, bool editMode)
        if (GuiTextBox(main_text_rect, textboxtext, 4096, 1)) {
            enter_pressed(textboxtext);
        }
        if (*textboxtext != 0) {
            //int GuiListView(Rectangle bounds, const char *text, int *scrollIndex, int *active)
            (void) GuiListView(multi_rect, list_entries, &scroll_value, 0);
        }
        snprintf(info, 64, "mult pos x: %f", WINDOW_WIDTH/2 - multi_rect_width / 2);
        DrawText(info, 10, 10, 8, BLACK);
        snprintf(info, 64, "mult pos y: %f", WINDOW_HEIGHT/2 - main_rect_height - padding);
        DrawText(info, 10, 20, 8, BLACK);
        snprintf(info, 64, "main pos x: %f", WINDOW_WIDTH/2 - main_rect_width/2);
        DrawText(info, 10, 30, 8, BLACK);
        snprintf(info, 64, "main pos y: %f", WINDOW_HEIGHT/2 - main_rect_height/2);
        DrawText(info, 10, 40, 8, BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}