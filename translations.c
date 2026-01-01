#include "translations.h"
#include <string.h>

// English translations
static const char* tr_en[TR_COUNT] = {
    [TR_MAIN_AWAKE] = "Awake",
    [TR_MAIN_FEEDING] = "Feeding",
    [TR_MAIN_SLEEPING] = "Sleeping",
    [TR_MAIN_DIAPER_LOGGED] = "Diaper logged!",
    [TR_MAIN_UP_FEED] = "^ Feed",
    [TR_MAIN_DOWN_SLEEP] = "v Sleep",
    [TR_MAIN_LEFT_DIAPER] = "< Diaper",
    [TR_MAIN_BACK_MENU] = "B Menu",
    [TR_MAIN_FEED_LABEL] = "Feed",
    [TR_MAIN_SLEEP_LABEL] = "Sleep",
    [TR_MAIN_DIAPER_LABEL] = "Diaper",
    [TR_MAIN_MENU_LABEL] = "Menu",
    
    [TR_MENU_PROFILE] = "Child profile",
    [TR_MENU_RECENT_LOG] = "Recent log",
    [TR_MENU_DELETE_LAST] = "Delete last",
    [TR_MENU_CLEAR_ALL] = "Clear all logs",
    [TR_MENU_LANGUAGE] = "Language",
    [TR_MENU_EXIT] = "Exit",
    
    [TR_PROFILE_TITLE] = "Profile",
    [TR_PROFILE_NAME] = "Name:",
    [TR_PROFILE_BIRTH] = "Birth:",
    [TR_PROFILE_AGE] = "Age:",
    [TR_PROFILE_EDIT_NAME] = "Edit name",
    [TR_PROFILE_EDIT_DATE] = "Edit birth date",
    [TR_PROFILE_LANGUAGE] = "Language",
    
    [TR_EDIT_NAME_TITLE] = "Edit Name",
    [TR_EDIT_NAME_NAME] = "Name:",
    [TR_EDIT_NAME_CHAR] = "Char:",
    [TR_EDIT_NAME_UP_DOWN] = "^/v:Char",
    [TR_EDIT_NAME_L_R] = "</>:Move",
    
    [TR_EDIT_DATE_TITLE] = "Edit Birth Date",
    [TR_EDIT_DATE_YEAR] = "Year:",
    [TR_EDIT_DATE_MONTH] = "Month:",
    [TR_EDIT_DATE_DAY] = "Day:",
    [TR_EDIT_DATE_INVALID] = "Invalid!",
    
    [TR_LOG_TITLE] = "Recent Log",
    [TR_LOG_NO_EVENTS] = "No events yet",
    [TR_LOG_DIAPER] = "Diaper",
    [TR_LOG_FEED_START] = "Feed start",
    [TR_LOG_FEED_STOP] = "Feed stop",
    [TR_LOG_SLEEP_START] = "Sleep start",
    [TR_LOG_SLEEP_STOP] = "Sleep stop",
    
    [TR_LANG_TITLE] = "Language",
    [TR_LANG_ENGLISH] = "English",
    [TR_LANG_RUSSIAN] = "Russian",
    [TR_LANG_SPANISH] = "Spanish",
    
    [TR_AGE_DAYS] = "days",
    [TR_AGE_WEEKS] = "weeks",
    [TR_AGE_MONTHS] = "months",
};

// Russian translations (using Latin transliteration - Flipper Zero fonts don't support Cyrillic)
static const char* tr_ru[TR_COUNT] = {
    [TR_MAIN_AWAKE] = "Bodrstvuet",
    [TR_MAIN_FEEDING] = "Kormlenie",
    [TR_MAIN_SLEEPING] = "Spit",
    [TR_MAIN_DIAPER_LOGGED] = "Podguznik!",
    [TR_MAIN_UP_FEED] = "^ Korm",
    [TR_MAIN_DOWN_SLEEP] = "v Son",
    [TR_MAIN_LEFT_DIAPER] = "< Podg",
    [TR_MAIN_BACK_MENU] = "B Menu",
    [TR_MAIN_FEED_LABEL] = "Korm",
    [TR_MAIN_SLEEP_LABEL] = "Son",
    [TR_MAIN_DIAPER_LABEL] = "Podg",
    [TR_MAIN_MENU_LABEL] = "Menu",
    
    [TR_MENU_PROFILE] = "Profil",
    [TR_MENU_RECENT_LOG] = "Zhurnal",
    [TR_MENU_DELETE_LAST] = "Udalit",
    [TR_MENU_CLEAR_ALL] = "Ochistit vse",
    [TR_MENU_LANGUAGE] = "Yazyk",
    [TR_MENU_EXIT] = "Vyhod",
    
    [TR_PROFILE_TITLE] = "Profil",
    [TR_PROFILE_NAME] = "Imya:",
    [TR_PROFILE_BIRTH] = "Rozhdenie:",
    [TR_PROFILE_AGE] = "Vozrast:",
    [TR_PROFILE_EDIT_NAME] = "Izmenit imya",
    [TR_PROFILE_EDIT_DATE] = "Data rozhdeniya",
    [TR_PROFILE_LANGUAGE] = "Yazyk",
    
    [TR_EDIT_NAME_TITLE] = "Izmenit imya",
    [TR_EDIT_NAME_NAME] = "Imya:",
    [TR_EDIT_NAME_CHAR] = "Bukva:",
    [TR_EDIT_NAME_UP_DOWN] = "^/v",
    [TR_EDIT_NAME_L_R] = "</>",
    
    [TR_EDIT_DATE_TITLE] = "Data rozhdeniya",
    [TR_EDIT_DATE_YEAR] = "God:",
    [TR_EDIT_DATE_MONTH] = "Mesyats:",
    [TR_EDIT_DATE_DAY] = "Den:",
    [TR_EDIT_DATE_INVALID] = "Neverno!",
    
    [TR_LOG_TITLE] = "Zhurnal",
    [TR_LOG_NO_EVENTS] = "Net sobytiy",
    [TR_LOG_DIAPER] = "Podguznik",
    [TR_LOG_FEED_START] = "Korm nach",
    [TR_LOG_FEED_STOP] = "Korm kon",
    [TR_LOG_SLEEP_START] = "Son nach",
    [TR_LOG_SLEEP_STOP] = "Son kon",
    
    [TR_LANG_TITLE] = "Yazyk",
    [TR_LANG_ENGLISH] = "Angliyskiy",
    [TR_LANG_RUSSIAN] = "Russkiy",
    [TR_LANG_SPANISH] = "Ispanskiy",
    
    [TR_AGE_DAYS] = "dney",
    [TR_AGE_WEEKS] = "nedel",
    [TR_AGE_MONTHS] = "mesyatsev",
};

// Spanish translations
static const char* tr_es[TR_COUNT] = {
    [TR_MAIN_AWAKE] = "Despierto",
    [TR_MAIN_FEEDING] = "Alimentando",
    [TR_MAIN_SLEEPING] = "Durmiendo",
    [TR_MAIN_DIAPER_LOGGED] = "Pañal!",
    [TR_MAIN_UP_FEED] = "^ Comer",
    [TR_MAIN_DOWN_SLEEP] = "v Dormir",
    [TR_MAIN_LEFT_DIAPER] = "< Panal",
    [TR_MAIN_BACK_MENU] = "B Menu",
    [TR_MAIN_FEED_LABEL] = "Comer",
    [TR_MAIN_SLEEP_LABEL] = "Dormir",
    [TR_MAIN_DIAPER_LABEL] = "Panal",
    [TR_MAIN_MENU_LABEL] = "Menu",
    
    [TR_MENU_PROFILE] = "Perfil",
    [TR_MENU_RECENT_LOG] = "Registro",
    [TR_MENU_DELETE_LAST] = "Borrar ultimo",
    [TR_MENU_CLEAR_ALL] = "Borrar todo",
    [TR_MENU_LANGUAGE] = "Idioma",
    [TR_MENU_EXIT] = "Salir",
    
    [TR_PROFILE_TITLE] = "Perfil",
    [TR_PROFILE_NAME] = "Nombre:",
    [TR_PROFILE_BIRTH] = "Nacimiento:",
    [TR_PROFILE_AGE] = "Edad:",
    [TR_PROFILE_EDIT_NAME] = "Editar nombre",
    [TR_PROFILE_EDIT_DATE] = "Fecha nacimiento",
    [TR_PROFILE_LANGUAGE] = "Idioma",
    
    [TR_EDIT_NAME_TITLE] = "Editar Nombre",
    [TR_EDIT_NAME_NAME] = "Nombre:",
    [TR_EDIT_NAME_CHAR] = "Letra:",
    [TR_EDIT_NAME_UP_DOWN] = "^/v",
    [TR_EDIT_NAME_L_R] = "</>",
    
    [TR_EDIT_DATE_TITLE] = "Fecha Nacimiento",
    [TR_EDIT_DATE_YEAR] = "Año:",
    [TR_EDIT_DATE_MONTH] = "Mes:",
    [TR_EDIT_DATE_DAY] = "Día:",
    [TR_EDIT_DATE_INVALID] = "Inválido!",
    
    [TR_LOG_TITLE] = "Registro",
    [TR_LOG_NO_EVENTS] = "Sin eventos",
    [TR_LOG_DIAPER] = "Pañal",
    [TR_LOG_FEED_START] = "Comer inicio",
    [TR_LOG_FEED_STOP] = "Comer fin",
    [TR_LOG_SLEEP_START] = "Dormir inicio",
    [TR_LOG_SLEEP_STOP] = "Dormir fin",
    
    [TR_LANG_TITLE] = "Idioma",
    [TR_LANG_ENGLISH] = "Inglés",
    [TR_LANG_RUSSIAN] = "Ruso",
    [TR_LANG_SPANISH] = "Español",
    
    [TR_AGE_DAYS] = "días",
    [TR_AGE_WEEKS] = "semanas",
    [TR_AGE_MONTHS] = "meses",
};

const char* tr_get(AppData* app, TranslationKey key) {
    if(!app || key >= TR_COUNT) {
        return "";
    }
    
    Language lang = (Language)app->cfg.language;
    const char** table = tr_en; // Default to English
    
    switch(lang) {
        case LangRussian:
            table = tr_ru;
            break;
        case LangSpanish:
            table = tr_es;
            break;
        case LangEnglish:
        default:
            table = tr_en;
            break;
    }
    
    return table[key] ? table[key] : "";
}

