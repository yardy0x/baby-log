#include "screen_draw.h"
#include "translations.h"
#include <furi_hal_rtc.h>
#include <datetime/datetime.h>

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

// Format age string
static void format_age(AppData* app, char* out, size_t out_size) {
    uint32_t days = calculate_age_days(app);
    
    // Note: This function needs app for translations, but we'll format here
    // The caller will add the translated unit
    if(days < 7) {
        snprintf(out, out_size, "%lu", (unsigned long)days);
    } else if(days < 30) {
        uint32_t weeks = days / 7;
        snprintf(out, out_size, "%lu", (unsigned long)weeks);
    } else {
        uint32_t months = days / 30;
        snprintf(out, out_size, "%lu", (unsigned long)months);
    }
}

void draw_profile(AppData* app, Canvas* canvas) {
    canvas_clear(canvas);
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, tr_get(app, TR_PROFILE_TITLE));
    
    canvas_set_font(canvas, FontSecondary);
    
    // Name
    char name_line[32];
    snprintf(name_line, sizeof(name_line), "%s %s", tr_get(app, TR_PROFILE_NAME), app->cfg.name);
    canvas_draw_str(canvas, 2, 24, name_line);
    
    // Birth date
    char date_line[32];
    snprintf(date_line, sizeof(date_line), "%s %04u-%02u-%02u",
        tr_get(app, TR_PROFILE_BIRTH), app->cfg.birth_y, app->cfg.birth_m, app->cfg.birth_d);
    canvas_draw_str(canvas, 2, 36, date_line);
    
    // Age
    char age_num[16];
    format_age(app, age_num, sizeof(age_num));
    uint32_t days = calculate_age_days(app);
    const char* age_unit = "";
    if(days < 7) {
        age_unit = tr_get(app, TR_AGE_DAYS);
    } else if(days < 30) {
        age_unit = tr_get(app, TR_AGE_WEEKS);
    } else {
        age_unit = tr_get(app, TR_AGE_MONTHS);
    }
    char age_line[40];
    snprintf(age_line, sizeof(age_line), "%s %s %s", tr_get(app, TR_PROFILE_AGE), age_num, age_unit);
    canvas_draw_str(canvas, 2, 48, age_line);
    
    // Menu options (back to 2 options - Language moved to main menu)
    int start_y = 56;
    const char* options[2] = {
        tr_get(app, TR_PROFILE_EDIT_NAME),
        tr_get(app, TR_PROFILE_EDIT_DATE)
    };
    
    for(int i = 0; i < 2; i++) {
        int y = start_y + i * 8;
        if(i == app->profile_index) {
            canvas_draw_str(canvas, 0, y, ">");
        }
        canvas_draw_str(canvas, 8, y, options[i]);
    }
}
