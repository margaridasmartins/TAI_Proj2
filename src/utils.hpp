#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <string>

using namespace std;

string replace_all(string str, const string &from, const string &to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos +=
        to.length();  // handle case where 'to' is a substring of 'from'
  }
  return str;
}

#endif
