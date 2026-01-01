#pragma once
#include "app_state.h"

typedef enum {
    LangEnglish = 0,
    LangRussian,
    LangSpanish,
} Language;

// Translation keys
typedef enum {
    // Main screen
    TR_MAIN_AWAKE,
    TR_MAIN_FEEDING,
    TR_MAIN_SLEEPING,
    TR_MAIN_DIAPER_LOGGED,
    TR_MAIN_UP_FEED,
    TR_MAIN_DOWN_SLEEP,
    TR_MAIN_LEFT_DIAPER,
    TR_MAIN_BACK_MENU,
    TR_MAIN_FEED_LABEL,    // Label without arrow
    TR_MAIN_SLEEP_LABEL,   // Label without arrow
    TR_MAIN_DIAPER_LABEL,  // Label without arrow
    TR_MAIN_MENU_LABEL,    // Label without arrow
    
    // Menu
    TR_MENU_PROFILE,
    TR_MENU_RECENT_LOG,
    TR_MENU_DELETE_LAST,
    TR_MENU_CLEAR_ALL,
    TR_MENU_LANGUAGE,
    TR_MENU_EXIT,
    
    // Profile
    TR_PROFILE_TITLE,
    TR_PROFILE_NAME,
    TR_PROFILE_BIRTH,
    TR_PROFILE_AGE,
    TR_PROFILE_EDIT_NAME,
    TR_PROFILE_EDIT_DATE,
    TR_PROFILE_LANGUAGE,
    
    // Edit name
    TR_EDIT_NAME_TITLE,
    TR_EDIT_NAME_NAME,
    TR_EDIT_NAME_CHAR,
    TR_EDIT_NAME_UP_DOWN,
    TR_EDIT_NAME_L_R,
    
    // Edit birth date
    TR_EDIT_DATE_TITLE,
    TR_EDIT_DATE_YEAR,
    TR_EDIT_DATE_MONTH,
    TR_EDIT_DATE_DAY,
    TR_EDIT_DATE_INVALID,
    
    // Recent log
    TR_LOG_TITLE,
    TR_LOG_NO_EVENTS,
    TR_LOG_DIAPER,
    TR_LOG_FEED_START,
    TR_LOG_FEED_STOP,
    TR_LOG_SLEEP_START,
    TR_LOG_SLEEP_STOP,
    
    // Language
    TR_LANG_TITLE,
    TR_LANG_ENGLISH,
    TR_LANG_RUSSIAN,
    TR_LANG_SPANISH,
    
    // Age units
    TR_AGE_DAYS,
    TR_AGE_WEEKS,
    TR_AGE_MONTHS,
    
    TR_COUNT // Total count
} TranslationKey;

const char* tr_get(AppData* app, TranslationKey key);

