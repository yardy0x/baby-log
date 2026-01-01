#pragma once
#include <furi.h>

#define MAX_NAME_LEN 12
#define MAX_LOG_ENTRIES 10

typedef struct {
    char name[16];
    uint16_t birth_y;
    uint8_t birth_m;
    uint8_t birth_d;
    uint8_t language; // Language enum value
} BabyConfig;

typedef struct {
    bool feeding_active;
    bool sleep_active;
    uint32_t feeding_start_ts;
    uint32_t sleep_start_ts;
    uint32_t last_diaper_ts; // Timestamp of last diaper log (for visual feedback)
} AppState;

typedef enum {
    LogEventDiaper = 0,
    LogEventFeedingStart,
    LogEventFeedingStop,
    LogEventSleepStart,
    LogEventSleepStop,
} LogEventType;

typedef struct {
    uint32_t timestamp;
    LogEventType type;
    uint32_t duration; // For stop events
    char id[33]; // Hex string ID
} LogEntry;

typedef struct {
    BabyConfig cfg;
    AppState state;

    // Menu navigation
    int menu_index;
    
    // Profile screen navigation
    int profile_index; // 0=view, 1=edit name, 2=edit birth date, 3=language
    
    // Language selection screen
    int lang_index; // 0=English, 1=Russian, 2=Spanish
    
    // Edit name screen
    char edit_name[16];
    int edit_name_pos;
    int edit_name_char; // Current character index (0-25 for A-Z)
    
    // Edit birth date screen
    int edit_date_field; // 0=year, 1=month, 2=day
    uint16_t edit_year;
    uint8_t edit_month;
    uint8_t edit_day;
    
    // Recent log screen
    int log_index; // Current scroll position
    LogEntry log_entries[MAX_LOG_ENTRIES];
    int log_count;
} AppData;

void load_config(AppData* app);
void save_config(AppData* app);
void load_state(AppData* app);
void save_state(AppData* app);
