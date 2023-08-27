#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

void __MINGW_ATTRIB_NORETURN error(const string message) {
  cerr << message;
  abort();
}

bool contains(unordered_map<string, string>& map, const string& key) {
  return map.find(key) != map.end();
}

bool contains(const string& str, const string& substr) {
  return str.find(substr) != string::npos;
}

string takeBefore(const string& str, const string& sep) {
  size_t position;
  if ((position = str.find(sep)) != string::npos) {
    return str.substr(0, position);
  }
  return str;
}

pair<string, string> split(const string& str, const string& sep) {
  size_t pos1, pos2;
  if ((pos1 = str.find(sep)) != string::npos) {
    pos2 = pos1 + sep.length();
    return { str.substr(0, pos1), str.substr(pos2, str.length() - pos2) };
  }
  error("Cannot split the string. It doesn't contain the separator.");
}

string trim(const string& str, const string& whitespace = " \t") {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos) return "";
  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;
  return str.substr(strBegin, strRange);
}
