#include <furi.h>
#include <gui/gui.h>
#include "app_state.h"
#include "screens.h"
#include <gui/view_port.h>
#include "screen_draw.h"
#include "log_store.h"
#include <input/input.h>
#include <furi_hal_rtc.h>
#include <datetime/datetime.h>

static Screen current_screen = ScreenMain;
static AppData app;
static bool should_exit = false;
static ViewPort* view_port = NULL;

// Menu items for ScreenMenu (6 items: Profile, Recent Log, Delete Last, Clear All, Language, Exit)
#define MENU_ITEM_COUNT 6

// Validate date helper
static bool is_valid_date(uint16_t year, uint8_t month, uint8_t day) {
    if(year < 2000 || year > 2100) return false;
    if(month < 1 || month > 12) return false;
    if(day < 1 || day > 31) return false;
    if(month == 2 && day > 29) return false;
    if((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) return false;
    
    DateTime now;
    furi_hal_rtc_get_datetime(&now);
    if(year > now.year) return false;
    if(year == now.year && month > now.month) return false;
    if(year == now.year && month == now.month && day > now.day) return false;
    return true;
}

static void draw_cb(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    switch(current_screen) {
        case ScreenMain: draw_main(&app, canvas); break;
        case ScreenMenu: draw_menu(&app, canvas); break;
        case ScreenProfile: draw_profile(&app, canvas); break;
        case ScreenEditName: draw_edit_name(&app, canvas); break;
        case ScreenEditBirthDate: draw_edit_birthdate(&app, canvas); break;
        case ScreenRecentLog: draw_recent_log(&app, canvas); break;
        case ScreenLanguage: draw_language(&app, canvas); break;
    }
}

// Handle button input and screen transitions
static void input_cb(InputEvent* event, void* ctx) {
    UNUSED(ctx);
    
    // Only process press events
    if(event->type != InputTypePress) {
        return;
    }

    switch(current_screen) {
        case ScreenMain:
            // Main screen quick actions
            // Feeding and sleeping are mutually exclusive
            if(event->key == InputKeyUp) {
                // Feeding button
                uint32_t now = log_get_timestamp();
                
                if(app.state.feeding_active) {
                    // Stop feeding - go to awake
                    uint32_t duration = (now > app.state.feeding_start_ts) ? 
                        (now - app.state.feeding_start_ts) : 0;
                    log_add_event(&app, LogEventFeedingStop, duration);
                    app.state.feeding_active = false;
                } else {
                    // Start feeding - stop sleep if active
                    if(app.state.sleep_active) {
                        // Stop sleep first and log it
                        uint32_t duration = (now > app.state.sleep_start_ts) ? 
                            (now - app.state.sleep_start_ts) : 0;
                        log_add_event(&app, LogEventSleepStop, duration);
                        app.state.sleep_active = false;
                    }
                    // Start feeding
                    app.state.feeding_start_ts = now;
                    app.state.feeding_active = true;
                    log_add_event(&app, LogEventFeedingStart, 0);
                }
                save_state(&app);
                view_port_update(view_port);
            } else if(event->key == InputKeyDown) {
                // Sleep button
                uint32_t now = log_get_timestamp();
                
                if(app.state.sleep_active) {
                    // Stop sleep - go to awake
                    uint32_t duration = (now > app.state.sleep_start_ts) ? 
                        (now - app.state.sleep_start_ts) : 0;
                    log_add_event(&app, LogEventSleepStop, duration);
                    app.state.sleep_active = false;
                } else {
                    // Start sleep - stop feeding if active
                    if(app.state.feeding_active) {
                        // Stop feeding first and log it
                        uint32_t duration = (now > app.state.feeding_start_ts) ? 
                            (now - app.state.feeding_start_ts) : 0;
                        log_add_event(&app, LogEventFeedingStop, duration);
                        app.state.feeding_active = false;
                    }
                    // Start sleep
                    app.state.sleep_start_ts = now;
                    app.state.sleep_active = true;
                    log_add_event(&app, LogEventSleepStart, 0);
                }
                save_state(&app);
                view_port_update(view_port);
            } else if(event->key == InputKeyLeft) {
                // Log diaper - with visual feedback
                uint32_t now = log_get_timestamp();
                log_add_event(&app, LogEventDiaper, 0);
                app.state.last_diaper_ts = now; // Store timestamp for feedback
                save_state(&app);
                view_port_update(view_port);
            } else if(event->key == InputKeyBack) {
                // Open menu
                current_screen = ScreenMenu;
                app.menu_index = 0;
                view_port_update(view_port);
            }
            break;

        case ScreenMenu:
            // Menu screen navigation (6 items now)
            if(event->key == InputKeyUp) {
                if(app.menu_index > 0) {
                    app.menu_index--;
                } else {
                    app.menu_index = MENU_ITEM_COUNT - 1;
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyDown) {
                app.menu_index = (app.menu_index + 1) % MENU_ITEM_COUNT;
                view_port_update(view_port);
            } else if(event->key == InputKeyOk) {
                if(app.menu_index == 0) {
                    // Child profile
                    current_screen = ScreenProfile;
                    app.profile_index = 0;
                    view_port_update(view_port);
                } else if(app.menu_index == 1) {
                    // Recent log
                    current_screen = ScreenRecentLog;
                    log_load_recent(&app);
                    app.log_index = 0;
                    view_port_update(view_port);
                } else if(app.menu_index == 2) {
                    // Delete last event - ensure logs are loaded first
                    if(app.log_count == 0) {
                        log_load_recent(&app);
                    }
                    log_delete_last(&app);
                    view_port_update(view_port);
                } else if(app.menu_index == 3) {
                    // Clear all logs
                    log_clear_all(&app);
                    view_port_update(view_port);
                } else if(app.menu_index == 4) {
                    // Language selection
                    app.lang_index = app.cfg.language;
                    current_screen = ScreenLanguage;
                    view_port_update(view_port);
                } else if(app.menu_index == 5) {
                    // Exit
                    should_exit = true;
                }
            } else if(event->key == InputKeyBack) {
                current_screen = ScreenMain;
                view_port_update(view_port);
            }
            break;

        case ScreenProfile:
            // Profile screen navigation (2 options - Language moved to main menu)
            if(event->key == InputKeyUp) {
                if(app.profile_index > 0) {
                    app.profile_index--;
                } else {
                    app.profile_index = 1;
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyDown) {
                app.profile_index = (app.profile_index + 1) % 2;
                view_port_update(view_port);
            } else if(event->key == InputKeyOk) {
                if(app.profile_index == 0) {
                    // Edit name
                    strncpy(app.edit_name, app.cfg.name, sizeof(app.edit_name) - 1);
                    app.edit_name[sizeof(app.edit_name) - 1] = '\0';
                    app.edit_name_pos = 0;
                    if(strlen(app.edit_name) > 0) {
                        char c = app.edit_name[0];
                        if(c == ' ') {
                            app.edit_name_char = 26; // Space
                        } else {
                            app.edit_name_char = c - 'A';
                            if(app.edit_name_char < 0 || app.edit_name_char > 25) {
                                app.edit_name_char = 0;
                            }
                        }
                    } else {
                        app.edit_name_char = 0;
                    }
                    current_screen = ScreenEditName;
                    view_port_update(view_port);
                } else if(app.profile_index == 1) {
                    // Edit birth date
                    app.edit_year = app.cfg.birth_y;
                    app.edit_month = app.cfg.birth_m;
                    app.edit_day = app.cfg.birth_d;
                    app.edit_date_field = 0;
                    current_screen = ScreenEditBirthDate;
                    view_port_update(view_port);
                }
            } else if(event->key == InputKeyBack) {
                current_screen = ScreenMenu;
                view_port_update(view_port);
            }
            break;

        case ScreenEditName:
            // Edit name screen
            // Character set: A-Z (0-25) + Space (26)
            if(event->key == InputKeyUp) {
                // Increase character (A-Z then Space)
                app.edit_name_char = (app.edit_name_char + 1) % 27;
                if(app.edit_name_pos < MAX_NAME_LEN) {
                    if(app.edit_name_char == 26) {
                        app.edit_name[app.edit_name_pos] = ' ';
                    } else {
                        app.edit_name[app.edit_name_pos] = 'A' + app.edit_name_char;
                    }
                    if((size_t)app.edit_name_pos >= strlen(app.edit_name)) {
                        app.edit_name[app.edit_name_pos + 1] = '\0';
                    }
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyDown) {
                // Decrease character (Space then Z-A)
                app.edit_name_char = (app.edit_name_char + 26) % 27;
                if(app.edit_name_pos < MAX_NAME_LEN) {
                    if(app.edit_name_char == 26) {
                        app.edit_name[app.edit_name_pos] = ' ';
                    } else {
                        app.edit_name[app.edit_name_pos] = 'A' + app.edit_name_char;
                    }
                    if((size_t)app.edit_name_pos >= strlen(app.edit_name)) {
                        app.edit_name[app.edit_name_pos + 1] = '\0';
                    }
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyLeft) {
                // Move cursor left or delete if at end
                size_t name_len = strlen(app.edit_name);
                if((size_t)app.edit_name_pos >= name_len && name_len > 0) {
                    // Delete last character
                    app.edit_name[name_len - 1] = '\0';
                    if(app.edit_name_pos > 0) {
                        app.edit_name_pos--;
                    }
                    if((size_t)app.edit_name_pos < strlen(app.edit_name)) {
                        char c = app.edit_name[app.edit_name_pos];
                        if(c == ' ') {
                            app.edit_name_char = 26;
                        } else {
                            app.edit_name_char = c - 'A';
                            if(app.edit_name_char < 0 || app.edit_name_char > 25) {
                                app.edit_name_char = 0;
                            }
                        }
                    } else {
                        app.edit_name_char = 0;
                    }
                } else if(app.edit_name_pos > 0) {
                    // Move cursor left
                    app.edit_name_pos--;
                    if((size_t)app.edit_name_pos < name_len) {
                        char c = app.edit_name[app.edit_name_pos];
                        if(c == ' ') {
                            app.edit_name_char = 26;
                        } else {
                            app.edit_name_char = c - 'A';
                            if(app.edit_name_char < 0 || app.edit_name_char > 25) {
                                app.edit_name_char = 0;
                            }
                        }
                    } else {
                        app.edit_name_char = 0;
                    }
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyRight) {
                // Move cursor right
                if(app.edit_name_pos < MAX_NAME_LEN - 1) {
                    app.edit_name_pos++;
                    size_t name_len = strlen(app.edit_name);
                    if((size_t)app.edit_name_pos < name_len) {
                        char c = app.edit_name[app.edit_name_pos];
                        if(c == ' ') {
                            app.edit_name_char = 26;
                        } else {
                            app.edit_name_char = c - 'A';
                            if(app.edit_name_char < 0 || app.edit_name_char > 25) {
                                app.edit_name_char = 0;
                            }
                        }
                    } else {
                        app.edit_name_char = 0;
                    }
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyOk) {
                // Confirm and save
                strncpy(app.cfg.name, app.edit_name, sizeof(app.cfg.name) - 1);
                app.cfg.name[sizeof(app.cfg.name) - 1] = '\0';
                save_config(&app);
                current_screen = ScreenProfile;
                view_port_update(view_port);
            } else if(event->key == InputKeyBack) {
                // Delete current character if at end, otherwise save and return
                size_t name_len = strlen(app.edit_name);
                if((size_t)app.edit_name_pos >= name_len && name_len > 0) {
                    // Delete last character
                    app.edit_name[name_len - 1] = '\0';
                    if(app.edit_name_pos > 0) {
                        app.edit_name_pos--;
                    }
                    if((size_t)app.edit_name_pos < strlen(app.edit_name)) {
                        char c = app.edit_name[app.edit_name_pos];
                        if(c == ' ') {
                            app.edit_name_char = 26;
                        } else {
                            app.edit_name_char = c - 'A';
                            if(app.edit_name_char < 0 || app.edit_name_char > 25) {
                                app.edit_name_char = 0;
                            }
                        }
                    } else {
                        app.edit_name_char = 0;
                    }
                    view_port_update(view_port);
                } else {
                    // Save and return
                    strncpy(app.cfg.name, app.edit_name, sizeof(app.cfg.name) - 1);
                    app.cfg.name[sizeof(app.cfg.name) - 1] = '\0';
                    save_config(&app);
                    current_screen = ScreenProfile;
                    view_port_update(view_port);
                }
            }
            break;

        case ScreenEditBirthDate:
            // Edit birth date screen
            if(event->key == InputKeyUp) {
                // Increase current field value
                if(app.edit_date_field == 0) {
                    // Year
                    app.edit_year++;
                    if(app.edit_year > 2100) app.edit_year = 2000;
                } else if(app.edit_date_field == 1) {
                    // Month
                    app.edit_month++;
                    if(app.edit_month > 12) app.edit_month = 1;
                } else if(app.edit_date_field == 2) {
                    // Day
                    app.edit_day++;
                    if(app.edit_day > 31) app.edit_day = 1;
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyDown) {
                // Decrease current field value
                if(app.edit_date_field == 0) {
                    // Year
                    if(app.edit_year > 2000) {
                        app.edit_year--;
                    } else {
                        app.edit_year = 2100;
                    }
                } else if(app.edit_date_field == 1) {
                    // Month
                    if(app.edit_month > 1) {
                        app.edit_month--;
                    } else {
                        app.edit_month = 12;
                    }
                } else if(app.edit_date_field == 2) {
                    // Day
                    if(app.edit_day > 1) {
                        app.edit_day--;
                    } else {
                        app.edit_day = 31;
                    }
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyOk) {
                // Move to next field
                app.edit_date_field = (app.edit_date_field + 1) % 3;
                view_port_update(view_port);
            } else if(event->key == InputKeyBack) {
                // Save if valid
                if(is_valid_date(app.edit_year, app.edit_month, app.edit_day)) {
                    app.cfg.birth_y = app.edit_year;
                    app.cfg.birth_m = app.edit_month;
                    app.cfg.birth_d = app.edit_day;
                    save_config(&app);
                }
                current_screen = ScreenProfile;
                view_port_update(view_port);
            }
            break;

        case ScreenRecentLog:
            // Recent log screen
            // Validate log_count and log_index before use
            if(app.log_count < 0 || app.log_count > MAX_LOG_ENTRIES) {
                app.log_count = 0;
            }
            if(app.log_index < 0) app.log_index = 0;
            if(app.log_index >= app.log_count && app.log_count > 0) {
                app.log_index = app.log_count - 1;
            }
            
            if(event->key == InputKeyUp) {
                // Scroll up
                if(app.log_index > 0) {
                    app.log_index--;
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyDown) {
                // Scroll down - show 4 items, so max index is count - 4
                int max_idx = (app.log_count > 4) ? app.log_count - 4 : 0;
                if(app.log_index < max_idx) {
                    app.log_index++;
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyBack) {
                current_screen = ScreenMenu;
                view_port_update(view_port);
            }
            break;

        case ScreenLanguage:
            // Language selection screen
            if(event->key == InputKeyUp) {
                if(app.lang_index > 0) {
                    app.lang_index--;
                } else {
                    app.lang_index = 2;
                }
                view_port_update(view_port);
            } else if(event->key == InputKeyDown) {
                app.lang_index = (app.lang_index + 1) % 3;
                view_port_update(view_port);
            } else if(event->key == InputKeyOk) {
                // Save language selection
                app.cfg.language = app.lang_index;
                save_config(&app);
                current_screen = ScreenProfile;
                view_port_update(view_port);
            } else if(event->key == InputKeyBack) {
                // Cancel - return to profile without saving
                current_screen = ScreenProfile;
                view_port_update(view_port);
            }
            break;
    }
}

int32_t baby_log_app(void* p) {
    UNUSED(p);

    // Initialize app data
    memset(&app, 0, sizeof(app));
    
    load_config(&app);
    load_state(&app);

    // Initialize defaults
    app.menu_index = 0;
    app.profile_index = 0;
    app.log_index = 0;
    app.edit_name_pos = 0;
    app.edit_name_char = 0;
    app.edit_date_field = 0;
    app.lang_index = 0;
    
    // Initialize edit_name from config
    strncpy(app.edit_name, app.cfg.name, sizeof(app.edit_name) - 1);
    app.edit_name[sizeof(app.edit_name) - 1] = '\0';

    Gui* gui = furi_record_open(RECORD_GUI);
    view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_cb, NULL);
    view_port_input_callback_set(view_port, input_cb, view_port);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Periodic update for timers and diaper feedback
    while(!should_exit) {
        furi_delay_ms(1000);
        
        // Clear diaper feedback after 3 seconds
        uint32_t now = log_get_timestamp();
        if(app.state.last_diaper_ts > 0 && 
           now >= app.state.last_diaper_ts && 
           (now - app.state.last_diaper_ts) >= 3) {
            app.state.last_diaper_ts = 0;
        }
        
        // Update display if timers are active or diaper feedback is showing
        if(app.state.feeding_active || app.state.sleep_active || app.state.last_diaper_ts > 0) {
            view_port_update(view_port);
        }
    }

    // Save state before exit
    save_state(&app);

    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
