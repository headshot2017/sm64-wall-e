#include "config.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "mod.h"

std::map<std::string, ConfigElement> config = {
    {"skip_sha1_checksum", {"Skips the ROM SHA-1 checksum. This allows using certain ROM hacks such as sound ROM hacks. (0=off, 1=on)", 0}},
    {"use_wasapi_audio", {"Prefer WASAPI audio backend over SDL2. Only works on Windows Vista and later. (0=off, 1=on)", 0}},
    {"rejectbot_music", {
        "Change the SM64 music used for WALL-E's reject-bot charm.\n"
        "# 1: Star Catch Fanfare\n"
        "# 2: Title screen\n"
        "# 3: Bob-omb Battlefield\n"
        "# 4: Inside the Castle Walls\n"
        "# 5: Jolly Roger Bay / Dire Dire Docks\n"
        "# 6: Shifting Sand Land / Lethal Lava Land\n"
        "# 7: Koopa's Theme\n"
        "# 8: Cool Cool Mountain / Snowman's Land\n"
        "# 9: Slider\n"
        "# 10: Big Boo's Haunt\n"
        "# 11: Piranha Plant's Lullaby\n"
        "# 12: Hazy Maze Cave\n"
        "# 13: Star Select\n"
        "# 14: Powerful Mario\n"
        "# 15: Metal Mario\n"
        "# 16: Koopa's Message\n"
        "# 17: Koopa's Road\n"
        "# 18: High Score\n"
        "# 19: Merry-Go-Round\n"
        "# 20: Race Fanfare\n"
        "# 21: Power Star Appears\n"
        "# 22: Stage Boss\n"
        "# 23: Koopa Clear\n"
        "# 24: Endless Stairs\n"
        "# 25: Ultimate Koopa\n"
        "# 26: Credits\n"
        "# 27: Correct Solution\n"
        "# 28: Toad's Message\n"
        "# 29: Peach's Message\n"
        "# 30: Opening Cutscene\n"
        "# 31: Ultimate Koopa Clear\n"
        "# 32: Ending Cutscene\n"
        "# 33: File Select\n"
        "# 34: Lakitu's Message",
        19}
    },
    {"sm64_music_variation", {
        "Plays a variation of the SM64 music chosen above, if it exists. (0=off, 1=on)\n"
        "# For Title Screen, it plays the Game Over music.\n"
        "# For Jolly Roger Bay, it plays the same music but with less instruments.",
        0}
    },
    //{"autospawn_mario_on_start", {"When loading a game, automatically spawn Mario.", 0}},
    //{"mario_in_cutscenes", {"Replaces player with Mario in the cutscenes.", 0}},
};


int getConfig(std::string value)
{
    return config.count(value) ? config[value].value : 0;
}

void saveConfig()
{
    std::ofstream configfile(CONFIG_FILENAME);
    for (auto& mapkey : config)
    {
        configfile << "# " << mapkey.second.desc << "\n";
        configfile << mapkey.first << ": " << mapkey.second.value << "\n\n";
    }

    std::cout << CONFIG_FILENAME << " saved\n";
}

void loadConfig()
{
    std::ifstream configfile(CONFIG_FILENAME);
    if (!configfile.is_open())
    {
        std::cout << CONFIG_FILENAME << " not found, creating...\n";
        saveConfig(); // saves default config
        return;
    }

    size_t totalCfg = 0; // if less than config.size(), save file to create new keys

    std::string line;
    while (getline(configfile, line))
    {
        // clear any whitespaces
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

        if (line[0] == '#' || line.empty())
            continue; // ignore comments and empty lines

        int delimiterPos = line.find(":");
        if (delimiterPos == -1)
            continue; // invalid config syntax

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);
        if (!config.count(key))
            continue; // invalid config key

        try
        {
            config[key].value = std::stoi(value);
            totalCfg++;
        }
        catch(...)
        {
            continue;
        }
    }

    if (totalCfg < config.size())
    {
        std::cout << "Updating " << CONFIG_FILENAME << " with new config keys...\n";
        saveConfig();
    }
}
