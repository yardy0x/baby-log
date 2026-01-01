#pragma once
#include "app_state.h"

uint32_t log_get_timestamp(void);
void log_add_event(AppData* app, LogEventType type, uint32_t duration);
void log_load_recent(AppData* app);
void log_delete_last(AppData* app);
void log_clear_all(AppData* app);

