#include <iostream>

using namespace std;

void error(const string message) {
  cerr << message;
  abort();
}