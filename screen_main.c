#include "screen_draw.h"
#include "log_store.h"
#include "translations.h"
#include <furi_hal_rtc.h>
#include <datetime/datetime.h>

// Animation frame counter (global to persist between draws)
static uint8_t g_anim_frame = 0;

// Calculate age in days
static uint32_t calculate_age_days(AppData* app) {
    DateTime now;
    furi_hal_rtc_get_datetime(&now);
    
    uint32_t now_days = now.year * 365 + now.month * 30 + now.day;
    uint32_t birth_days = app->cfg.birth_y * 365 + app->cfg.birth_m * 30 + app->cfg.birth_d;
    
    if(now_days >= birth_days) {
        return now_days - birth_days;
    }
    return 0;
}

// Format age string as "Xm Yd" (months and days)
static void format_age(AppData* app, char* out, size_t out_size) {
    uint32_t total_days = calculate_age_days(app);
    uint32_t months = total_days / 30;
    uint32_t days = total_days % 30;
    
    if(months == 0) {
        snprintf(out, out_size, "%lud", (unsigned long)days);
    } else {
        snprintf(out, out_size, "%lum %lud", (unsigned long)months, (unsigned long)days);
    }
}

// Format duration string (minutes)
static void format_duration(uint32_t seconds, char* out, size_t out_size) {
    uint32_t minutes = seconds / 60;
    if(minutes < 60) {
        snprintf(out, out_size, "%lum", (unsigned long)minutes);
    } else {
        uint32_t hours = minutes / 60;
        minutes = minutes % 60;
        snprintf(out, out_size, "%luh%lum", (unsigned long)hours, (unsigned long)minutes);
    }
}

// ==== BIRD ANIMATIONS ====
// All birds drawn at position (x, y) which is top-left of ~16x14 area

// Draw bird body (base shape used by all states)
static void draw_bird_body(Canvas* canvas, int x, int y) {
    // Body (oval shape)
    canvas_draw_line(canvas, x+4, y+6, x+12, y+6);   // Top of body
    canvas_draw_line(canvas, x+3, y+7, x+13, y+7);
    canvas_draw_line(canvas, x+2, y+8, x+14, y+8);
    canvas_draw_line(canvas, x+2, y+9, x+14, y+9);
    canvas_draw_line(canvas, x+3, y+10, x+13, y+10);
    canvas_draw_line(canvas, x+4, y+11, x+12, y+11); // Bottom of body
    
    // Tail (left side)
    canvas_draw_line(canvas, x, y+7, x+3, y+8);
    canvas_draw_line(canvas, x, y+8, x+2, y+9);
    canvas_draw_line(canvas, x+1, y+9, x+3, y+10);
    
    // Legs
    canvas_draw_line(canvas, x+7, y+12, x+7, y+14);
    canvas_draw_line(canvas, x+10, y+12, x+10, y+14);
    canvas_draw_dot(canvas, x+6, y+14);
    canvas_draw_dot(canvas, x+8, y+14);
    canvas_draw_dot(canvas, x+9, y+14);
    canvas_draw_dot(canvas, x+11, y+14);
}

// Awake bird - standing, blinking
static void draw_bird_awake(Canvas* canvas, int x, int y, uint8_t frame) {
    draw_bird_body(canvas, x, y);
    
    // Head
    canvas_draw_line(canvas, x+10, y+2, x+14, y+2);
    canvas_draw_line(canvas, x+9, y+3, x+15, y+3);
    canvas_draw_line(canvas, x+9, y+4, x+15, y+4);
    canvas_draw_line(canvas, x+10, y+5, x+14, y+5);
    
    // Eye (blinks every 8 frames)
    if((frame % 16) < 14) {
        canvas_draw_dot(canvas, x+12, y+3); // Open eye
        canvas_draw_dot(canvas, x+12, y+4);
    } else {
        canvas_draw_line(canvas, x+11, y+4, x+13, y+4); // Closed eye (blink)
    }
    
    // Beak
    canvas_draw_dot(canvas, x+16, y+4);
    canvas_draw_dot(canvas, x+17, y+4);
    
    // Wing detail
    canvas_draw_line(canvas, x+5, y+7, x+8, y+9);
}

// Feeding bird - pecking animation
static void draw_bird_feeding(Canvas* canvas, int x, int y, uint8_t frame) {
    int peck = (frame % 8) < 4 ? 0 : 2; // Pecking motion
    
    draw_bird_body(canvas, x, y + peck);
    
    // Head (lower when pecking)
    int hy = y + peck + (peck > 0 ? 1 : 0);
    canvas_draw_line(canvas, x+10, hy+2, x+14, hy+2);
    canvas_draw_line(canvas, x+9, hy+3, x+15, hy+3);
    canvas_draw_line(canvas, x+9, hy+4, x+15, hy+4);
    canvas_draw_line(canvas, x+10, hy+5, x+14, hy+5);
    
    // Eye
    canvas_draw_dot(canvas, x+12, hy+3);
    
    // Beak (open when pecking)
    if(peck > 0) {
        canvas_draw_dot(canvas, x+16, hy+3);
        canvas_draw_dot(canvas, x+17, hy+3);
        canvas_draw_dot(canvas, x+16, hy+5);
    } else {
        canvas_draw_dot(canvas, x+16, hy+4);
        canvas_draw_dot(canvas, x+17, hy+4);
    }
    
    // Food dots (appearing/disappearing)
    if((frame % 16) < 8) {
        canvas_draw_dot(canvas, x+18, y+13);
        canvas_draw_dot(canvas, x+20, y+12);
    }
    if((frame % 16) >= 4 && (frame % 16) < 12) {
        canvas_draw_dot(canvas, x+16, y+14);
    }
}

// Sleeping bird - Zzz animation
static void draw_bird_sleeping(Canvas* canvas, int x, int y, uint8_t frame) {
    draw_bird_body(canvas, x, y);
    
    // Head (tucked)
    canvas_draw_line(canvas, x+10, y+3, x+14, y+3);
    canvas_draw_line(canvas, x+9, y+4, x+15, y+4);
    canvas_draw_line(canvas, x+9, y+5, x+15, y+5);
    canvas_draw_line(canvas, x+10, y+6, x+14, y+6);
    
    // Closed eyes (sleeping)
    canvas_draw_line(canvas, x+11, y+4, x+13, y+4);
    
    // Small beak
    canvas_draw_dot(canvas, x+16, y+5);
    
    // Zzz animation (floating up)
    int zoff = (frame % 12) / 4; // 0, 1, 2 offset
    canvas_set_font(canvas, FontSecondary);
    if(zoff == 0) {
        canvas_draw_str(canvas, x+15, y+2, "z");
    } else if(zoff == 1) {
        canvas_draw_str(canvas, x+15, y+1, "z");
        canvas_draw_str(canvas, x+18, y-1, "z");
    } else {
        canvas_draw_str(canvas, x+15, y, "z");
        canvas_draw_str(canvas, x+18, y-2, "z");
        canvas_draw_str(canvas, x+21, y-4, "Z");
    }
}

// Diaper logged - happy bird with hearts
static void draw_bird_diaper(Canvas* canvas, int x, int y, uint8_t frame) {
    draw_bird_body(canvas, x, y);
    
    // Happy head
    canvas_draw_line(canvas, x+10, y+2, x+14, y+2);
    canvas_draw_line(canvas, x+9, y+3, x+15, y+3);
    canvas_draw_line(canvas, x+9, y+4, x+15, y+4);
    canvas_draw_line(canvas, x+10, y+5, x+14, y+5);
    
    // Happy eyes (^_^)
    canvas_draw_dot(canvas, x+11, y+3);
    canvas_draw_dot(canvas, x+13, y+3);
    
    // Open beak (chirping!)
    canvas_draw_dot(canvas, x+16, y+3);
    canvas_draw_dot(canvas, x+16, y+5);
    
    // Hearts/stars floating
    int hoff = (frame % 8) / 2;
    if(hoff < 3) {
        // Draw small heart at offset position
        int hx = x + 17 + hoff;
        int hy = y - hoff;
        canvas_draw_dot(canvas, hx, hy+1);
        canvas_draw_dot(canvas, hx+2, hy+1);
        canvas_draw_dot(canvas, hx, hy+2);
        canvas_draw_dot(canvas, hx+1, hy+2);
        canvas_draw_dot(canvas, hx+2, hy+2);
        canvas_draw_dot(canvas, hx+1, hy+3);
    }
    
    // Wing flapping
    if((frame % 4) < 2) {
        canvas_draw_line(canvas, x+5, y+5, x+8, y+3); // Wing up
    } else {
        canvas_draw_line(canvas, x+5, y+7, x+8, y+9); // Wing down
    }
}

void draw_main(AppData* app, Canvas* canvas) {
    canvas_clear(canvas);
    
    // Increment animation frame
    g_anim_frame++;
    
    // Child name (top left)
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, app->cfg.name);
    
    // Age (top right area)
    char age_str[16];
    format_age(app, age_str, sizeof(age_str));
    canvas_draw_str(canvas, 80, 10, age_str);
    
    // Draw animated bird based on state (right side of screen)
    int bird_x = 100;
    int bird_y = 16;
    
    uint32_t now = log_get_timestamp();
    bool show_diaper_feedback = (app->state.last_diaper_ts > 0 && 
        now >= app->state.last_diaper_ts && 
        (now - app->state.last_diaper_ts) < 3);
    
    if(show_diaper_feedback) {
        draw_bird_diaper(canvas, bird_x, bird_y, g_anim_frame);
    } else if(app->state.feeding_active) {
        draw_bird_feeding(canvas, bird_x, bird_y, g_anim_frame);
    } else if(app->state.sleep_active) {
        draw_bird_sleeping(canvas, bird_x, bird_y, g_anim_frame);
    } else {
        draw_bird_awake(canvas, bird_x, bird_y, g_anim_frame);
    }
    
    // Status text (below name)
    canvas_set_font(canvas, FontSecondary);
    int y_pos = 24;
    
    if(app->state.feeding_active) {
        uint32_t duration = (now > app->state.feeding_start_ts) ? 
            (now - app->state.feeding_start_ts) : 0;
        char dur_str[16];
        format_duration(duration, dur_str, sizeof(dur_str));
        char status[32];
        snprintf(status, sizeof(status), "%s %s", tr_get(app, TR_MAIN_FEEDING), dur_str);
        canvas_draw_str(canvas, 2, y_pos, status);
    } else if(app->state.sleep_active) {
        uint32_t duration = (now > app->state.sleep_start_ts) ? 
            (now - app->state.sleep_start_ts) : 0;
        char dur_str[16];
        format_duration(duration, dur_str, sizeof(dur_str));
        char status[32];
        snprintf(status, sizeof(status), "%s %s", tr_get(app, TR_MAIN_SLEEPING), dur_str);
        canvas_draw_str(canvas, 2, y_pos, status);
    } else {
        canvas_draw_str(canvas, 2, y_pos, tr_get(app, TR_MAIN_AWAKE));
    }
    
    // Quick actions at bottom
    canvas_set_font(canvas, FontSecondary);
    
    // Row 1 at y=44: UP arrow + Feed, DOWN arrow + Sleep  
    canvas_draw_line(canvas, 4, 44, 7, 38);
    canvas_draw_line(canvas, 7, 38, 10, 44);
    canvas_draw_str(canvas, 14, 44, tr_get(app, TR_MAIN_FEED_LABEL));
    
    canvas_draw_line(canvas, 54, 38, 57, 44);
    canvas_draw_line(canvas, 57, 44, 60, 38);
    canvas_draw_str(canvas, 64, 44, tr_get(app, TR_MAIN_SLEEP_LABEL));
    
    // Row 2 at y=56: LEFT arrow + Diaper, BACK + Menu
    canvas_draw_line(canvas, 2, 53, 8, 50);
    canvas_draw_line(canvas, 2, 53, 8, 56);
    canvas_draw_str(canvas, 14, 56, tr_get(app, TR_MAIN_DIAPER_LABEL));
    
    canvas_draw_str(canvas, 54, 56, "B:");
    canvas_draw_str(canvas, 66, 56, tr_get(app, TR_MAIN_MENU_LABEL));
    
    // Diaper feedback text overlay
    if(show_diaper_feedback) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 2, 30, 70, 12);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str(canvas, 4, 39, tr_get(app, TR_MAIN_DIAPER_LOGGED));
        canvas_set_color(canvas, ColorBlack);
    }
}
