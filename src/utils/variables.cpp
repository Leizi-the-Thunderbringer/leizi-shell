#include "utils/variables.h"

#include <cctype>

Variable::Variable(const std::string& str, bool readonly)
    : type(VarType::STRING), stringValue(str), intValue(0), isReadonly(readonly) {}

Variable::Variable(const std::vector<std::string>& arr, bool readonly)
    : type(VarType::ARRAY), arrayValue(arr), intValue(0), isReadonly(readonly) {}

Variable::Variable(int value, bool readonly)
    : type(VarType::INTEGER), intValue(value), isReadonly(readonly) {}

std::string Variable::toString() const {
    switch (type) {
        case VarType::STRING:
            return stringValue;
        case VarType::INTEGER:
            return std::to_string(intValue);
        case VarType::ARRAY:
            return arrayValue.empty() ? std::string() : arrayValue.front();
        default:
            return std::string();
    }
}

Variable& VariableManager::set(const std::string& name, const Variable& value) {
    auto [it, _] = variables_.insert_or_assign(name, value);
    return it->second;
}

Variable& VariableManager::setString(const std::string& name, const std::string& value, bool readonly) {
    return set(name, Variable(value, readonly));
}

Variable& VariableManager::setArray(const std::string& name, const std::vector<std::string>& values, bool readonly) {
    return set(name, Variable(values, readonly));
}

Variable& VariableManager::setInteger(const std::string& name, int value, bool readonly) {
    return set(name, Variable(value, readonly));
}

const Variable* VariableManager::get(const std::string& name) const {
    auto it = variables_.find(name);
    return it != variables_.end() ? &it->second : nullptr;
}

Variable* VariableManager::get(const std::string& name) {
    auto it = variables_.find(name);
    return it != variables_.end() ? &it->second : nullptr;
}

bool VariableManager::erase(const std::string& name) {
    return variables_.erase(name) > 0;
}

bool VariableManager::contains(const std::string& name) const {
    return variables_.find(name) != variables_.end();
}

std::string VariableManager::expand(const std::string& input, const Resolver& resolver) const {
    std::string result = input;
    size_t pos = 0;

    while ((pos = result.find('$', pos)) != std::string::npos) {
        if (pos + 1 >= result.size()) break;

        size_t start = pos + 1;
        size_t end = start;

        bool braced = (start < result.size() && result[start] == '{');
        if (braced) {
            ++start;
            end = result.find('}', start);
            if (end == std::string::npos) {
                ++pos;
                continue;
            }
        } else {
            while (end < result.size() &&
                   (std::isalnum(static_cast<unsigned char>(result[end])) || result[end] == '_')) {
                ++end;
            }
        }

        if (end > start) {
            std::string varName = result.substr(start, end - start);
            std::string value;

            if (const auto* variable = get(varName)) {
                value = variable->toString();
            } else if (resolver) {
                if (auto resolved = resolver(varName); resolved.has_value()) {
                    value = *resolved;
                }
            }

            size_t replaceStart = pos;
            size_t replaceEnd = braced ? end + 1 : end;
            result.replace(replaceStart, replaceEnd - replaceStart, value);
            pos = replaceStart + value.length();
        } else {
            ++pos;
        }
    }

    return result;
}

