#ifndef _FCM_HPP_
#define _FCM_HPP_

#include <assert.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>

#include "utils.hpp"

using namespace std;

struct state {
  map<char, uint> occorrencies;
  uint sum;
};

class Table {
 protected:
  uint k;
  map<char, uint> symbols;  // overall symbol frequency
  uint total;               // number of symbols in the file

 public:
  Table(uint k, map<char, uint> symbols);
  virtual uint getSymbolsSize()=0;
  virtual void train(FILE *fptr) = 0;
  virtual double letter_entropy(char *context, char letter, float a, uint symbols_size) = 0;
  virtual double get_entropy(float a) = 0;
  virtual void generate_text(float a, char prior[], uint text_size,
                             bool relative_random, bool show_random) = 0;
  virtual void print() = 0;
};

class TableHash : public Table {
 private:
  unordered_map<string, state> table;  // hash table

 public:
  TableHash(uint k, map<char, uint> symbols);
  uint getSymbolsSize();
  void train(FILE *fptr);
  double letter_entropy(char *context, char letter, float a, uint simbols_size);
  double get_entropy(float a);
  void generate_text(float a, char prior[], uint text_size,
                     bool relative_random, bool show_random);
  void print();
};

class FCM {
 private:
  uint k;
  Table *table;
  map<char, uint> symbols;
  map<char, uint> alphabet;

 public:
  FCM(uint k);

  uint getSymbolSize();
  void train(FILE *fptr, float threshold);
  double get_entropy(float a);
  double letter_entropy(char *context, char letter, float a, uint simbols_size);
  void generate_text(float a, char prior[], uint text_size,
                     bool relative_random, bool show_random);
  void print_table();
};

Table::Table(uint k, map<char, uint> symbols) {
  this->k = k;
  this->symbols = symbols;
  this->total = 0;
}

///////////////////////////////////////////////////////////////////////

TableHash::TableHash(uint k, map<char, uint> symbols) : Table(k, symbols) {}

void TableHash::train(FILE *fptr) {
  rewind(fptr);
  table.clear();

  char context[k];

  // first k letters
  fgets(context, k + 1, fptr);

  // k+1 letter
  char next_char = fgetc(fptr);
  do {
    symbols[next_char]++;
    this->total++;

    if (table.find(context) != table.end()) {
      table[context]
          .occorrencies[next_char]++;  // increment count of the symbol nextchar
                                       // for given context
      table[context].sum++;  // increase count of times the context was found
    } else {
      map<char, uint> occ;  // create a new state if context is not found
      occ[next_char] = 1;
      state st = {occ, 1};
      table[context] = st;
    }

    // slide one
    for (uint i = 0; i < k - 1; i++) {
      context[i] = context[i + 1];
    }
    context[k - 1] = next_char;

    next_char = fgetc(fptr);

  } while (next_char != EOF);
  rewind(fptr);
}

void TableHash::print() {
  // print header
  printf("%6s ", "");
  for (auto s : symbols) {
    string c;
    c += s.first;
    c = replace_all(c, "\n", "\\n");
    printf("%4s ", c.c_str());
  }
  printf("\n");

  // print rows from all existant contexts
  for (auto pair : table) {
    string c = replace_all(pair.first, "\n", "\\n");
    printf("%6s ", c.c_str());

    auto it1 = symbols.begin();
    auto it2 = pair.second.occorrencies.begin();

    while (it1 != symbols.end()) {
      if ((*it1).first == (*it2).first) {
        printf("%4d ", (*it2).second);
        it1++;
        it2++;
      } else {
        // place 0 for characters not saved
        printf("%4d ", 0);
        it1++;
      }
    }
    printf("\n");
  }
}

uint TableHash::getSymbolsSize(){
  return symbols.size();
}

double TableHash::letter_entropy(char *context, char letter, float a, uint symbols_size) {
  double prob = 0;
  auto it = table.find(context);

  if (it != table.end()) {
    auto it2 = (*it).second.occorrencies.find(letter);

    if (it2 != (*it).second.occorrencies.end()) {
      prob = (double)((*it2).second + a) / ((*it).second.sum + a * symbols_size);
    } else {
      prob = (double)(a) / ((*it).second.sum + a * symbols_size);
    }
  } else {
    prob = (double)1 / symbols_size;
  }

  return -log2(prob);
}

double TableHash::get_entropy(float a) {
  assert(a >= 0);
  double ent = 0;
  double contextEnt = 0;
  double letterProb;

  // transverse the table and calculate entropy
  for (auto pair : table) {
    auto it = pair.second.occorrencies.begin();
    while (it != pair.second.occorrencies.end()) {
      letterProb =
          (double)((*it).second + a) / (pair.second.sum + a * symbols.size());
      contextEnt -= letterProb * log2(letterProb);
      it++;
    }
    // letters not present in occorrencies
    int n = symbols.size() - pair.second.occorrencies.size();
    letterProb = (double)a / (pair.second.sum + a * symbols.size());
    contextEnt -= (letterProb * log2(letterProb)) * n;

    ent += (double)(pair.second.sum + a * symbols.size()) /
           (this->total + a * pow(symbols.size(), k + 1)) * contextEnt;
    contextEnt = 0;
  }
  // contexts not present in table
  double n = pow(symbols.size(), k) - table.size();
  letterProb = (double)1 / symbols.size();
  contextEnt = -(letterProb * log2(letterProb)) * symbols.size();

  ent += (double)a * symbols.size() /
         (this->total + a * pow(symbols.size(), k + 1)) * contextEnt * n;

  return ent;
}

void TableHash::generate_text(float a, char *prior, uint text_size,
                              bool relative_random, bool show_random) {
  assert(a >= 0);
  float random;
  float prob;

  if (prior[0] == 0) {
    // create random prior
    auto it = table.begin();
    advance(it, (int)(rand() % table.size()));
    strcpy(prior, (it->first).c_str());
  }

  printf("\n\33[4m%s\33[0m", prior);
  // write text_size characters
  for (uint i = 0; i < text_size; i++) {
    prob = 0;
    random = (float)rand() / RAND_MAX;  // generate target probability

    if (table.find(prior) != table.end()) {
      for (auto pair : symbols) {
        // add the probabilities of each letter until random is smaller
        if (table[prior].occorrencies.find(pair.first) !=
            table[prior].occorrencies.end()) {
          // symbol registed on map
          prob += (double)(table[prior].occorrencies[pair.first] + a) /
                  (table[prior].sum + a * symbols.size());
        } else {
          // symbol not found
          prob += (double)a / (table[prior].sum + a * symbols.size());
        }

        if (prob > random) {
          printf("%c", pair.first);
          for (uint j = 0; j < k - 1; j++) {
            prior[j] = prior[j + 1];
          }
          prior[k - 1] = pair.first;
          break;
        }
      }
    } else {
      char rand_char;
      if (relative_random) {
        // put a random char relative to the letter occorrency
        for (auto pair : symbols) {
          prob += (float)pair.second / this->total;
          if (prob > random) {
            rand_char = pair.first;
            break;
          }
        }
      } else {
        // put a random char uniformly distributed
        auto it = symbols.begin();
        advance(it, rand() % symbols.size());
        rand_char = (*it).first;
      }

      if (show_random)
        printf("\33[34m%c\33[0m", rand_char);
      else
        printf("%c", rand_char);

      for (uint j = 0; j < k - 1; j++) {
        prior[j] = prior[j + 1];
      }
      prior[k - 1] = rand_char;
    }
  }
  printf("\n");
}

///////////////////////////////////////////////////////////////////////

FCM::FCM(uint k) { this->k = k; }

void FCM::train(FILE *fptr, float threshold = 0) {
  rewind(fptr);  // move the file pointer back to the start of the file
  symbols.clear();
  alphabet.clear();

  // check the alphabet of the file
  uint id = 0;
  char c = fgetc(fptr);
  do {
    if (symbols.find(c) == symbols.end()) {
      // new symbol
      symbols[c] = 0;
      alphabet[c] = id++;
    }
    c = fgetc(fptr);
  } while (c != EOF);

  // Calculate theorical size of 2D array
  double tablesize = (pow(symbols.size(), k + 1)) * 16 / 8 / 1024 / 1024;
  printf("Theoretical size of 2D array: %f MB\n", tablesize);

  printf("Creating hash table...\n");
  table = new TableHash(k, symbols);
  table->train(fptr);
}
uint FCM::getSymbolSize(){return table->getSymbolsSize();};
double FCM::get_entropy(float a) { return table->get_entropy(a); }
double FCM::letter_entropy(char context[], char next_char, float a, uint size) {
  return table->letter_entropy(context, next_char, a, size);
}
void FCM::print_table() { table->print(); }

void FCM::generate_text(float a, char prior[], uint text_size,
                        bool relative_random, bool show_random) {
  table->generate_text(a, prior, text_size, relative_random, show_random);
}

#endif
