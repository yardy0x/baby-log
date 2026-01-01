#include "screen_draw.h"
#include "translations.h"

void draw_language(AppData* app, Canvas* canvas) {
    canvas_clear(canvas);
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, tr_get(app, TR_LANG_TITLE));
    
    canvas_set_font(canvas, FontSecondary);
    
    const char* languages[3] = {
        tr_get(app, TR_LANG_ENGLISH),
        tr_get(app, TR_LANG_RUSSIAN),
        tr_get(app, TR_LANG_SPANISH)
    };
    
    int start_y = 24;
    int line_height = 12;
    
    for(int i = 0; i < 3; i++) {
        int y = start_y + i * line_height;
        
        // Draw cursor
        if(i == app->lang_index) {
            canvas_draw_str(canvas, 0, y, ">");
        }
        
        // Draw language name
        canvas_draw_str(canvas, 8, y, languages[i]);
        
        // Show checkmark for current language
        if(i == app->cfg.language) {
            canvas_draw_str(canvas, 110, y, "*");
        }
    }
}

