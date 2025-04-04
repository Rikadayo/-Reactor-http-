#pragma once
#include <string>
#include <map>
#include <tinyxml2.h>

class SqlConfig {
public:
    static SqlConfig& getInstance() {
        static SqlConfig instance;
        return instance;
    }
    
    bool load(const std::string& filename) {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
            return false;
        }
        
        auto root = doc.FirstChildElement("sqlmap");
        if (!root) return false;
        
        for (auto node = root->FirstChildElement("sql"); 
             node; 
             node = node->NextSiblingElement("sql")) {
            const char* id = node->Attribute("id");
            const char* sql = node->GetText();
            if (id && sql) {
                sqlMap_[id] = sql;
            }
        }
        return true;
    }
    
    std::string getSql(const std::string& id) const {
        auto it = sqlMap_.find(id);
        return it != sqlMap_.end() ? it->second : "";
    }
    
private:
    SqlConfig() = default;
    std::map<std::string, std::string> sqlMap_;
}; 