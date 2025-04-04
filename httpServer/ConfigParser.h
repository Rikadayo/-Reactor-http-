#pragma once
#include <string>
#include <map>
#include <fstream>

class ConfigParser {
public:
    bool load(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) return false;
        
        std::string line;
        std::string section;
        while (std::getline(fin, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            if (line[0] == '[') {
                section = line.substr(1, line.find(']') - 1);
                continue;
            }
            
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = section + "." + line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                configs_[key] = value;
            }
        }
        return true;
    }
    
    std::string get(const std::string& key, const std::string& defaultValue = "") {
        auto it = configs_.find(key);
        return it != configs_.end() ? it->second : defaultValue;
    }
    
private:
    std::map<std::string, std::string> configs_;
}; 