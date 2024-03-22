#include "config.h"
#include <fstream>
#include <sstream>

bool Config::LoadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string label;
            int verticalValue, horizontalValue, delayValue;
            std::getline(iss, label, ':');
            iss >> verticalValue >> horizontalValue >> delayValue;

            comboBoxLabels.push_back(label);
            sliderValues.emplace_back(verticalValue, horizontalValue);
            delayValues.push_back(delayValue);
        }
        file.close();
        return true;
    }

    return false;
}