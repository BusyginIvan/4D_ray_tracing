#ifndef RAY_TRACING_UTIL_H
#define RAY_TRACING_UTIL_H

#include <iostream>
#include <unordered_map>

using namespace std;

void __MINGW_ATTRIB_NORETURN error(const string& message);

bool contains(unordered_map<string, string>& map, const string& key);

bool contains(const string& str, const string& substr);
string takeBefore(const string& str, const string& sep);
pair<string, string> split(const string& str, const string& sep);
string trim(const string& str, const string& whitespace = " \t");
string toUpperCase(const string& str);
string toLowerCase(const string& str);

#endif
