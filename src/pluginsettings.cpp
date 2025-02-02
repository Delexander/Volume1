#include <rack.hpp>
#include "pluginsettings.hpp"
#include "AuxSources.hpp"


DelexanderVol1Settings pluginSettings;

DelexanderVol1Settings::DelexanderVol1Settings() {
    initDefaults();
}

void DelexanderVol1Settings::initDefaults() {
    // Initialize to Defaults
    auxInputDefaults[0][AuxInputModes::RESET] = true;
    auxInputDefaults[1][AuxInputModes::CLOCK] = true;
    auxInputDefaults[2][AuxInputModes::WILDCARD_MOD] = true;
    auxInputDefaults[3][AuxInputModes::DOUBLE_MORPH] = true;
    auxInputDefaults[4][AuxInputModes::MORPH] = true;
}

void DelexanderVol1Settings::saveToJson() {
    json_t* settingsJ = json_object();

    json_object_set_new(settingsJ, "glowingInkDefault", json_boolean(glowingInkDefault));
    
    json_object_set_new(settingsJ, "vuLightsDefault", json_boolean(vuLightsDefault));

    json_t* allowMultipleModesJ = json_array();
    for (int auxIndex = 0; auxIndex < 5; auxIndex++) {
        json_t* allowanceJ = json_object();
        json_object_set_new(allowanceJ, (std::string("Aux Input ") + std::to_string(auxIndex).c_str() + ": " + "Multimode Allowed").c_str(), json_boolean(allowMultipleModes[auxIndex]));
        json_array_append_new(allowMultipleModesJ, allowanceJ);
    }
    json_object_set_new(settingsJ, "Aux Inputs: Allow Multiple Modes", allowMultipleModesJ);

    for (int auxIndex = 0; auxIndex < 5; auxIndex++) {
        json_t* auxDefaultsJ = json_array();
        for (int mode = 0; mode < AuxInputModes::NUM_MODES; mode++) {
            json_t* auxDefaultJ = json_object();
            json_object_set_new(auxDefaultJ, AuxInputModeLabels[mode].c_str(), json_boolean(auxInputDefaults[auxIndex][mode]));
            json_array_append_new(auxDefaultsJ, auxDefaultJ);
        }
        json_object_set_new(settingsJ, (std::string("Aux Input ") + std::to_string(auxIndex) + " Default Modes").c_str(), auxDefaultsJ);
    }

    std::string settingsFilename = rack::asset::user("DelexanderVol1.json");
    FILE* file = fopen(settingsFilename.c_str(), "w");
    if (file) {
        json_dumpf(settingsJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
        fclose(file);
    }
    json_decref(settingsJ);
}

void DelexanderVol1Settings::readFromJson() {
    std::string settingsFilename = rack::asset::user("DelexanderVol1.json");
    FILE* file = fopen(settingsFilename.c_str(), "r");
    if (!file) {
        initDefaults();
        saveToJson();
        return;
    }

    json_error_t error;
    json_t* settingsJ = json_loadf(file, 0, &error);
    if (!settingsJ) {
        // invalid setting json file
        fclose(file);
        initDefaults();
        saveToJson();
        return;
    }

    json_t* glowingInkDefaultJ = json_object_get(settingsJ, "glowingInkDefault");
    if (glowingInkDefaultJ)
        glowingInkDefault = json_boolean_value(glowingInkDefaultJ);

    json_t* vuLightsDefaultJ = json_object_get(settingsJ, "vuLightsDefault");
    if (vuLightsDefaultJ)
        vuLightsDefault = json_boolean_value(vuLightsDefaultJ);

    json_t* allowMultipleModesJ = json_object_get(settingsJ, "Aux Inputs: Allow Multiple Modes");
    if (allowMultipleModesJ) {
        json_t* allowanceJ; size_t allowanceIndex;
        json_array_foreach(allowMultipleModesJ, allowanceIndex, allowanceJ)
            allowMultipleModes[allowanceIndex] = json_boolean_value(json_object_get(allowanceJ, (std::string("Aux Input ") + std::to_string(allowanceIndex) + ": " + "Multimode Allowed").c_str()));
    }

    for (int auxIndex = 0; auxIndex < 5; auxIndex++) {
        json_t* auxInputDefaultsJ = json_object_get(settingsJ, (std::string("Aux Input ") + std::to_string(auxIndex) + " Default Modes").c_str());
        if (auxInputDefaultsJ) {
            json_t* auxDefaultJ; size_t auxDefaultIndex;
            json_array_foreach(auxInputDefaultsJ, auxDefaultIndex, auxDefaultJ) {
                auxInputDefaults[auxIndex][auxDefaultIndex] = json_boolean_value(json_object_get(auxDefaultJ, AuxInputModeLabels[auxDefaultIndex].c_str()));
            }
        }
    }

    fclose(file);
    json_decref(settingsJ);
}
