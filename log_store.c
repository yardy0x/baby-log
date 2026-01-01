#include "app_state.h"
#include <storage/storage.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>
#include <furi.h>
#include <furi_hal_rtc.h>
#include <string.h>

#define LOG_PATH EXT_PATH("apps/baby_log/log.jsonl")
#define DELETED_PATH EXT_PATH("apps/baby_log/deleted.jsonl")
#define DEBUG_PATH EXT_PATH("apps/baby_log/debug.log")

// ==== DEBUG LOGGING ====
// Write debug message to file for crash investigation
static void debug_log(const char* msg) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage) return;
    
    storage_common_mkdir(storage, EXT_PATH("apps/baby_log"));
    
    Stream* stream = buffered_file_stream_alloc(storage);
    if(!stream) {
        furi_record_close(RECORD_STORAGE);
        return;
    }
    
    if(buffered_file_stream_open(stream, DEBUG_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        stream_write(stream, (uint8_t*)msg, strlen(msg));
        stream_write(stream, (uint8_t*)"\n", 1);
        buffered_file_stream_close(stream);
    }
    
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}

// Generate a simple hex ID from timestamp
static void generate_id(uint32_t ts, char* id_out) {
    snprintf(id_out, 33, "%08lx%08lx", (unsigned long)ts, (unsigned long)(ts * 1103515245 + 12345));
}

// Add ID to deleted list
static void mark_deleted(const char* id) {
    if(!id || id[0] == '\0' || strlen(id) > 32) return;
    
    debug_log("mark_deleted: start");
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage) return;
    
    storage_common_mkdir(storage, EXT_PATH("apps/baby_log"));
    
    Stream* stream = buffered_file_stream_alloc(storage);
    if(!stream) {
        furi_record_close(RECORD_STORAGE);
        return;
    }
    
    if(buffered_file_stream_open(stream, DELETED_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        char buffer[64]; // Small buffer
        int written = snprintf(buffer, sizeof(buffer), "{\"id\":\"%s\"}\n", id);
        if(written > 0 && written < (int)sizeof(buffer)) {
            stream_write(stream, (uint8_t*)buffer, written);
        }
        buffered_file_stream_close(stream);
    }
    
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
    debug_log("mark_deleted: done");
}

// Get current timestamp (Unix epoch seconds)
uint32_t log_get_timestamp(void) {
    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    
    uint32_t days = 0;
    uint32_t year = datetime.year;
    uint32_t month = datetime.month;
    uint32_t day = datetime.day;
    
    days = (year - 1970) * 365 + (year - 1969) / 4;
    days += (month - 1) * 30;
    days += day - 1;
    
    return days * 86400 + datetime.hour * 3600 + datetime.minute * 60 + datetime.second;
}

// Format timestamp to string
static void format_timestamp(uint32_t ts, char* out, size_t out_size) {
    UNUSED(ts);
    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    snprintf(out, out_size, "%04u-%02u-%02uT%02u:%02u:%02u",
        datetime.year, datetime.month, datetime.day,
        datetime.hour, datetime.minute, datetime.second);
}

// Log an event
void log_add_event(AppData* app, LogEventType type, uint32_t duration) {
    UNUSED(app);
    
    debug_log("log_add_event: start");
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, EXT_PATH("apps/baby_log"));
    
    Stream* stream = buffered_file_stream_alloc(storage);
    
    if(buffered_file_stream_open(stream, LOG_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        uint32_t ts = log_get_timestamp();
        char id[33];
        generate_id(ts, id);
        
        char time_str[32];
        format_timestamp(ts, time_str, sizeof(time_str));
        
        const char* type_str = "unknown";
        switch(type) {
            case LogEventDiaper: type_str = "diaper"; break;
            case LogEventFeedingStart: type_str = "feeding_start"; break;
            case LogEventFeedingStop: type_str = "feeding_stop"; break;
            case LogEventSleepStart: type_str = "sleep_start"; break;
            case LogEventSleepStop: type_str = "sleep_stop"; break;
        }
        
        char buffer[128]; // Reduced from 256
        if(duration > 0) {
            snprintf(buffer, sizeof(buffer),
                "{\"ts\":\"%s\",\"type\":\"%s\",\"id\":\"%s\",\"dur\":%lu}\n",
                time_str, type_str, id, (unsigned long)duration);
        } else {
            snprintf(buffer, sizeof(buffer),
                "{\"ts\":\"%s\",\"type\":\"%s\",\"id\":\"%s\"}\n",
                time_str, type_str, id);
        }
        stream_write(stream, (uint8_t*)buffer, strlen(buffer));
        buffered_file_stream_close(stream);
    }
    
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
    debug_log("log_add_event: done");
}

// Parse event type from line
static LogEventType parse_event_type(const char* line) {
    if(strstr(line, "\"type\":\"diaper\"")) return LogEventDiaper;
    if(strstr(line, "\"type\":\"feeding_start\"")) return LogEventFeedingStart;
    if(strstr(line, "\"type\":\"feeding_stop\"")) return LogEventFeedingStop;
    if(strstr(line, "\"type\":\"sleep_start\"")) return LogEventSleepStart;
    if(strstr(line, "\"type\":\"sleep_stop\"")) return LogEventSleepStop;
    return LogEventDiaper; // Default
}

// Extract ID from line into buffer (max 32 chars)
static bool extract_id(const char* line, char* id_out) {
    const char* id_pos = strstr(line, "\"id\":\"");
    if(!id_pos) {
        id_out[0] = '\0';
        return false;
    }
    id_pos += 6;
    int i = 0;
    while(*id_pos != '"' && *id_pos != '\0' && i < 32) {
        id_out[i++] = *id_pos++;
    }
    id_out[i] = '\0';
    return (i > 0);
}

// Extract duration from line
static uint32_t extract_duration(const char* line) {
    const char* dur_pos = strstr(line, "\"dur\":");
    if(!dur_pos) {
        dur_pos = strstr(line, "\"duration\":");
        if(!dur_pos) return 0;
        dur_pos += 11;
    } else {
        dur_pos += 6;
    }
    uint32_t dur = 0;
    while(*dur_pos >= '0' && *dur_pos <= '9') {
        dur = dur * 10 + (*dur_pos - '0');
        dur_pos++;
    }
    return dur;
}

// ==== ULTRA-SIMPLE LOG LOADING ====
// Uses STATIC buffer to avoid stack overflow
// Only loads last 5 entries to minimize memory usage
static char g_log_buffer[512]; // Global buffer - not on stack!
static char g_deleted_buffer[256]; // Global buffer for deleted IDs

void log_load_recent(AppData* app) {
    debug_log("log_load_recent: start");
    
    if(!app) {
        debug_log("log_load_recent: app is NULL");
        return;
    }
    
    // Clear entries first
    app->log_count = 0;
    for(int i = 0; i < MAX_LOG_ENTRIES; i++) {
        app->log_entries[i].id[0] = '\0';
        app->log_entries[i].type = LogEventDiaper;
        app->log_entries[i].timestamp = 0;
        app->log_entries[i].duration = 0;
    }
    
    debug_log("log_load_recent: loading deleted");
    
    // Load deleted IDs into global buffer
    g_deleted_buffer[0] = '\0';
    {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        if(storage) {
            Stream* stream = buffered_file_stream_alloc(storage);
            if(stream) {
                if(buffered_file_stream_open(stream, DELETED_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
                    size_t size = stream_size(stream);
                    if(size > 0 && size < sizeof(g_deleted_buffer)) {
                        size_t read = stream_read(stream, (uint8_t*)g_deleted_buffer, size);
                        if(read < sizeof(g_deleted_buffer)) {
                            g_deleted_buffer[read] = '\0';
                        }
                    }
                    buffered_file_stream_close(stream);
                }
                stream_free(stream);
            }
            furi_record_close(RECORD_STORAGE);
        }
    }
    
    debug_log("log_load_recent: opening log file");
    
    // Open log file
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage) {
        debug_log("log_load_recent: storage failed");
        return;
    }
    
    Stream* stream = buffered_file_stream_alloc(storage);
    if(!stream) {
        debug_log("log_load_recent: stream alloc failed");
        furi_record_close(RECORD_STORAGE);
        return;
    }
    
    if(!buffered_file_stream_open(stream, LOG_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        debug_log("log_load_recent: file open failed (no logs yet)");
        stream_free(stream);
        furi_record_close(RECORD_STORAGE);
        return;
    }
    
    size_t file_size = stream_size(stream);
    debug_log("log_load_recent: file opened");
    
    if(file_size == 0) {
        debug_log("log_load_recent: file empty");
        buffered_file_stream_close(stream);
        stream_free(stream);
        furi_record_close(RECORD_STORAGE);
        return;
    }
    
    // Read only last 512 bytes (about 4-5 log entries)
    size_t read_size = (file_size > sizeof(g_log_buffer) - 1) ? sizeof(g_log_buffer) - 1 : file_size;
    size_t skip_size = (file_size > read_size) ? file_size - read_size : 0;
    
    // Skip to near end of file by reading and discarding
    if(skip_size > 0) {
        debug_log("log_load_recent: skipping bytes");
        char skip_byte;
        for(size_t i = 0; i < skip_size; i++) {
            if(stream_read(stream, (uint8_t*)&skip_byte, 1) != 1) break;
        }
    }
    
    debug_log("log_load_recent: reading data");
    size_t read = stream_read(stream, (uint8_t*)g_log_buffer, read_size);
    if(read < sizeof(g_log_buffer)) {
        g_log_buffer[read] = '\0';
    } else {
        g_log_buffer[sizeof(g_log_buffer) - 1] = '\0';
    }
    
    buffered_file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
    
    debug_log("log_load_recent: parsing lines");
    
    // Parse lines - simple approach
    // Find lines and store pointers
    char* lines[10];
    int line_count = 0;
    
    char* pos = g_log_buffer;
    while(*pos && line_count < 10) {
        // Skip whitespace
        while(*pos == '\n' || *pos == '\r') pos++;
        if(*pos == '\0') break;
        
        // Mark line start
        lines[line_count] = pos;
        
        // Find line end
        while(*pos && *pos != '\n' && *pos != '\r') pos++;
        
        // Null-terminate line
        if(*pos) {
            *pos = '\0';
            pos++;
        }
        
        line_count++;
    }
    
    debug_log("log_load_recent: processing entries");
    
    // Process lines (most recent last in file)
    int entry_count = 0;
    for(int i = 0; i < line_count && entry_count < MAX_LOG_ENTRIES; i++) {
        char* line = lines[i];
        if(!line || strlen(line) < 10) continue;
        
        // Check if starts with '{'
        if(line[0] != '{') continue;
        
        // Extract ID
        char id[33];
        if(!extract_id(line, id)) continue;
        
        // Check if deleted
        if(g_deleted_buffer[0] != '\0') {
            char search[48];
            snprintf(search, sizeof(search), "\"id\":\"%s\"", id);
            if(strstr(g_deleted_buffer, search)) continue; // Skip deleted
        }
        
        // Add entry
        LogEntry* entry = &app->log_entries[entry_count];
        strncpy(entry->id, id, 32);
        entry->id[32] = '\0';
        entry->type = parse_event_type(line);
        entry->duration = extract_duration(line);
        entry->timestamp = log_get_timestamp(); // Use current time as approx
        
        entry_count++;
    }
    
    app->log_count = entry_count;
    
    debug_log("log_load_recent: done");
}

// Delete last event - ultra simple
void log_delete_last(AppData* app) {
    debug_log("log_delete_last: start");
    
    if(!app) {
        debug_log("log_delete_last: app is NULL");
        return;
    }
    
    if(app->log_count <= 0 || app->log_count > MAX_LOG_ENTRIES) {
        debug_log("log_delete_last: invalid count");
        return;
    }
    
    int idx = app->log_count - 1;
    if(idx < 0 || idx >= MAX_LOG_ENTRIES) {
        debug_log("log_delete_last: invalid index");
        return;
    }
    
    LogEntry* entry = &app->log_entries[idx];
    if(entry->id[0] == '\0') {
        debug_log("log_delete_last: empty ID");
        return;
    }
    
    debug_log("log_delete_last: marking deleted");
    mark_deleted(entry->id);
    
    debug_log("log_delete_last: reloading");
    log_load_recent(app);
    
    debug_log("log_delete_last: done");
}

// Clear all logs - delete both log and deleted files
void log_clear_all(AppData* app) {
    debug_log("log_clear_all: start");
    
    if(!app) return;
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage) {
        debug_log("log_clear_all: storage failed");
        return;
    }
    
    // Delete the log file
    storage_common_remove(storage, LOG_PATH);
    debug_log("log_clear_all: removed log.jsonl");
    
    // Delete the deleted IDs file
    storage_common_remove(storage, DELETED_PATH);
    debug_log("log_clear_all: removed deleted.jsonl");
    
    // Delete the debug log too
    storage_common_remove(storage, DEBUG_PATH);
    
    furi_record_close(RECORD_STORAGE);
    
    // Clear in-memory entries
    app->log_count = 0;
    for(int i = 0; i < MAX_LOG_ENTRIES; i++) {
        app->log_entries[i].id[0] = '\0';
        app->log_entries[i].type = LogEventDiaper;
        app->log_entries[i].timestamp = 0;
        app->log_entries[i].duration = 0;
    }
    
    debug_log("log_clear_all: done");
}
