#include "variables.h"
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>

namespace leizi {

// Variable implementation
Variable::Variable(const VariableValue& val, bool exported)
    : value(val) {
    if (exported) {
        flags |= static_cast<uint8_t>(VariableFlags::EXPORTED);
    }
}

bool Variable::isExported() const {
    return flags & static_cast<uint8_t>(VariableFlags::EXPORTED);
}

bool Variable::isReadonly() const {
    return flags & static_cast<uint8_t>(VariableFlags::READONLY);
}

bool Variable::isLocal() const {
    return flags & static_cast<uint8_t>(VariableFlags::LOCAL);
}

bool Variable::isSpecial() const {
    return flags & static_cast<uint8_t>(VariableFlags::SPECIAL);
}

std::string Variable::toString() const {
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    } else if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<std::vector<std::string>>(value)) {
        const auto& arr = std::get<std::vector<std::string>>(value);
        if (!arr.empty()) {
            return arr[0];  // Return first element for string context
        }
    }
    return "";
}

std::vector<std::string> Variable::toArray() const {
    if (std::holds_alternative<std::vector<std::string>>(value)) {
        return std::get<std::vector<std::string>>(value);
    } else {
        return {toString()};
    }
}

// VariableManager implementation
VariableManager::VariableManager() {
    initializeSpecialVariables();
}

void VariableManager::initializeSpecialVariables() {
    // Initialize from environment
    for (char** env = environ; *env != nullptr; ++env) {
        std::string envVar(*env);
        size_t pos = envVar.find('=');
        if (pos != std::string::npos) {
            std::string name = envVar.substr(0, pos);
            std::string value = envVar.substr(pos + 1);
            set(name, value, true);
        }
    }

    // Set shell-specific variables
    set("SHELL", "/usr/local/bin/leizi", true);
    set("LEIZI_VERSION", "1.0.1", true);

    // Initialize special dynamic variables
    registerSpecialVar("PWD", []() {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            return std::string(cwd);
        }
        return std::string();
    });

    registerSpecialVar("USER", []() {
        const char* user = getenv("USER");
        if (!user) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                return std::string(pw->pw_name);
            }
        }
        return user ? std::string(user) : std::string();
    });

    registerSpecialVar("HOME", []() {
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                return std::string(pw->pw_dir);
            }
        }
        return home ? std::string(home) : std::string();
    });

    registerSpecialVar("HOSTNAME", []() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
        return std::string("localhost");
    });

    registerSpecialVar("$$", []() {
        return std::to_string(getpid());
    });
}

void VariableManager::set(const std::string& name, const VariableValue& value, bool exported) {
    variables_[name] = Variable(value, exported);
}

void VariableManager::setReadonly(const std::string& name, const VariableValue& value) {
    Variable var(value);
    var.flags |= static_cast<uint8_t>(VariableFlags::READONLY);
    variables_[name] = var;
}

void VariableManager::setLocal(const std::string& name, const VariableValue& value) {
    Variable var(value);
    var.flags |= static_cast<uint8_t>(VariableFlags::LOCAL);
    variables_[name] = var;
}

Variable* VariableManager::get(const std::string& name) {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return &it->second;
    }
    return nullptr;
}

const Variable* VariableManager::get(const std::string& name) const {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string VariableManager::getValue(const std::string& name) const {
    // Check special variables first
    auto specialIt = specialVars_.find(name);
    if (specialIt != specialVars_.end()) {
        return specialIt->second();
    }

    const Variable* var = get(name);
    if (var) {
        return var->toString();
    }
    return "";
}

bool VariableManager::unset(const std::string& name) {
    auto it = variables_.find(name);
    if (it != variables_.end() && !it->second.isReadonly()) {
        variables_.erase(it);
        return true;
    }
    return false;
}

void VariableManager::exportVar(const std::string& name) {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        it->second.flags |= static_cast<uint8_t>(VariableFlags::EXPORTED);
    }
}

void VariableManager::unexportVar(const std::string& name) {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        it->second.flags &= ~static_cast<uint8_t>(VariableFlags::EXPORTED);
    }
}

std::string VariableManager::expand(const std::string& text) const {
    std::string result;
    size_t i = 0;

    while (i < text.length()) {
        if (text[i] == '$') {
            if (i + 1 < text.length()) {
                if (text[i + 1] == '{') {
                    // ${var} format
                    size_t end = text.find('}', i + 2);
                    if (end != std::string::npos) {
                        std::string varName = text.substr(i + 2, end - i - 2);
                        result += expandVariable(varName);
                        i = end + 1;
                        continue;
                    }
                } else if (text[i + 1] == '$') {
                    // $$ special case
                    result += getValue("$$");
                    i += 2;
                    continue;
                } else if (text[i + 1] == '?') {
                    // $? special case
                    result += getValue("?");
                    i += 2;
                    continue;
                } else {
                    // $var format
                    size_t start = i + 1;
                    size_t end = start;
                    while (end < text.length() &&
                           (std::isalnum(text[end]) || text[end] == '_')) {
                        end++;
                    }
                    if (end > start) {
                        std::string varName = text.substr(start, end - start);
                        result += expandVariable(varName);
                        i = end;
                        continue;
                    }
                }
            }
        }
        result += text[i];
        i++;
    }

    return result;
}

std::string VariableManager::expandVariable(const std::string& varName) const {
    // Handle parameter expansion formats like ${var:-default}
    size_t colonPos = varName.find(':');
    if (colonPos != std::string::npos && colonPos + 1 < varName.length()) {
        std::string name = varName.substr(0, colonPos);
        char op = varName[colonPos + 1];
        std::string param = (colonPos + 2 < varName.length()) ?
                            varName.substr(colonPos + 2) : "";

        std::string value = getValue(name);

        if (op == '-') {
            // ${var:-default} - use default if var is unset or empty
            return value.empty() ? param : value;
        } else if (op == '+') {
            // ${var:+alt} - use alt if var is set and not empty
            return !value.empty() ? param : "";
        } else if (op == '=') {
            // ${var:=default} - set var to default if unset or empty
            if (value.empty()) {
                const_cast<VariableManager*>(this)->set(name, param);
                return param;
            }
            return value;
        } else if (op == '?') {
            // ${var:?error} - error if var is unset or empty
            if (value.empty()) {
                throw std::runtime_error("Variable " + name + " is not set: " + param);
            }
            return value;
        }
    }

    return getValue(varName);
}

std::unordered_map<std::string, Variable> VariableManager::getAllVariables() const {
    return variables_;
}

std::unordered_map<std::string, std::string> VariableManager::getExportedVariables() const {
    std::unordered_map<std::string, std::string> exported;
    for (const auto& [name, var] : variables_) {
        if (var.isExported()) {
            exported[name] = var.toString();
        }
    }
    return exported;
}

void VariableManager::registerSpecialVar(const std::string& name,
                                         std::function<std::string()> getter) {
    specialVars_[name] = getter;
}

} // namespace leizi