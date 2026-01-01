#include "screen_draw.h"
#include "translations.h"
#include <furi_hal_rtc.h>
#include <datetime/datetime.h>

// Check if date is valid
static bool is_valid_date(uint16_t year, uint8_t month, uint8_t day) {
    if(year < 2000 || year > 2100) return false;
    if(month < 1 || month > 12) return false;
    if(day < 1 || day > 31) return false;
    
    // Check month-specific day limits
    if(month == 2 && day > 29) return false;
    if((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) return false;
    
    // Check not in future
    DateTime now;
    furi_hal_rtc_get_datetime(&now);
    if(year > now.year) return false;
    if(year == now.year && month > now.month) return false;
    if(year == now.year && month == now.month && day > now.day) return false;
    
    return true;
}

void draw_edit_birthdate(AppData* app, Canvas* canvas) {
    canvas_clear(canvas);
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, tr_get(app, TR_EDIT_DATE_TITLE));
    
    canvas_set_font(canvas, FontSecondary);
    
    // Year
    int y_pos = 28;
    if(app->edit_date_field == 0) {
        canvas_draw_str(canvas, 0, y_pos, ">");
    }
    char year_str[32];
    snprintf(year_str, sizeof(year_str), "%s %u", tr_get(app, TR_EDIT_DATE_YEAR), app->edit_year);
    canvas_draw_str(canvas, 8, y_pos, year_str);
    
    // Month
    y_pos = 40;
    if(app->edit_date_field == 1) {
        canvas_draw_str(canvas, 0, y_pos, ">");
    }
    char month_str[32];
    snprintf(month_str, sizeof(month_str), "%s %u", tr_get(app, TR_EDIT_DATE_MONTH), app->edit_month);
    canvas_draw_str(canvas, 8, y_pos, month_str);
    
    // Day
    y_pos = 52;
    if(app->edit_date_field == 2) {
        canvas_draw_str(canvas, 0, y_pos, ">");
    }
    char day_str[32];
    snprintf(day_str, sizeof(day_str), "%s %u", tr_get(app, TR_EDIT_DATE_DAY), app->edit_day);
    canvas_draw_str(canvas, 8, y_pos, day_str);
    
    // Validation status (inline)
    if(!is_valid_date(app->edit_year, app->edit_month, app->edit_day)) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 70, y_pos - 1, 58, 10);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str(canvas, 72, y_pos + 7, tr_get(app, TR_EDIT_DATE_INVALID));
        canvas_set_color(canvas, ColorBlack);
    }
}

