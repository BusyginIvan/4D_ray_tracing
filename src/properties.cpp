#include "properties.h"
#include <fstream>
#include <string>
#include <iomanip>
#include "util.h"

using namespace std;

static const string COMMENT_SEPARATOR = "#";
static const string KEY_VALUE_SEPARATOR = "=";

Properties::Properties(const string& path) {
  const string ERROR_MESSAGE_BASE = "Failed to initialize properties.";
  string line;
  ifstream inputFileStream(path);

  if (inputFileStream.is_open()) {
    while (getline(inputFileStream, line)) {
      line = trim(takeBefore(line, COMMENT_SEPARATOR));
      if (line.empty())
        continue;
      if (!contains(line, KEY_VALUE_SEPARATOR))
        error(ERROR_MESSAGE_BASE + " Cannot parse the line: \"" + line + "\"");
      pair<string, string> keyAndValue = split(line, KEY_VALUE_SEPARATOR);
      map.insert({ trim(keyAndValue.first), trim(keyAndValue.second) });
    }
  } else {
    error(ERROR_MESSAGE_BASE + " Cannot open file.");
  }

  inputFileStream.close();
}

string Properties::getStringOrNull(const string& key) {
  return map[key];
}

string Properties::getString(const string& key) {
  if (!contains(map, key)) {
    error("Error! Cannot find property \"" + key + "\".");
  }
  cout << key << ": " << map[key] << endl;
  return map[key];
}

int Properties::getInt(const string& key) {
  try {
    return stoi(getString(key));
  } catch (invalid_argument const &e) {
    error("Error! Cannot parse int value of property \"" + key + "\".");
  } catch (out_of_range const &e) {
    error("Error! Value of property \"" + key + "\" is out of the range.");
  }
}

unsigned Properties::getUnsignedInt(const string& key) {
  int value = getInt(key);
  if (value < 0) error("Error! Value of property \"" + key + "\" must be positive.");
  return value;
}

float Properties::getFloat(const string& key) {
  stringstream stream(getString(key));
  float result;
  if (stream >> result) {
    return result;
  } else {
    error("Error! Cannot parse float value of property \"" + key + "\".");
  }
}