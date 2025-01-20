
#include "../include/config.h"
#include "../cJSON.h"
#include <stdio.h>


GameConfig loadConfig(const char *filePath) {
    GameConfig config;
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de configuration");
        exit(1);
    }

    char buffer[1024] = {0};
    
    if (fread(buffer, 1, sizeof(buffer) - 1, file) == 0) {
        perror("Erreur lors de la lecture du fichier");
        fclose(file);
        exit(1);
    }
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        printf("Erreur de parsing JSON: %s\n", cJSON_GetErrorPtr());
        exit(1);
    }

    config.volume = cJSON_GetObjectItem(json, "volume")->valueint;
    config.width = cJSON_GetObjectItem(json, "resolution_width")->valueint;
    config.height = cJSON_GetObjectItem(json, "resolution_height")->valueint;
    config.jumpKey = cJSON_GetObjectItem(json, "jumpKey")->valueint;
    config.dashKey = cJSON_GetObjectItem(json, "dashKey")->valueint;
    config.moveLeftKey = cJSON_GetObjectItem(json, "moveLeftKey")->valueint;
    config.moveRightKey = cJSON_GetObjectItem(json, "moveRightKey")->valueint;

    cJSON_Delete(json);
    return config;
}