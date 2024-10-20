
/* 
Adapted code based on response from VS Code Copilot 20241020.
Prompt "Give a code example for esphome to parse obis
	output code into key value unit and name the obis codes correctly"
*/

#define MAX_OBIS_CODES 20
#define MAX_OBIS_LENGTH 20
#define MAX_NAME_LENGTH 50
#define MAX_UNIT_LENGTH 10
#define MAX_VALUE_LENGTH 50

typedef struct {
    char obis_code[MAX_OBIS_LENGTH];
    char name[MAX_NAME_LENGTH];
    char value[MAX_VALUE_LENGTH];
    char unit[MAX_UNIT_LENGTH];
} OBISData;

// Define a dictionary to map OBIS codes to their names
typedef struct {
    char obis_code[MAX_OBIS_LENGTH];
    char name[MAX_NAME_LENGTH];
} OBISDictionary;

OBISDictionary obis_dict[] = {
        {"6.8", "Cumulative Energy (MWh)"},
        {"6.26", "Cumulative Volume (m3)"},
        {"9.21", "Serial Number"},
        {"6.4", "Current Power (kW)"},
        {"6.27", "Flow Rate (m3ph)"},
        {"6.29", "Temperature C (Flow)"},
        {"6.28", "Temperature C (Return)"},
        {"6.30", "Temperature C (Difference)"},
        {"6.26*01", "Volume (Tariff 1)"},
        {"6.8*01", "Energy (Tariff 1)"},
        {"F", "Firmware Version"},
        {"9.20", "Serial Number"},
        {"6.35", "Flow Rate (m)"},
        {"6.6", "Power max (kW)"},
        {"6.6*01", "Power max (Tariff 1)"},
        {"6.33", "Flow Rate (m3ph)"},
        {"9.4", "Temperature (C)"},
        {"6.31", "Operating Hours (h)"},
        {"6.32", "Downtime (h)"},
        {"9.22", "Status"},
        {"9.6", "Extended Status"},
        {"9.7", "Error Code"},
        {"6.32*01", "Downtime (Tariff 1)"},
        {"6.36", "Date and Time"},
        {"6.33*01", "Flow Rate (Tariff 1)"},
        {"6.8.1", "Energy Phase 1"},
        {"6.8.2", "Energy Phase 2"},
        {"6.8.3", "Energy Phase 3"},
        {"6.8.4", "Energy Phase 4"},
        {"6.8.5", "Energy Phase 5"},
        {"6.8.1*01", "Energy Phase 1 (Tariff 1)"},
        {"6.8.2*01", "Energy Phase 2 (Tariff 1)"},
        {"6.8.3*01", "Energy Phase 3 (Tariff 1)"},
        {"6.8.4*01", "Energy Phase 4 (Tariff 1)"},
        {"6.8.5*01", "Energy Phase 5 (Tariff 1)"},
    };

const char* get_obis_name(const char* obis_code) {
    for (int i = 0; i < sizeof(obis_dict) / sizeof(obis_dict[0]); i++) {
        if (strcmp(obis_dict[i].obis_code, obis_code) == 0) {
            return obis_dict[i].name;
        }
    }
    return "Unknown";
}

void parse_obis(const char* obis_string, OBISData* parsed_data, int* parsed_count) {
    const char* ptr = obis_string;
    *parsed_count = 0;

    while (*ptr != '\0' && *parsed_count < MAX_OBIS_CODES) {
        OBISData* data = &parsed_data[*parsed_count];
	while (*ptr == 0xa || *ptr == 0x0d) ptr++; // Skip newlines
        const char* obis_code_end = strchr(ptr, '(');
        if (obis_code_end == NULL) break;

        strncpy(data->obis_code, ptr, obis_code_end - ptr);
        data->obis_code[obis_code_end - ptr] = '\0';

        const char* value_start = obis_code_end + 1;
        const char* value_end = strchr(value_start, ')');
        if (value_end == NULL) break;

        strncpy(data->value, value_start, value_end - value_start);
        data->value[value_end - value_start] = '\0';

        char* unit_start = strchr(data->value, '*');
        if (unit_start != NULL) {
            *unit_start = '\0';
            strncpy(data->unit, unit_start + 1, MAX_UNIT_LENGTH - 1);
        } else {
            data->unit[0] = '\0';
        }

        strncpy(data->name, get_obis_name(data->obis_code), MAX_NAME_LENGTH - 1);

        ptr = value_end + 1;
        (*parsed_count)++;
    }
}

void print_parsed_data(const OBISData* parsed_data, int parsed_count) {
    for (int i = 0; i < parsed_count; i++) {
        ESP_LOGD("OBIS", "OBIS Code: %s", parsed_data[i].obis_code);
        ESP_LOGD("OBIS", "Name: %s", parsed_data[i].name);
        ESP_LOGD("OBIS", "Value: %s", parsed_data[i].value);
        ESP_LOGD("OBIS", "Unit: %s", parsed_data[i].unit);
        ESP_LOGD("OBIS", "");
    }
}
