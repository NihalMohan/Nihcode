#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cJSON.c"
#include "cJSON.h"

#define DEV_LIM 100
#define MAX_UUID_LENGTH 37
#define MAX_BASE64_LENGTH 5

typedef struct {
    char name[50];
    char type[20];
    char info[100];
    char value[MAX_BASE64_LENGTH];
    long timestamp;
    char uuid[MAX_UUID_LENGTH];
} Device;

int main() {
    FILE* file;
    char data[5000];
    size_t dataSize;
    Device devices[DEV_LIM];
    int numDevices = 0;
    int i;
    long currentTimestamp = time(NULL);

    // Read the JSON data from the file
    file = fopen_s("data.json", "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }

    dataSize = fread(data, 1, sizeof(data), file);
    fclose(file);

    // Parse the JSON data
    cJSON* json = cJSON_Parse(data);
    cJSON* devicesJson = cJSON_GetObjectItem(json, "Devices");
    if (devicesJson == NULL) {
        printf("Invalid JSON format: 'Devices' array not found.\n");
        return 1;
    }

    // Iterate over the devices and parse the necessary information
    cJSON* deviceJson;
    cJSON_ArrayForEach(deviceJson, devicesJson) {
        if (numDevices >= DEV_LIM) {
            printf("Maximum number of devices reached.\n");
            break;
        }

        cJSON* nameJson = cJSON_GetObjectItem(deviceJson, "Name");
        cJSON* typeJson = cJSON_GetObjectItem(deviceJson, "Type");
        cJSON* infoJson = cJSON_GetObjectItem(deviceJson, "Info");
        cJSON* valueJson = cJSON_GetObjectItem(deviceJson, "value");
        cJSON* timestampJson = cJSON_GetObjectItem(deviceJson, "timestamp");

        if (nameJson == NULL || typeJson == NULL || infoJson == NULL ||
            valueJson == NULL || timestampJson == NULL) {
            printf("Invalid JSON format: Missing device information.\n");
            continue;
        }

        strncpy(devices[numDevices].name, nameJson->valuestring, sizeof(devices[numDevices].name));
        strncpy(devices[numDevices].type, typeJson->valuestring, sizeof(devices[numDevices].type));
        strncpy(devices[numDevices].info, infoJson->valuestring, sizeof(devices[numDevices].info));
        strncpy(devices[numDevices].value, valueJson->valuestring, sizeof(devices[numDevices].value));
        devices[numDevices].timestamp = atol(timestampJson->valuestring);

        // Extract the uuid from the info field
        const char* uuidStart = strstr(devices[numDevices].info, "uuid:");
        if (uuidStart != NULL) {
            strncpy(devices[numDevices].uuid, uuidStart + 5, MAX_UUID_LENGTH);
        }
        else {
            printf("Invalid JSON format: 'uuid' not found in 'Info' field.\n");
            continue;
        }

        // Discard devices with timestamps before the current time
        if (devices[numDevices].timestamp < currentTimestamp) {
            numDevices++;
        }
    }

    // Calculate the total of all value entries
    int total = 0;
    for (i = 0; i < numDevices; i++) {
        total += atoi(devices[i].value);
    }

    // Output the values total and the list of uuids
    cJSON* outputJson = cJSON_CreateObject();
    cJSON_AddItemToObject(outputJson, "Total", cJSON_CreateNumber(total));
    cJSON* uuidsArrayJson = cJSON_CreateArray();
    for (i = 0; i < numDevices; i++) {
        cJSON_AddItemToArray(uuidsArrayJson, cJSON_CreateString(devices[i].uuid));
    }
    cJSON_AddItemToObject(outputJson, "UUIDs", uuidsArrayJson);

    // Write the data to a file
    file = fopen_s("output.json", "w");
    if (file == NULL) {
        printf("Failed to create output file.\n");
        return 1;
    }
    char* outputString = cJSON_Print(outputJson);
    fprintf(file, "%s", outputString);
    fclose(file);

    // Cleanup
    cJSON_Delete(json);
    free(outputString);

    printf("Output file 'output.json' created successfully.\n");

    return 0;
}

