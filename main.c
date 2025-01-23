#define Rectangle Win_Rectangle
#define CloseWindow Win_CloseWindow
#define ShowCursor Win_ShowCursor
#define LoadImage Win_LoadImage
#define DrawText Win_DrawText
#define DrawTextEx Win_DrawTextEx
#define PlaySound Win_PlaySound

#include <curl/curl.h>
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef LoadImage
#undef DrawText
#undef DrawTextEx
#undef PlaySound

#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <ctype.h>


#define MAX_HEROES 10
#define MAX_UNITS 10
#define MAX_CREATURES 10
#define MAX_RESEARCH 4
#define MAX_LEVELS 3
#define MAX_NAME_LENGTH 100
#define MAX_DESC_LENGTH 200

#define GRID_SIZE 20
#define CELL_SIZE 40
#define MAX_CELL_UNITS 100


#define NUM_UNIT_TYPES 8


typedef struct {
    char name[MAX_NAME_LENGTH];
    char bonus_turu[MAX_NAME_LENGTH];
    int bonus_degeri;
    char aciklama[MAX_DESC_LENGTH];
} Hero;


typedef struct {
    char name[MAX_NAME_LENGTH];
    int saldiri;
    int savunma;
    int saglik;
    int kritik_sans;
    Color color;
} UnitType;


typedef struct {
    char name[MAX_NAME_LENGTH];
    char etki_turu[MAX_NAME_LENGTH];
    int etki_degeri;
    char aciklama[MAX_DESC_LENGTH];
} Creature;


typedef struct {
    int deger;
    char aciklama[MAX_DESC_LENGTH];
} ResearchLevel;


typedef struct {
    char name[MAX_NAME_LENGTH];
    ResearchLevel levels[MAX_LEVELS];
} Research;


typedef struct {
    Hero heroes[MAX_HEROES];
    int hero_count;

    UnitType units[MAX_UNITS];
    int unit_count;

    Creature creatures[MAX_CREATURES];
    int creature_count;

    Research research[MAX_RESEARCH];
    int research_count;
} GameData;


typedef struct {
    UnitType *type;
    int count;
    int total_health;

    int positions[GRID_SIZE * GRID_SIZE][2];
    int units_in_cell[GRID_SIZE * GRID_SIZE];
    int health_in_cell[GRID_SIZE * GRID_SIZE];
    int position_count;
} ArmyUnit;


typedef struct {
    char name[MAX_NAME_LENGTH];
    ArmyUnit units[MAX_UNITS];
    int unit_count;

    Hero heroes[MAX_HEROES];
    int hero_count;

    Creature creatures[MAX_CREATURES];
    int creature_count;

    Research researches[MAX_RESEARCH];
    int research_count;


    int starting_row;
    int starting_col;
    int direction;
} Army;


typedef struct {
    char names[MAX_UNITS][MAX_NAME_LENGTH];
    int count;
} NameList;


typedef struct {
    char name[MAX_NAME_LENGTH];
    struct {
        char name[MAX_NAME_LENGTH];
        int count;
    } units[MAX_UNITS];
    int unit_count;
    NameList heroes;
    NameList monsters;
    struct {
        char type[MAX_NAME_LENGTH];
        int level;
    } researches[MAX_RESEARCH];
    int research_count;
} Empire;


GameData game_data = {0};
FILE *log_file;


struct MemoryStruct {
    char *memory;
    size_t size;
};


static size_t BellekYazmaCallback(void *contents, size_t size, size_t nmemb, void *userp);
char *url_den_json_indir(const char *url);
char* json_dosyasi_oku(const char *file_path);
void kahraman_coz(const char *json_data, const char *hero_name);
void birim_coz(const char *json_data, const char *unit_name);
void yaratik_coz(const char *json_data, const char *creature_name);
void arastirma_coz(const char *json_data, const char *research_name);
void json_satiri_temizle(char *line);
void dizi_isle(char *line, NameList *list);
void json_satir_satir_isle(char *json_data, Empire *human_empire, Empire *orc_legion);
void ordu_baslat(Army *army, Empire *empire, GameData *game_data, int starting_row, int starting_col, int direction);
void savas_simule_et(Army *army1, Army *army2, int *turn);
void saldiri_yap(Army *attacker, Army *defender, int turn);
int toplam_saldiri_gucu_hesapla(Army *attacker, Army *defender, int turn);
int toplam_savunma_gucu_hesapla(Army *defender, Army *attacker, int turn);
int net_hasari_hesapla(int attack_power, int defense_power);
void hasari_dagit(Army *defender, int net_damage);
int kritik_vurus_kontrol_et(Army *army);
double kahraman_bonusu_al(Army *army, const char *unit_name, const char *bonus_type);
double yaratik_bonusu_al(Army *army, const char *bonus_type);
double yaratik_debuff_al(Army *enemy_army, const char *bonus_type);
double arastirma_bonusu_al(Army *army, const char *bonus_type);
void yorgunluk_uygula(Army *army);
double yorgunluk_carpani_al(int turn);
int ordu_yenildi_mi(Army *army);
char *kullanici_secimi_al_ve_indir(void);
void birim_renkleri_ata(GameData *game_data);
void birimleri_izgaraya_dagit(Army *army);
void izgarayi_ciz();
void ordulari_ciz(Army *army1, Army *army2);
void goruntuleri_guncelle(Army *army1, Army *army2, int turn);
void aciklamalari_ciz(GameData *game_data);
void can_barini_ciz(int x, int y, int health_percentage);


static size_t BellekYazmaCallback(void *contents, size_t size, size_t nmemb, void *userp) {

    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Bellek tahsisi basarisiz oldu.\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = '\0';

    return realsize;
}


char *url_den_json_indir(const char *url) {
    // ... (Aynı kalacak)
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  // Baslangicta bos bir hafiza blogu
    chunk.size = 0;

    if (chunk.memory == NULL) {
        fprintf(stderr, "Bellek tahsisi basarisiz oldu.\n");
        return NULL;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (!curl_handle) {
        fprintf(stderr, "CURL baslatilamadi.\n");
        free(chunk.memory);
        return NULL;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, BellekYazmaCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");


    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        fprintf(stderr, "Indirme hatasi: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return NULL;
    }


    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return chunk.memory;
}


char* json_dosyasi_oku(const char *file_path) {

    FILE *file = fopen(file_path, "r");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }
    rewind(file);

    char *json_data = (char *)malloc(file_size + 1);
    if (json_data == NULL) {
        fclose(file);
        return NULL;
    }

    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0';
    fclose(file);

    return json_data;
}


void kahraman_coz(const char *json_data, const char *hero_name) {

    if (game_data.hero_count >= MAX_HEROES) return;

    char *hero_start = strstr(json_data, hero_name);
    if (hero_start != NULL) {
        char *bonus_tur_start = strstr(hero_start, "\"bonus_turu\": ");
        char *bonus_deger_start = strstr(hero_start, "\"bonus_degeri\": ");
        char *aciklama_start = strstr(hero_start, "\"aciklama\": ");

        Hero *hero = &game_data.heroes[game_data.hero_count++];
        strncpy(hero->name, hero_name, MAX_NAME_LENGTH - 1);
        hero->name[MAX_NAME_LENGTH - 1] = '\0'; // Null-terminator ekle

        if (bonus_tur_start) sscanf(bonus_tur_start, "\"bonus_turu\": \"%99[^\"]\"", hero->bonus_turu);
        else strncpy(hero->bonus_turu, "Bilinmiyor", MAX_NAME_LENGTH - 1);

        if (bonus_deger_start) {
            char bonus_deger_str[10];
            sscanf(bonus_deger_start, "\"bonus_degeri\": \"%9[^\"]\"", bonus_deger_str);
            hero->bonus_degeri = atoi(bonus_deger_str);
        } else {
            hero->bonus_degeri = 0;
        }

        if (aciklama_start) sscanf(aciklama_start, "\"aciklama\": \"%199[^\"]\"", hero->aciklama);
        else strncpy(hero->aciklama, "Aciklama bulunamadi", MAX_DESC_LENGTH - 1);
    }
}


void birim_coz(const char *json_data, const char *unit_name) {

    if (game_data.unit_count >= MAX_UNITS) return;

    char *unit_start = strstr(json_data, unit_name);
    if (unit_start != NULL) {
        UnitType *unit = &game_data.units[game_data.unit_count++];
        strncpy(unit->name, unit_name, MAX_NAME_LENGTH - 1);
        unit->name[MAX_NAME_LENGTH - 1] = '\0';

        sscanf(strstr(unit_start, "\"saldiri\": "), "\"saldiri\": %d", &unit->saldiri);
        sscanf(strstr(unit_start, "\"savunma\": "), "\"savunma\": %d", &unit->savunma);
        sscanf(strstr(unit_start, "\"saglik\": "), "\"saglik\": %d", &unit->saglik);
        sscanf(strstr(unit_start, "\"kritik_sans\": "), "\"kritik_sans\": %d", &unit->kritik_sans);
    }
}


void yaratik_coz(const char *json_data, const char *creature_name) {

    if (game_data.creature_count >= MAX_CREATURES) return;

    char *creature_start = strstr(json_data, creature_name);
    if (creature_start != NULL) {
        Creature *creature = &game_data.creatures[game_data.creature_count++];
        strncpy(creature->name, creature_name, MAX_NAME_LENGTH - 1);
        creature->name[MAX_NAME_LENGTH - 1] = '\0';

        sscanf(strstr(creature_start, "\"etki_turu\": "), "\"etki_turu\": \"%99[^\"]\"", creature->etki_turu);

        char etki_degeri_str[10];
        sscanf(strstr(creature_start, "\"etki_degeri\": "), "\"etki_degeri\": \"%9[^\"]\"", etki_degeri_str);
        creature->etki_degeri = atoi(etki_degeri_str);

        sscanf(strstr(creature_start, "\"aciklama\": "), "\"aciklama\": \"%199[^\"]\"", creature->aciklama);
    }
}


void arastirma_coz(const char *json_data, const char *research_name) {

    if (game_data.research_count >= MAX_RESEARCH) return;

    char *research_start = strstr(json_data, research_name);
    if (research_start != NULL) {
        Research *research = &game_data.research[game_data.research_count++];
        strncpy(research->name, research_name, MAX_NAME_LENGTH - 1);
        research->name[MAX_NAME_LENGTH - 1] = '\0';

        for (int i = 0; i < MAX_LEVELS; i++) {
            char level_key[20];
            sprintf(level_key, "seviye_%d", i + 1);

            char *level_start = strstr(research_start, level_key);
            if (level_start) {
                char deger_str[10];
                sscanf(strstr(level_start, "\"deger\": "), "\"deger\": \"%9[^\"]\"", deger_str);
                research->levels[i].deger = atoi(deger_str);
                sscanf(strstr(level_start, "\"aciklama\": "), "\"aciklama\": \"%199[^\"]\"", research->levels[i].aciklama);
            }
        }
    }
}


void json_satiri_temizle(char *line) {
    char *src = line, *dst = line;
    while (*src) {
        if (*src != '"' && *src != '{' && *src != '}' && *src != '[' && *src != ']') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}


void dizi_isle(char *line, NameList *list) {
    json_satiri_temizle(line);
    const char *start = strchr(line, ':');
    if (start == NULL) return;
    start++;
    char item[200];
    while (sscanf(start, " %199[^,]", item) == 1) {
        for (char *p = item; *p; p++) {
            if (*p == '_') *p = ' ';
        }
        strncpy(list->names[list->count], item, MAX_NAME_LENGTH - 1);
        list->names[list->count][MAX_NAME_LENGTH - 1] = '\0';
        list->count++;
        start = strchr(start, ',');
        if (start) start++;
        else break;
    }
}


void json_satir_satir_isle(char *json_data, Empire *human_empire, Empire *orc_legion) {

    char *line = strtok(json_data, "\n");
    int current_section = 0;
    int current_subsection = 0;

    while (line != NULL) {
        json_satiri_temizle(line);

        if (strstr(line, "insan_imparatorlugu:") != NULL) {
            current_section = 1;
        } else if (strstr(line, "ork_legi:") != NULL) {
            current_section = 2;
        }

        if (strstr(line, "birimler:") != NULL) {
            current_subsection = 1;
        } else if (strstr(line, "kahramanlar:") != NULL) {
            current_subsection = 2;
        } else if (strstr(line, "canavarlar:") != NULL) {
            current_subsection = 3;
        } else if (strstr(line, "arastirma_seviyeleri:") != NULL || strstr(line, "arastirma_seviyesi:") != NULL) {
            current_subsection = 4;
        }

        char key[100] = {0}, value[100] = {0};
        if (sscanf(line, " %99[^:]: %99s", key, value) == 2) {
            if (current_section == 1) {
                if (current_subsection == 1) {
                    strncpy(human_empire->units[human_empire->unit_count].name, key, MAX_NAME_LENGTH - 1);
                    human_empire->units[human_empire->unit_count].name[MAX_NAME_LENGTH - 1] = '\0';
                    human_empire->units[human_empire->unit_count].count = atoi(value);
                    human_empire->unit_count++;
                } else if (current_subsection == 4) {

                    strncpy(human_empire->researches[human_empire->research_count].type, key, MAX_NAME_LENGTH - 1);
                    human_empire->researches[human_empire->research_count].type[MAX_NAME_LENGTH - 1] = '\0';
                    human_empire->researches[human_empire->research_count].level = atoi(value);
                    human_empire->research_count++;
                }
            } else if (current_section == 2) {
                if (current_subsection == 1) {
                    strncpy(orc_legion->units[orc_legion->unit_count].name, key, MAX_NAME_LENGTH - 1);
                    orc_legion->units[orc_legion->unit_count].name[MAX_NAME_LENGTH - 1] = '\0';
                    orc_legion->units[orc_legion->unit_count].count = atoi(value);
                    orc_legion->unit_count++;
                } else if (current_subsection == 4) {

                    strncpy(orc_legion->researches[orc_legion->research_count].type, key, MAX_NAME_LENGTH - 1);
                    orc_legion->researches[orc_legion->research_count].type[MAX_NAME_LENGTH - 1] = '\0';
                    orc_legion->researches[orc_legion->research_count].level = atoi(value);
                    orc_legion->research_count++;
                }
            }
        } else {

            if (current_subsection == 2 || current_subsection == 3) {
                if (current_section == 1) {
                    if (current_subsection == 2) {
                        dizi_isle(line, &human_empire->heroes);
                    } else if (current_subsection == 3) {
                        dizi_isle(line, &human_empire->monsters);
                    }
                } else if (current_section == 2) {
                    if (current_subsection == 2) {
                        dizi_isle(line, &orc_legion->heroes);
                    } else if (current_subsection == 3) {
                        dizi_isle(line, &orc_legion->monsters);
                    }
                }
            }
        }

        line = strtok(NULL, "\n");
    }
}


void ordu_baslat(Army *army, Empire *empire, GameData *game_data, int starting_row, int starting_col, int direction) {
    strncpy(army->name, empire->name, MAX_NAME_LENGTH - 1);
    army->name[MAX_NAME_LENGTH - 1] = '\0';
    army->unit_count = 0;


    army->starting_row = starting_row;
    army->starting_col = starting_col;
    army->direction = direction;


    for (int i = 0; i < empire->unit_count; i++) {
        char *unit_name = empire->units[i].name;
        int unit_count = empire->units[i].count;


        UnitType *unit_type = NULL;
        for (int j = 0; j < game_data->unit_count; j++) {
            if (strcmp(unit_name, game_data->units[j].name) == 0) {
                unit_type = &game_data->units[j];
                break;
            }
        }

        if (unit_type == NULL) {
            printf("Birim turu bulunamadi: %s\n", unit_name);
            continue;
        }

        army->units[army->unit_count].type = unit_type;
        army->units[army->unit_count].count = unit_count;
        army->units[army->unit_count].total_health = unit_type->saglik * unit_count;
        army->units[army->unit_count].position_count = 0;
        army->unit_count++;
    }


    army->hero_count = 0;
    for (int i = 0; i < empire->heroes.count; i++) {
        char *hero_name = empire->heroes.names[i];


        for (int j = 0; j < game_data->hero_count; j++) {
            if (strcmp(hero_name, game_data->heroes[j].name) == 0) {
                army->heroes[army->hero_count++] = game_data->heroes[j];
                break;
            }
        }
    }


    army->creature_count = 0;
    for (int i = 0; i < empire->monsters.count; i++) {
        char *creature_name = empire->monsters.names[i];


        for (int j = 0; j < game_data->creature_count; j++) {
            if (strcmp(creature_name, game_data->creatures[j].name) == 0) {
                army->creatures[army->creature_count++] = game_data->creatures[j];
                break;
            }
        }
    }


    army->research_count = 0;
    for (int i = 0; i < empire->research_count; i++) {
        char *research_name = empire->researches[i].type;
        int research_level = empire->researches[i].level;


        for (int j = 0; j < game_data->research_count; j++) {
            if (strcmp(research_name, game_data->research[j].name) == 0) {
                army->researches[army->research_count] = game_data->research[j];

                for (int k = 0; k < research_level; k++) {
                    army->researches[army->research_count].levels[k] = game_data->research[j].levels[k];
                }
                army->research_count++;
                break;
            }
        }
    }


    birimleri_izgaraya_dagit(army);
}


void birim_renkleri_ata(GameData *game_data) {
    Color unit_colors[NUM_UNIT_TYPES] = {
        RED, ORANGE, DARKGREEN, BROWN, DARKPURPLE, SKYBLUE, YELLOW, DARKGRAY
    };

    for (int i = 0; i < game_data->unit_count; i++) {
        game_data->units[i].color = unit_colors[i % NUM_UNIT_TYPES];
    }
}


void birimleri_izgaraya_dagit(Army *army) {
    int row = army->starting_row;
    int col = army->starting_col;
    int dir = army->direction;

    for (int i = 0; i < army->unit_count; i++) {
        ArmyUnit *unit = &army->units[i];
        unit->position_count = 0;

        int units_remaining = unit->count;
        while (units_remaining > 0) {
            int units_in_cell = units_remaining >= MAX_CELL_UNITS ? MAX_CELL_UNITS : units_remaining;
            unit->positions[unit->position_count][0] = row;
            unit->positions[unit->position_count][1] = col;
            unit->units_in_cell[unit->position_count] = units_in_cell;
            unit->health_in_cell[unit->position_count] = units_in_cell * unit->type->saglik;
            unit->position_count++;
            units_remaining -= units_in_cell;


            col += dir;
            if (col >= GRID_SIZE) {
                col = 0;
                row += dir;
                if (row >= GRID_SIZE) row = 0;
            } else if (col < 0) {
                col = GRID_SIZE - 1;
                row += dir;
                if (row < 0) row = GRID_SIZE - 1;
            }
        }
    }
}


int ordu_yenildi_mi(Army *army) {
    for (int i = 0; i < army->unit_count; i++) {
        if (army->units[i].count > 0) {
            return 0;
        }
    }
    return 1;
}

double yorgunluk_carpani_al(int turn) {
    int fatigue_steps = turn / 5;
    double fatigue_multiplier = 1.0 - (fatigue_steps * 0.1);
    if (fatigue_multiplier < 0.5) fatigue_multiplier = 0.5; // Minimum %50
    return fatigue_multiplier;
}

void yorgunluk_uygula(Army *army) {
    for (int i = 0; i < army->unit_count; i++) {
        ArmyUnit *unit = &army->units[i];

        unit->type->saldiri = (int)(unit->type->saldiri * 0.9); // %10 azalt
        unit->type->savunma = (int)(unit->type->savunma * 0.9); // %10 azalt
    }
}

void saldiri_yap(Army *attacker, Army *defender, int turn) {
    printf("\n%s saldiriyor...\n", attacker->name);
    fprintf(log_file, "\n%s saldiriyor...\n", attacker->name);

    int total_attack_power = toplam_saldiri_gucu_hesapla(attacker, defender, turn);
    fprintf(log_file, "Toplam Saldiri Gucu: %d\n", total_attack_power);

    int total_defense_power = toplam_savunma_gucu_hesapla(defender, attacker, turn);
    fprintf(log_file, "Toplam Savunma Gucu: %d\n", total_defense_power);

    int net_damage = net_hasari_hesapla(total_attack_power, total_defense_power);
    fprintf(log_file, "Net Hasar: %d\n", net_damage);

    if (net_damage <= 0) {
        printf("Saldiri etkisiz oldu.\n");
        fprintf(log_file, "Saldiri etkisiz oldu.\n");
        return;
    }

    hasari_dagit(defender, net_damage);

    printf("%s, %s ordusuna %d hasar verdi.\n", attacker->name, defender->name, net_damage);
    fprintf(log_file, "%s, %s ordusuna %d hasar verdi.\n", attacker->name, defender->name, net_damage);
}


void savas_simule_et(Army *army1, Army *army2, int *turn) {
    while (!ordu_yenildi_mi(army1) && !ordu_yenildi_mi(army2)) {
        fprintf(log_file, "\n==== Turn %d ====\n", *turn);


        saldiri_yap(army1, army2, *turn);
        birimleri_izgaraya_dagit(army1);
        birimleri_izgaraya_dagit(army2);

        if (ordu_yenildi_mi(army2)) break;


        saldiri_yap(army2, army1, *turn);
        birimleri_izgaraya_dagit(army1);
        birimleri_izgaraya_dagit(army2);

        if (ordu_yenildi_mi(army1)) break;

        if (*turn % 5 == 0) {
            yorgunluk_uygula(army1);
            yorgunluk_uygula(army2);
            fprintf(log_file, "Yorgunluk etkisi uygulandi.\n");
        }

        (*turn)++;
    }
}

char *kullanici_secimi_al_ve_indir(void) {
    int choice;
    char *json_data = NULL;

    printf("1-10 arasi bir sayi secin: ");
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 10) {
        fprintf(stderr, "Gecersiz giris. 1 ile 10 arasinda bir sayi secin.\n");
        return NULL;
    }

    const char *urls[] = {
        "https://yapbenzet.org.tr/1.json",
        "https://yapbenzet.org.tr/2.json",
        "https://yapbenzet.org.tr/3.json",
        "https://yapbenzet.org.tr/4.json",
        "https://yapbenzet.org.tr/5.json",
        "https://yapbenzet.org.tr/6.json",
        "https://yapbenzet.org.tr/7.json",
        "https://yapbenzet.org.tr/8.json",
        "https://yapbenzet.org.tr/9.json",
        "https://yapbenzet.org.tr/10.json"
    };

    const char *selected_url = urls[choice - 1];
    printf("Secilen URL: %s\n", selected_url);
    json_data = url_den_json_indir(selected_url);

    if (json_data != NULL) {
        printf("Veri basariyla indirildi.\n");
        return json_data;
    } else {
        fprintf(stderr, "Veri indirilemedi. Lutfen tekrar deneyin.\n");
        return NULL;
    }
}


int main() {

    const char *file_path_heroes = "heroes.json";
    const char *file_path_units = "unit_types.json";
    const char *file_path_creatures = "creatures.json";
    const char *file_path_research = "research.json";


    char *json_heroes = json_dosyasi_oku(file_path_heroes);
    if (json_heroes) {
        kahraman_coz(json_heroes, "Alparslan");
        kahraman_coz(json_heroes, "Fatih_Sultan_Mehmet");
        kahraman_coz(json_heroes, "Mete_Han");
        kahraman_coz(json_heroes, "Yavuz_Sultan_Selim");
        kahraman_coz(json_heroes, "Tugrul_Bey");
        kahraman_coz(json_heroes, "Goruk_Vahsi");
        kahraman_coz(json_heroes, "Thruk_Kemikkiran");
        kahraman_coz(json_heroes, "Vrog_Kafakiran");
        kahraman_coz(json_heroes, "Ugar_Zalim");
        free(json_heroes);
    } else {
        fprintf(stderr, "heroes.json dosyasi okunamadi.\n");
        return 1;
    }


    char *json_units = json_dosyasi_oku(file_path_units);
    if (json_units) {
        birim_coz(json_units, "piyadeler");
        birim_coz(json_units, "okcular");
        birim_coz(json_units, "suvariler");
        birim_coz(json_units, "kusatma_makineleri");
        birim_coz(json_units, "ork_dovusculeri");
        birim_coz(json_units, "mizrakcilar");
        birim_coz(json_units, "varg_binicileri");
        birim_coz(json_units, "troller");
        free(json_units);
    } else {
        fprintf(stderr, "unit_types.json dosyasi okunamadi.\n");
        return 1;
    }


    char *json_creatures = json_dosyasi_oku(file_path_creatures);
    if (json_creatures) {
        yaratik_coz(json_creatures, "Ejderha");
        yaratik_coz(json_creatures, "Agri_Dagi_Devleri");
        yaratik_coz(json_creatures, "Tepegoz");
        yaratik_coz(json_creatures, "Karakurt");
        yaratik_coz(json_creatures, "Samur");
        yaratik_coz(json_creatures, "Kara_Troll");
        yaratik_coz(json_creatures, "Golge_Kurtlari");
        yaratik_coz(json_creatures, "Camur_Devleri");
        yaratik_coz(json_creatures, "Ates_Iblisi");
        yaratik_coz(json_creatures, "Makrog_Savas_Beyi");
        free(json_creatures);
    } else {
        fprintf(stderr, "creatures.json dosyasi okunamadi.\n");
        return 1;
    }


    char *json_research = json_dosyasi_oku(file_path_research);
    if (json_research) {
        arastirma_coz(json_research, "savunma_ustaligi");
        arastirma_coz(json_research, "saldiri_gelistirmesi");
        arastirma_coz(json_research, "elit_egitim");
        arastirma_coz(json_research, "kusatma_ustaligi");
        free(json_research);
    } else {
        fprintf(stderr, "research.json dosyasi okunamadi.\n");
        return 1;
    }


    birim_renkleri_ata(&game_data);


    Empire human_empire = {0};
    Empire orc_legion = {0};
    char *json_data = kullanici_secimi_al_ve_indir();
    if (json_data == NULL) {
        fprintf(stderr, "JSON verisi indirilemedi.\n");
        return 1;
    }
    json_satir_satir_isle(json_data, &human_empire, &orc_legion);
    free(json_data);


    Army human_army = {0};
    Army orc_army = {0};
    strncpy(human_empire.name, "INSAN IMPARATORLUGU", MAX_NAME_LENGTH - 1);
    human_empire.name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(orc_legion.name, "ORK LEJYONU IMPARATORLUGU", MAX_NAME_LENGTH - 1);
    orc_legion.name[MAX_NAME_LENGTH - 1] = '\0';

    ordu_baslat(&human_army, &human_empire, &game_data, 0, 0, 1); // Start from top-left
    ordu_baslat(&orc_army, &orc_legion, &game_data, GRID_SIZE - 1, GRID_SIZE - 1, -1); // Start from bottom-right


    log_file = fopen("savas_sim.txt", "w");
    if (log_file == NULL) {
        fprintf(stderr, "Log dosyası açılamadı.\n");
        return 1;
    }

    int screenWidth = GRID_SIZE * CELL_SIZE + 300;  // Extra space for legends
    int screenHeight = GRID_SIZE * CELL_SIZE + 100; // Extra space for titles

    InitWindow(screenWidth, screenHeight, "Battle Simulation");

    SetTargetFPS(60);

    int turn = 1;
    int battle_over = 0;
    int positions_updated = 0;


    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        izgarayi_ciz();
        ordulari_ciz(&human_army, &orc_army);
        aciklamalari_ciz(&game_data);


        DrawText("INSAN IMPARATORLUGU", GRID_SIZE * CELL_SIZE / 2 - MeasureText("INSAN IMPARATORLUGU", 20) / 2, 10, 20, BLUE);
        DrawText("ORK LEJYONU IMPARATORLUGU", GRID_SIZE * CELL_SIZE / 2 - MeasureText("ORK LEJYONU IMPARATORLUGU", 20) / 2, screenHeight - 30, 20, RED);


        DrawText("Savas baslamak uzere... Devam etmek icin herhangi bir tusa basin.", 10, screenHeight / 2 - 10, 20, BLACK);

        EndDrawing();


        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || GetKeyPressed() != 0) {
            break;
        }
    }


    savas_simule_et(&human_army, &orc_army, &turn);
    battle_over = 1;


    if (ordu_yenildi_mi(&human_army)) {
        printf("%s yenildi!\n", human_army.name);
        fprintf(log_file, "%s yenildi!\n", human_army.name);
    } else if (ordu_yenildi_mi(&orc_army)) {
        printf("%s yenildi!\n", orc_army.name);
        fprintf(log_file, "%s yenildi!\n", orc_army.name);
    } else {
        printf("Savaş devam ediyor...\n");
        fprintf(log_file, "Savaş devam ediyor...\n");
    }


    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        izgarayi_ciz();
        ordulari_ciz(&human_army, &orc_army);
        aciklamalari_ciz(&game_data);


        DrawText("INSAN IMPARATORLUGU", GRID_SIZE * CELL_SIZE / 2 - MeasureText("INSAN IMPARATORLUGU", 20) / 2, 10, 20, BLUE);
        DrawText("ORK LEJYONU IMPARATORLUGU", GRID_SIZE * CELL_SIZE / 2 - MeasureText("ORK LEJYONU IMPARATORLUGU", 20) / 2, screenHeight - 30, 20, RED);

        if (battle_over) {
            const char *winner = ordu_yenildi_mi(&human_army) ? "ORK LEJYONU KAZANDI!" : "INSAN IMPARATORLUGU KAZANDI!";
            DrawText(winner, screenWidth / 2 - MeasureText(winner, 30) / 2, screenHeight / 2 - 15, 30, BLACK);
            DrawText("Cikmak icin ESC tusuna basin.", screenWidth / 2 - MeasureText("Cikmak icin ESC tusuna basin.", 20) / 2, screenHeight / 2 + 20, 20, DARKGRAY);
        }

        EndDrawing();


    }

    CloseWindow();


    fclose(log_file);

    return 0;
}


void izgarayi_ciz() {
    for (int i = 0; i <= GRID_SIZE; i++) {
        DrawLine(i * CELL_SIZE, 50, i * CELL_SIZE, GRID_SIZE * CELL_SIZE + 50, LIGHTGRAY);
        DrawLine(0, i * CELL_SIZE + 50, GRID_SIZE * CELL_SIZE, i * CELL_SIZE + 50, LIGHTGRAY);
    }
}


void ordulari_ciz(Army *army1, Army *army2) {

    for (int i = 0; i < army1->unit_count; i++) {
        ArmyUnit *unit = &army1->units[i];
        if (unit->count <= 0) continue;
        for (int j = 0; j < unit->position_count; j++) {
            int x = unit->positions[j][1];
            int y = unit->positions[j][0];
            DrawRectangle(x * CELL_SIZE + 1, y * CELL_SIZE + 51, CELL_SIZE - 2, CELL_SIZE - 2, unit->type->color);


            int max_health_in_cell = unit->units_in_cell[j] * unit->type->saglik;
            int health_percentage = (unit->health_in_cell[j] * 100) / max_health_in_cell;
            can_barini_ciz(x * CELL_SIZE, y * CELL_SIZE + 50, health_percentage);


            char count_str[10];
            sprintf(count_str, "B:%d", unit->units_in_cell[j]);
            DrawText(count_str, x * CELL_SIZE + 2, y * CELL_SIZE + 52, 8, BLACK);


            char health_str[10];
            sprintf(health_str, "S:%d", unit->health_in_cell[j]);
            DrawText(health_str, x * CELL_SIZE + 2, y * CELL_SIZE + 60, 8, DARKGREEN);
        }
    }


    for (int i = 0; i < army2->unit_count; i++) {
        ArmyUnit *unit = &army2->units[i];
        if (unit->count <= 0) continue;
        for (int j = 0; j < unit->position_count; j++) {
            int x = unit->positions[j][1];
            int y = unit->positions[j][0];
            DrawRectangle(x * CELL_SIZE + 1, y * CELL_SIZE + 51, CELL_SIZE - 2, CELL_SIZE - 2, unit->type->color);


            int max_health_in_cell = unit->units_in_cell[j] * unit->type->saglik;
            int health_percentage = (unit->health_in_cell[j] * 100) / max_health_in_cell;
            can_barini_ciz(x * CELL_SIZE, y * CELL_SIZE + 50, health_percentage);


            char count_str[10];
            sprintf(count_str, "B:%d", unit->units_in_cell[j]);
            DrawText(count_str, x * CELL_SIZE + 2, y * CELL_SIZE + 52, 8, BLACK);


            char health_str[10];
            sprintf(health_str, "S:%d", unit->health_in_cell[j]);
            DrawText(health_str, x * CELL_SIZE + 2, y * CELL_SIZE + 60, 8, DARKGREEN);
        }
    }
}


void can_barini_ciz(int x, int y, int health_percentage) {
    DrawRectangle(x + 1, y + CELL_SIZE - 5, CELL_SIZE - 2, 4, DARKGRAY);
    DrawRectangle(x + 1, y + CELL_SIZE - 5, (CELL_SIZE - 2) * health_percentage / 100, 4, GREEN);
}


void aciklamalari_ciz(GameData *game_data) {
    int startX = GRID_SIZE * CELL_SIZE + 10;
    int startY = 50;

    DrawText("Birimler:", startX, startY, 20, BLACK);
    startY += 30;

    for (int i = 0; i < game_data->unit_count; i++) {
        UnitType *unit = &game_data->units[i];
        DrawRectangle(startX, startY + i * 30, 20, 20, unit->color);
        DrawText(unit->name, startX + 30, startY + i * 30 + 5, 15, BLACK);
    }
}


int toplam_saldiri_gucu_hesapla(Army *attacker, Army *defender, int turn) {

    int total_attack = 0;

    for (int i = 0; i < attacker->unit_count; i++) {
        ArmyUnit *unit = &attacker->units[i];

        if (unit->count <= 0) continue;

        double attack_power = unit->type->saldiri * unit->count;


        double hero_bonus = kahraman_bonusu_al(attacker, unit->type->name, "saldiri");

        double research_bonus = arastirma_bonusu_al(attacker, "saldiri");

        double creature_bonus = yaratik_bonusu_al(attacker, "saldiri");

        double enemy_creature_debuff = yaratik_debuff_al(defender, "saldiri");


        double fatigue_multiplier = yorgunluk_carpani_al(turn);


        double total_multiplier = (1 + hero_bonus + research_bonus + creature_bonus - enemy_creature_debuff) * fatigue_multiplier;

        attack_power *= total_multiplier;

        total_attack += (int)attack_power;
    }


    if (kritik_vurus_kontrol_et(attacker)) {
        total_attack = (int)(total_attack * 1.5);
        printf("Kritik vurus gerceklesti!\n");
    }

    return total_attack;
}


int toplam_savunma_gucu_hesapla(Army *defender, Army *attacker, int turn) {

    int total_defense = 0;

    for (int i = 0; i < defender->unit_count; i++) {
        ArmyUnit *unit = &defender->units[i];

        if (unit->count <= 0) continue;

        double defense_power = unit->type->savunma * unit->count;


        double hero_bonus = kahraman_bonusu_al(defender, unit->type->name, "savunma");

        double research_bonus = arastirma_bonusu_al(defender, "savunma");

        double creature_bonus = yaratik_bonusu_al(defender, "savunma");

        double enemy_creature_debuff = yaratik_debuff_al(attacker, "savunma");


        double fatigue_multiplier = yorgunluk_carpani_al(turn);


        double total_multiplier = (1 + hero_bonus + research_bonus + creature_bonus - enemy_creature_debuff) * fatigue_multiplier;

        defense_power *= total_multiplier;

        total_defense += (int)defense_power;
    }

    return total_defense;
}


int net_hasari_hesapla(int attack_power, int defense_power) {
    int net_damage = attack_power - defense_power;
    if (net_damage < 0) net_damage = 0;
    return net_damage;
}


void hasari_dagit(Army *defender, int net_damage) {
    int total_defense = 0;
    for (int i = 0; i < defender->unit_count; i++) {
        ArmyUnit *unit = &defender->units[i];
        if (unit->count <= 0) continue;
        total_defense += unit->type->savunma * unit->count;
    }


    for (int i = 0; i < defender->unit_count; i++) {
        ArmyUnit *unit = &defender->units[i];

        if (unit->count <= 0) continue;

        int unit_defense = unit->type->savunma * unit->count;

        double defense_ratio = (double)unit_defense / total_defense;

        int damage_to_unit = (int)(net_damage * defense_ratio);


        int total_unit_health = 0;
        for (int j = 0; j < unit->position_count; j++) {
            total_unit_health += unit->health_in_cell[j];
        }

        if (total_unit_health <= 0) continue;

        for (int j = 0; j < unit->position_count; j++) {
            int cell_health = unit->health_in_cell[j];
            if (cell_health <= 0) continue;

            double cell_health_ratio = (double)cell_health / total_unit_health;
            int damage_to_cell = (int)(damage_to_unit * cell_health_ratio);

            unit->health_in_cell[j] -= damage_to_cell;
            if (unit->health_in_cell[j] < 0) unit->health_in_cell[j] = 0;


            int units_lost = damage_to_cell / unit->type->saglik;
            if (units_lost >= unit->units_in_cell[j]) {
                unit->units_in_cell[j] = 0;
                unit->health_in_cell[j] = 0;
            } else {
                unit->units_in_cell[j] -= units_lost;

                int remaining_health = unit->health_in_cell[j] % unit->type->saglik;
                if (remaining_health == 0 && unit->units_in_cell[j] > 0) {
                    unit->health_in_cell[j] = unit->units_in_cell[j] * unit->type->saglik;
                } else {
                    unit->health_in_cell[j] = unit->units_in_cell[j] * unit->type->saglik + remaining_health;
                }
            }
        }


        unit->total_health = 0;
        unit->count = 0;
        int new_position_count = 0;
        for (int j = 0; j < unit->position_count; j++) {
            if (unit->units_in_cell[j] > 0) {
                unit->total_health += unit->health_in_cell[j];
                unit->count += unit->units_in_cell[j];
                // Birim hala hücrede varsa pozisyonu koru
                unit->positions[new_position_count][0] = unit->positions[j][0];
                unit->positions[new_position_count][1] = unit->positions[j][1];
                unit->units_in_cell[new_position_count] = unit->units_in_cell[j];
                unit->health_in_cell[new_position_count] = unit->health_in_cell[j];
                new_position_count++;
            }
        }
        unit->position_count = new_position_count;
    }
}


int kritik_vurus_kontrol_et(Army *army) {
    static int attack_count = 0;
    attack_count++;

    for (int i = 0; i < army->unit_count; i++) {
        ArmyUnit *unit = &army->units[i];
        if (unit->count <= 0) continue;

        int critical_chance = unit->type->kritik_sans;
        if (critical_chance == 0) continue;
        int attacks_needed_for_critical = 100 / critical_chance;

        if (attack_count % attacks_needed_for_critical == 0) {
            return 1;
        }
    }
    return 0;
}


double kahraman_bonusu_al(Army *army, const char *unit_name, const char *bonus_type) {
    double bonus = 0.0;

    for (int i = 0; i < army->hero_count; i++) {
        Hero *hero = &army->heroes[i];

        if (strcmp(hero->bonus_turu, bonus_type) == 0) {

            if (strstr(hero->aciklama, unit_name) != NULL) {
                bonus += hero->bonus_degeri / 100.0;
            }
        }
    }

    return bonus;
}


double yaratik_bonusu_al(Army *army, const char *bonus_type) {
    double bonus = 0.0;

    for (int i = 0; i < army->creature_count; i++) {
        Creature *creature = &army->creatures[i];

        if (strcmp(creature->etki_turu, bonus_type) == 0) {
            bonus += creature->etki_degeri / 100.0;
        }
    }

    return bonus;
}


double yaratik_debuff_al(Army *enemy_army, const char *bonus_type) {
    double debuff = 0.0;

    for (int i = 0; i < enemy_army->creature_count; i++) {
        Creature *creature = &enemy_army->creatures[i];

        if (strcmp(creature->etki_turu, bonus_type) == 0) {
            debuff += creature->etki_degeri / 100.0;
        }
    }

    return debuff;
}


double arastirma_bonusu_al(Army *army, const char *bonus_type) {
    double bonus = 0.0;

    for (int i = 0; i < army->research_count; i++) {
        Research *research = &army->researches[i];

        for (int j = 0; j < MAX_LEVELS; j++) {
            ResearchLevel *level = &research->levels[j];

            if (level->deger > 0 && strstr(level->aciklama, bonus_type) != NULL) {
                bonus += level->deger / 100.0;
            }
        }
    }

    return bonus;
}