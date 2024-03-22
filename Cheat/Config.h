#pragma once


#include <string>
#include <vector>
#include <utility> // for std::pair

class Config {
public:
    // Function to load configuration from the file
    bool LoadFromFile(const std::string& filePath);

    // Getters
    const std::vector<std::string>& GetComboBoxLabels() const { return comboBoxLabels; }
    const std::vector<std::pair<int, int>>& GetSliderValues() const { return sliderValues; }
    const std::vector<int>& GetDelayValues() const { return delayValues; }

private:
    std::vector<std::string> comboBoxLabels;
    std::vector<std::pair<int, int>> sliderValues;
    std::vector<int> delayValues;
};