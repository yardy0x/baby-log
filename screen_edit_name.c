#include "screen_draw.h"
#include "translations.h"

void draw_edit_name(AppData* app, Canvas* canvas) {
    canvas_clear(canvas);
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, tr_get(app, TR_EDIT_NAME_TITLE));
    
    // Current name being edited
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, tr_get(app, TR_EDIT_NAME_NAME));
    
    // Draw name - only show filled characters plus cursor position
    int x = 2;
    int y = 34;
    size_t name_len = strlen(app->edit_name);
    int max_show = ((int)name_len > app->edit_name_pos) ? (int)name_len : app->edit_name_pos + 1;
    max_show = (max_show < MAX_NAME_LEN) ? max_show : MAX_NAME_LEN;
    
    // Show characters up to cursor position + 1
    for(int i = 0; i < max_show; i++) {
        if(i == app->edit_name_pos) {
            // Draw cursor box
            canvas_draw_box(canvas, x + i * 8 - 1, y - 8, 10, 10);
            canvas_invert_color(canvas);
        }
        
        char c;
        if((size_t)i < name_len) {
            c = app->edit_name[i];
            if(c == ' ') c = '_'; // Show space as underscore for visibility
        } else {
            c = 'A' + app->edit_name_char; // Show current selection
        }
        char str[2] = {c, '\0'};
        canvas_draw_str(canvas, x + i * 8, y, str);
        
        if(i == app->edit_name_pos) {
            canvas_invert_color(canvas);
        }
    }
    
    // Current character selector
    char current_char;
    if((size_t)app->edit_name_pos < name_len) {
        current_char = app->edit_name[app->edit_name_pos];
        if(current_char == ' ') current_char = '_';
    } else {
        if(app->edit_name_char == 26) {
            current_char = '_'; // Space shown as underscore
        } else {
            current_char = 'A' + app->edit_name_char;
        }
    }
    
    // Show current letter and instructions (compact)
    char letter_str[24];
    snprintf(letter_str, sizeof(letter_str), "%s %c", tr_get(app, TR_EDIT_NAME_CHAR), current_char);
    canvas_draw_str(canvas, 2, 46, letter_str);
    // Combine instructions on one line to fit screen
    char instr[32];
    snprintf(instr, sizeof(instr), "%s %s", tr_get(app, TR_EDIT_NAME_UP_DOWN), tr_get(app, TR_EDIT_NAME_L_R));
    canvas_draw_str(canvas, 2, 56, instr);
}
