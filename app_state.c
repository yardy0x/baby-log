#include "app_state.h"
#include <storage/storage.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>
#include <furi.h>

#define CONFIG_PATH EXT_PATH("apps/baby_log/config.json")
#define STATE_PATH EXT_PATH("apps/baby_log/state.json")

// Simple JSON parsing helpers (no malloc)
static int parse_int(const char* str, int* val) {
    *val = 0;
    int sign = 1;
    if(*str == '-') {
        sign = -1;
        str++;
    }
    while(*str >= '0' && *str <= '9') {
        *val = *val * 10 + (*str - '0');
        str++;
    }
    *val *= sign;
    return 1;
}

static int parse_uint16(const char* str, uint16_t* val) {
    *val = 0;
    while(*str >= '0' && *str <= '9') {
        *val = *val * 10 + (*str - '0');
        str++;
    }
    return 1;
}

static int parse_uint8(const char* str, uint8_t* val) {
    *val = 0;
    while(*str >= '0' && *str <= '9') {
        *val = *val * 10 + (*str - '0');
        str++;
    }
    return 1;
}

static const char* find_key(const char* json, const char* key) {
    char key_pattern[64];
    snprintf(key_pattern, sizeof(key_pattern), "\"%s\"", key);
    const char* pos = strstr(json, key_pattern);
    if(!pos) return NULL;
    pos = strchr(pos, ':');
    if(!pos) return NULL;
    pos++;
    while(*pos == ' ' || *pos == '\t') pos++;
    return pos;
}

static void extract_string(const char* start, char* out, size_t max_len) {
    if(*start != '"') {
        out[0] = '\0';
        return;
    }
    start++;
    size_t i = 0;
    while(*start != '"' && *start != '\0' && i < max_len - 1) {
        out[i++] = *start++;
    }
    out[i] = '\0';
}

void load_config(AppData* app) {
    // Default values
    strncpy(app->cfg.name, "Baby", sizeof(app->cfg.name) - 1);
    app->cfg.name[sizeof(app->cfg.name) - 1] = '\0';
    app->cfg.birth_y = 2024;
    app->cfg.birth_m = 1;
    app->cfg.birth_d = 1;
    app->cfg.language = 0; // English by default

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = buffered_file_stream_alloc(storage);
    
    if(buffered_file_stream_open(stream, CONFIG_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t size = stream_size(stream);
        if(size > 0 && size < 1024) {
            char buffer[1024];
            size_t read = stream_read(stream, (uint8_t*)buffer, size);
            buffer[read] = '\0';
            
            const char* name_pos = find_key(buffer, "name");
            if(name_pos) {
                extract_string(name_pos, app->cfg.name, sizeof(app->cfg.name));
            }
            
            const char* year_pos = find_key(buffer, "birth_y");
            if(year_pos) {
                parse_uint16(year_pos, &app->cfg.birth_y);
            }
            
            const char* month_pos = find_key(buffer, "birth_m");
            if(month_pos) {
                parse_uint8(month_pos, &app->cfg.birth_m);
            }
            
            const char* day_pos = find_key(buffer, "birth_d");
            if(day_pos) {
                parse_uint8(day_pos, &app->cfg.birth_d);
            }
            
            const char* lang_pos = find_key(buffer, "language");
            if(lang_pos) {
                uint8_t lang = 0;
                parse_uint8(lang_pos, &lang);
                if(lang <= 2) { // Valid language range
                    app->cfg.language = lang;
                }
            }
        }
        buffered_file_stream_close(stream);
    }
    
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}

void save_config(AppData* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    
    // Ensure directory exists
    storage_common_mkdir(storage, EXT_PATH("apps/baby_log"));
    
    Stream* stream = buffered_file_stream_alloc(storage);
    
    if(buffered_file_stream_open(stream, CONFIG_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
            "{\n"
            "  \"name\": \"%s\",\n"
            "  \"birth_y\": %u,\n"
            "  \"birth_m\": %u,\n"
            "  \"birth_d\": %u,\n"
            "  \"language\": %u\n"
            "}\n",
            app->cfg.name,
            app->cfg.birth_y,
            app->cfg.birth_m,
            app->cfg.birth_d,
            app->cfg.language);
        stream_write(stream, (uint8_t*)buffer, strlen(buffer));
        buffered_file_stream_close(stream);
    }
    
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}

void load_state(AppData* app) {
    // Default values
    app->state.feeding_active = false;
    app->state.sleep_active = false;
    app->state.feeding_start_ts = 0;
    app->state.sleep_start_ts = 0;
    app->state.last_diaper_ts = 0;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = buffered_file_stream_alloc(storage);
    
    if(buffered_file_stream_open(stream, STATE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t size = stream_size(stream);
        if(size > 0 && size < 512) {
            char buffer[512];
            size_t read = stream_read(stream, (uint8_t*)buffer, size);
            buffer[read] = '\0';
            
            const char* feeding_pos = find_key(buffer, "feeding_active");
            if(feeding_pos) {
                app->state.feeding_active = (*feeding_pos == 't');
            }
            
            const char* sleep_pos = find_key(buffer, "sleep_active");
            if(sleep_pos) {
                app->state.sleep_active = (*sleep_pos == 't');
            }
            
            const char* feeding_ts_pos = find_key(buffer, "feeding_start_ts");
            if(feeding_ts_pos) {
                uint32_t ts = 0;
                parse_int(feeding_ts_pos, (int*)&ts);
                app->state.feeding_start_ts = ts;
            }
            
            const char* sleep_ts_pos = find_key(buffer, "sleep_start_ts");
            if(sleep_ts_pos) {
                uint32_t ts = 0;
                parse_int(sleep_ts_pos, (int*)&ts);
                app->state.sleep_start_ts = ts;
            }
        }
        buffered_file_stream_close(stream);
    }
    
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}

void save_state(AppData* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    
    // Ensure directory exists
    storage_common_mkdir(storage, EXT_PATH("apps/baby_log"));
    
    Stream* stream = buffered_file_stream_alloc(storage);
    
    if(buffered_file_stream_open(stream, STATE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
            "{\n"
            "  \"feeding_active\": %s,\n"
            "  \"sleep_active\": %s,\n"
            "  \"feeding_start_ts\": %lu,\n"
            "  \"sleep_start_ts\": %lu\n"
            "}\n",
            app->state.feeding_active ? "true" : "false",
            app->state.sleep_active ? "true" : "false",
            (unsigned long)app->state.feeding_start_ts,
            (unsigned long)app->state.sleep_start_ts);
        stream_write(stream, (uint8_t*)buffer, strlen(buffer));
        buffered_file_stream_close(stream);
    }
    
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}
