#include "screen_draw.h"
#include "translations.h"

#define MENU_ITEMS 6

void draw_menu(AppData* app, Canvas* canvas) {
    canvas_clear(canvas);
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Menu");
    
    canvas_set_font(canvas, FontSecondary);
    
    // Menu items: Profile, Recent log, Delete last, Clear all, Language, Exit
    const char* menu_items[MENU_ITEMS] = {
        tr_get(app, TR_MENU_PROFILE),
        tr_get(app, TR_MENU_RECENT_LOG),
        tr_get(app, TR_MENU_DELETE_LAST),
        tr_get(app, TR_MENU_CLEAR_ALL),
        tr_get(app, TR_MENU_LANGUAGE),
        tr_get(app, TR_MENU_EXIT)
    };
    
    int start_y = 24;
    int line_height = 10;
    
    // Show up to 4 items with scrolling
    int visible_count = 4;
    int start_idx = app->menu_index - 1; // Keep cursor in middle when possible
    if(start_idx < 0) start_idx = 0;
    if(start_idx > MENU_ITEMS - visible_count) start_idx = MENU_ITEMS - visible_count;
    
    for(int i = 0; i < visible_count; i++) {
        int idx = start_idx + i;
        if(idx >= MENU_ITEMS) break;
        
        int y = start_y + i * line_height;
        
        // Draw cursor
        if(idx == app->menu_index) {
            canvas_draw_str(canvas, 0, y, ">");
        }
        
        // Draw menu item
        canvas_draw_str(canvas, 8, y, menu_items[idx]);
    }
    
    // Scroll indicator
    if(MENU_ITEMS > visible_count) {
        char scroll[16];
        snprintf(scroll, sizeof(scroll), "%d/%d", app->menu_index + 1, MENU_ITEMS);
        canvas_draw_str(canvas, 100, 10, scroll);
    }
}
