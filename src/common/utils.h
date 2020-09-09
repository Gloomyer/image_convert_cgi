#include <iostream>

#ifndef MY_UTILS_HPP
#define MY_UTILS_HPP

int str_has(const char *s, const char *c);

const char *get_suffix(std::string &uri);

std::vector<std::string> str_spilt(const std::string &str, const std::string &symbol = ",");

#endif //UTILS_HPP

