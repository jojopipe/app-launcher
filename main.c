#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#define __USE_MISC
#include <dirent.h>
#include <pwd.h>

//#define DRAW_DEBUG_INFO


int char_match(char c, char *s);
void string_split(char **output, int *outputc, char *input, int inptuc, char split[]);
int is_binary_in_path(char const *file_name);

static int window_width;
static int window_height;

char *PATH;



/**
 * compares c against s.
 * \brief is c contained in s?
 * \param c
 * \param s
 * \return
 *  \retval 1 if c is contained in s
 *  \retval 0, else
 */
int char_match(char c, char *s) {
    char *end = &s[strlen(s)];
    for (char *p = s; p < end; ++p) { //pointer magic
        if (*p == c) {
            return 1;
        }
    }
    return 0;
}

/**
 * \warning this function directly manipulates the input string!
 *
 * \brief takes a string and splits it by the passed character. split-strings are collected in output.
 * \param output output array, filled with sub-strings of input
 * \param outputc length of output array
 * \param input string to be splitted
 * \param inptuc length of input
 * \param split character to split by
 * \returns (pseudo returns) output, outputc
 */
void string_split(char **output, int *outputc, char *input, int inptuc, char split[]) {
    if (!input) return;
    *outputc = 0;
    output[*outputc] = input;
    for (int i = 1; i < inptuc; ++i) {
        if (input[i] == '\0') break;
        if (char_match(input[i], split)) {
            input[i] = '\0';
            /* look for next non-split char / skip over every split char */
            for (int j = i + 1; j < inptuc; ++j) {
                if (char_match(input[j], split)) continue;
                if (input[j] == '\0') break;
                output[++(*outputc)] = &input[j];
                i += (j - i - 1);
                break;
            }
        }
    }
    output[++(*outputc)] = NULL;
    /* ↑↑↑
     * increment once more, since, to this point,
     * outputc only represented the last index of the array
     * also add NULL as last element so execvp works as expected
     */
}

int enter_pressed(char *file_name) {
    int in_path = is_binary_in_path(file_name);
    if (!in_path) {
        return 1;
    }
    printf("exec: %s\n", file_name);
    pid_t child_pid;
    child_pid = fork();
    if (child_pid < 0) {
        fprintf(stderr, "bug =w=\n");
        return -1;
    }
    else if (child_pid == 0) {
        //child process
        int exec_result = execlp(file_name, file_name, NULL);
        exit(exec_result);
    }
    else {
        //parent process
        exit(0);
    }
    return 0;
}

int is_binary_in_path(char const *file_name) {
    char path_copy[4096];
    int path_len = (int) strlen(PATH);
    memcpy(path_copy, PATH, (path_len < 4096) ? path_len : 4096);
    char *output[4096];
    int outputc;
    string_split(output, &outputc, path_copy, 4096, ":");
    char curr_dir[4096];
    for (int i = 0; i < outputc; ++i) {
        (void) strcpy(curr_dir, output[i]);
        DIR* dir = opendir(curr_dir);
        if (!dir) continue;
        struct dirent* entry = NULL;
        while((entry = readdir(dir))) {
            if (entry->d_type & (DT_REG | DT_LNK)) {
                if (strcmp(file_name, entry->d_name) == 0) return 1;
            }
        }
    }
    return 0;
}

void put_multitext_entries(char *list, char const *file, int *entryc) {
    memset(list, 0, 4096);
    int list_empty = 1;
    int LIST_MAX_COUNT = 100;
    *entryc = 0;

    char path_copy[4096];
    int path_len = (int) strlen(PATH);
    memcpy(path_copy, PATH, (path_len < 4096) ? path_len : 4096);
    char *output[4096];
    int outputc;
    string_split(output, &outputc, path_copy, 4096, ":");
    char curr_dir[4096];
    for (int i = 0; i < outputc; ++i) {
        (void) strcpy(curr_dir, output[i]);
        DIR* dir = opendir(curr_dir);
        if (!dir) continue;
        struct dirent* entry = NULL;
        while((entry = readdir(dir))) {
            if (entry->d_type & (DT_REG | DT_LNK)) {
                if (strncmp(file, entry->d_name, strlen(file)) == 0) {
                    //add entry to list
                    if (!list_empty) {
                        strcat(list, ";");
                    }
                    strcat(list, entry->d_name);
                    list_empty = 0;
                    if (++(*entryc) > LIST_MAX_COUNT) {
                        return;
                    }
                }
            }
        }
    }
}

void getListItem(char *item, char *list, int index) {
    int counter = 0;
    int found = 0;
    int end = 0;
    for (int i = 0; i < (int) strlen(list); ++i) {
        char curr = list[i];
        if (counter == index) {
            found = i;
            break;
        }
        if (curr == ';') {
            ++counter;
        }
    }
    for (int i = 0; i < (int) strlen(list); ++i) {
        char curr = list[found + i];
        if (curr == ';') {
            end = found + i;
            break;
        }
    }
    if (!found && !end) {
        strcpy(item, list);
        return;
    }
    list[end] = 0;
    memcpy(item, &list[found], end - found);
}

int main(void) {
    PATH = getenv("PATH");
    if (!PATH) {
        printf("wtf is going on with your system, oh mah gah\n");
        exit(1);
    }
    printf("path: %s\n", PATH);
    int monitor = GetCurrentMonitor();
    window_width = 1920/3;
    window_height = 1080/15;

    InitWindow(window_width, window_height, "app launcher");
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    char config_file[1024];
    strcpy(config_file, homedir);
    strcat(config_file, "/.config/app-launcher/theme.rgs");
    GuiLoadStyle(config_file);
    SetTargetFPS(GetMonitorRefreshRate(monitor));
    char textbox_text[4096] = "";
    char last_text[4096] = "";
    char list_entries[4096] = "";
    int entryc = 0;
    int scroll_value = 0;
    int multi_active = 0;
    int right_disabled = 0;

    float padding = 5.0f;
    float window_padding = 3.0f;

    float main_rect_width = window_width - 2 * window_padding;
    float main_rect_height = window_height - 2 * window_padding;
    Rectangle main_text_rect;
    main_text_rect = (Rectangle) {window_padding,
                                  window_padding,
                                  main_rect_width, main_rect_height};
    float multi_rect_width = main_rect_width;
    float multi_rect_height = main_rect_height * 5;
    Rectangle multi_rect = {window_padding,
                            window_padding + main_rect_height + padding,
                           multi_rect_width, multi_rect_height};

#ifdef DRAW_DEBUG_INFO
    char info[64];
#endif //DRAW_DEBUG_INFO

    while (!WindowShouldClose())
    {
        SetWindowSize(window_width, window_height);

        if (IsKeyPressed(KEY_ENTER)) {
            int result = 0;
            result = enter_pressed(textbox_text);
            if (result == 1) {
                char item[128];
                memset(item, 0, 128);
                getListItem(item, list_entries, multi_active);
                if (enter_pressed(item) == 1)
                    memset(textbox_text, 0, 4096);
            }
            else exit(result);
        }

        if (IsKeyPressed(KEY_DOWN)) {
            if (multi_active > 3 && multi_active < entryc-7)
                ++scroll_value;
            if (multi_active < entryc-1)
                ++multi_active;
        }
        if (IsKeyPressed(KEY_UP)) {
            if (multi_active != 0)
                --multi_active;
            if (multi_active < entryc-7)
                --scroll_value;
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            char item[128];
            if (!right_disabled) {
                right_disabled = 2;
                memset(item, 0, 128);
                getListItem(item, list_entries, multi_active);
                strcpy(textbox_text, item);
            }
        }

        BeginDrawing();
        ClearBackground((Color) {255,255,255,0});
        //Draw Main Text Box:
        GuiTextBox(main_text_rect, textbox_text, 4096, 1);
        //check if text has changed:
        //check if main text box is empty:
        if (*textbox_text != 0) {
            window_height = 1080/15 + padding + multi_rect_height;
            if (strcmp(textbox_text, last_text) != 0) { /* only check for entries when input has changed */
                scroll_value = 0;
                multi_active = 0;
                if (right_disabled > 0) right_disabled--;
                strcpy(last_text, textbox_text);
                //get new entries and put in multiview thingy
                put_multitext_entries(list_entries, textbox_text, &entryc);
                scroll_value = 0;
            }
            (void) GuiListView(multi_rect, list_entries, &scroll_value, &multi_active);
        }
        else {
            scroll_value = 0;
            multi_active = 0;
            window_height = 1080/15;
        }
#ifdef DRAW_DEBUG_INFO
        snprintf(info, 64, "mult pos x: %f", window_width / 2 - multi_rect_width / 2);
        DrawText(info, 10, 10, 8, BLACK);
        snprintf(info, 64, "mult pos y: %f", window_height / 2 - main_rect_height - padding);
        DrawText(info, 10, 20, 8, BLACK);
        snprintf(info, 64, "main pos x: %f", window_width / 2 - main_rect_width / 2);
        DrawText(info, 10, 30, 8, BLACK);
        snprintf(info, 64, "main pos y: %f", window_height / 2 - main_rect_height / 2);
        DrawText(info, 10, 40, 8, BLACK);
        snprintf(info, 64, "bin in path: %d", bin_in_path);
        DrawText(info, 10, 50, 8, BLACK);
#endif //DRAW_DEBUG_INFO
        EndDrawing();
    }
    CloseWindow();
    return 0;
}