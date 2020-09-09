//
// Created by Gloomy Pan on 2020/9/8.
//

#include <vector>
#include "utils.h"


int str_has(const char *s, const char *c) {
    int i = 0, j = 0, flag = -1;
    while (i < strlen(s) && j < strlen(c)) {
        if (s[i] == c[j]) {//如果字符相同则两个字符都增加
            i++;
            j++;
        } else {
            i = i - j + 1; //主串字符回到比较最开始比较的后一个字符
            j = 0;     //字串字符重新开始
        }
        if (j == strlen(c)) { //如果匹配成功
            flag = 1;  //字串出现
            break;
        }
    }
    return flag;
}

const char *get_suffix(std::string &uri) {
    int idx = uri.find_last_of(".");
    return uri.substr(idx + 1).c_str();
}

std::vector<std::string> str_spilt(const std::string &str, const std::string &symbol) {
    std::vector<std::string> elements;
    size_t pos = 0;
    size_t len = str.length();
    size_t symbol_len = symbol.length();
    if (symbol_len == 0) return elements;
    while (pos < len) {
        int find_pos = str.find(symbol, pos);
        if (find_pos < 0) {
            elements.push_back(str.substr(pos, len - pos));
            break;
        }
        elements.push_back(str.substr(pos, find_pos - pos));
        pos = find_pos + symbol_len;
    }
    return elements;
}