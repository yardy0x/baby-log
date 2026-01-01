#include "screen_draw.h"
#include "translations.h"
#include <furi_hal_rtc.h>
#include <datetime/datetime.h>

// Format time from timestamp (simplified - show relative or HH:MM)
static void format_time(uint32_t ts, char* out, size_t out_size) {
    UNUSED(ts);
    // Use current time approximation
    DateTime now;
    furi_hal_rtc_get_datetime(&now);
    snprintf(out, out_size, "%02u:%02u", now.hour, now.minute);
}

// Get event type string (translated)
static const char* get_event_type_str(AppData* app, LogEventType type) {
    switch(type) {
        case LogEventDiaper: return tr_get(app, TR_LOG_DIAPER);
        case LogEventFeedingStart: return tr_get(app, TR_LOG_FEED_START);
        case LogEventFeedingStop: return tr_get(app, TR_LOG_FEED_STOP);
        case LogEventSleepStart: return tr_get(app, TR_LOG_SLEEP_START);
        case LogEventSleepStop: return tr_get(app, TR_LOG_SLEEP_STOP);
        default: return "Unknown";
    }
}

// Format duration for stop events
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

void draw_recent_log(AppData* app, Canvas* canvas) {
    if(!app || !canvas) return;
    
    canvas_clear(canvas);
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, tr_get(app, TR_LOG_TITLE));
    
    // Validate log_count is in safe range
    if(app->log_count < 0 || app->log_count > MAX_LOG_ENTRIES) {
        app->log_count = 0;
    }
    
    if(app->log_count == 0) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 30, tr_get(app, TR_LOG_NO_EVENTS));
        return;
    }
    
    canvas_set_font(canvas, FontSecondary);
    
    // Validate log_index
    if(app->log_index < 0) app->log_index = 0;
    if(app->log_index >= app->log_count) app->log_index = app->log_count - 1;
    
    // Show up to 4 entries (safer for screen space)
    int visible_count = (app->log_count < 4) ? app->log_count : 4;
    int start_idx = app->log_index;
    if(start_idx + visible_count > app->log_count) {
        start_idx = app->log_count - visible_count;
        if(start_idx < 0) start_idx = 0;
    }
    
    int y_pos = 24;
    int line_height = 10; // Reduced to fit better
    
    for(int i = 0; i < visible_count; i++) {
        int idx = start_idx + i;
        
        // Extra bounds checking
        if(idx < 0 || idx >= app->log_count || idx >= MAX_LOG_ENTRIES) {
            break;
        }
        
        LogEntry* entry = &app->log_entries[idx];
        if(!entry) break;
        
        char time_str[16];
        format_time(entry->timestamp, time_str, sizeof(time_str));
        
        const char* type_str = get_event_type_str(app, entry->type);
        if(!type_str) type_str = "?";
        
        char line[64];
        if(entry->duration > 0 && 
           (entry->type == LogEventFeedingStop || entry->type == LogEventSleepStop)) {
            char dur_str[16];
            format_duration(entry->duration, dur_str, sizeof(dur_str));
            snprintf(line, sizeof(line), "%s %s %s", time_str, type_str, dur_str);
        } else {
            snprintf(line, sizeof(line), "%s %s", time_str, type_str);
        }
        
        canvas_draw_str(canvas, 2, y_pos + i * line_height, line);
    }
    
    // Scroll indicator (only if more than visible)
    if(app->log_count > visible_count) {
        char scroll_str[24];
        snprintf(scroll_str, sizeof(scroll_str), "%d/%d", start_idx + 1, app->log_count);
        canvas_draw_str(canvas, 100, 10, scroll_str);
    }
}
