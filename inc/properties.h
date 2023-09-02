#ifndef RAY_TRACING_PROPERTIES_H
#define RAY_TRACING_PROPERTIES_H

#include <unordered_map>

using namespace std;

struct Properties {
  unordered_map<string, string> map;

  Properties(const string& path);
  string getStringOrNull(const string& key);
  string getString(const string& key);
  int getInt(const string& key);
  unsigned getUnsignedInt(const string& key);
  float getFloat(const string& key);
  bool getBool(const string& key);
};

#endif
