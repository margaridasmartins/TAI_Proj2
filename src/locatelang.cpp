#include <bits/stdc++.h>
#include <dirent.h>
#include <unistd.h>

#include "fcm.hpp"
#include "lang.hpp"

const char *reset = "\u001b[0m";
const char *bold = "\u001b[1m";
const char *wrong = "\u001b[31m";
const char *colours[] = {
    "\u001b[33;1m", "\u001b[34;1m", "\u001b[35;1m", "\u001b[36;1m",
    "\u001b[32;1m", "\u001b[31;1m", "\u001b[37;1m",
};

void pprint(FILE *fptr_t, const vector<lang_location> locations,
            const vector<lang_location> real_locations,
            const bool show_accuracy, const bool show_text) {
  // count total number of chars
  fseek(fptr_t, 0L, SEEK_END);
  uint total_chars = ftell(fptr_t) - 3;
  fseek(fptr_t, 0L, SEEK_SET);  // rewind

  if (show_accuracy) {
    // skip first line
    string s;
    char ignore[1000];
    fgets(ignore, 1000, fptr_t);
    s = ignore;
    total_chars -= s.length();
  }

  // language colour pallet
  uint c = 0;
  unordered_map<string, string> pallet;
  for (auto v : locations) {
    if (pallet.find(v.lang) == pallet.end()) {
      pallet[v.lang] = colours[c++ % 8];
    }
  }

  auto next_it = locations.begin(), it = next_it++;
  auto next_rit = real_locations.begin(), rit = next_rit++;
  uint buffer, wrong_chars = 0, read_chars = 0;
  string last_lang;

  do {
    // printf("|%s|\n", it->lang.c_str());

    // print current language
    if (show_text && last_lang.compare(it->lang) != 0) {
      string c = it->lang;
      string s = pallet[c];
      printf("%s<%s:%d>%s", s.c_str(), it->lang.c_str(),
             it->location, reset);
      // printf("badd \n");
      last_lang = it->lang;
      // printf("good \n");
    }

    uint next_it_loc =
        (next_it != locations.end()) ? next_it->location : total_chars;

    uint next_rit_loc = (show_accuracy && next_rit != real_locations.end())
                            ? next_rit->location
                            : total_chars;

    // calculate next buffer
    buffer = min(min(next_it_loc, next_rit_loc), total_chars) - read_chars;
    read_chars += buffer;

    // printf("%s %d %d %d %d %s \n", colours[3], buffer, next_it_loc, next_rit_loc,
    //        total_chars, reset);

    // read buffer
    char stream[buffer + 1];
    if (show_text) {
      fread(stream, 1, buffer, fptr_t);
      stream[buffer] = 0;
    }

    // validate buffer
    if (show_accuracy && last_lang.compare(rit->lang) != 0) {
      wrong_chars += buffer;
      if (show_text) printf("%s%s%s", wrong, stream, reset);
    } else {
      if (show_text) printf("%s", stream);
    }

    // advance inferior iterator
    if (show_accuracy && next_it_loc > next_rit_loc) {
      rit++;
      next_rit++;
    } else {
      it++;
      next_it++;
    }

  } while (it != locations.end() &&
           (!show_accuracy || rit != real_locations.end()));
  printf("\n");

  // read rest of the file
  // char next_char = fgetc(fptr_t);
  // do {
  //   read_chars++;
  //   if (show_text) printf("%c", next_char);
  //   next_char = fgetc(fptr_t);
  // } while (next_char != EOF);

  if (show_accuracy)
    printf("\n%sAccuracy:%s %.2f %%\n", bold, reset,
           (double)(read_chars - wrong_chars) / read_chars * 100);
}

int main(int argc, char *argv[]) {
  string help_text =
      "Usage:\n"
      "  ./locatelang models_dir filename_t alpha [-h] [-b N] [-k N] "
      "[--accuracy] [--print]\n"
      "Required:\n"
      "  models_dir       The name of the directory with the language files\n"
      "  filename_t       The name of the file with the text under analysis\n"
      "  alpha            The value for the smoothing parameter\n"
      "Options:\n"
      "  -h   | --help            Show this message and exit\n"
      "  -b N | --buffer N        The value of buffer size for switching to "
      "another language (default: 10)\n"
      "  -k N | --context-size N  Use a specific context size instead of a "
      "default combination of 3 contexts\n"
      "       | --print           Show the text under analysis whith labeled "
      "with the languages detected\n"
      "       | --accuracy        Read the first line as real locations and "
      "calculate the accuracy\n"
      "Example:\n"
      "  ./locatelang ../models-tiny/ ../tests/locatelang.txt 0.001\n";

  if (argc < 4) {
    printf("ERR: Incorrect number of arguments\n\n%s", help_text.c_str());
    exit(1);
  }

  char models_dir[100];
  char filename_t[100];
  sprintf(models_dir, "../%s", argv[1]);
  sprintf(filename_t, "../%s", argv[2]);
  float a = atof(argv[3]);

  FILE *fptr_t;
  if ((fptr_t = fopen(filename_t, "r")) == NULL) {
    printf("ERR: File \"%s\" not found\n", filename_t);
    exit(2);
  }

  struct dirent *entry;
  DIR *dp = opendir(models_dir);
  if (dp == NULL) {
    fprintf(stderr, "ERR: Path \"%s\" does not exist or could not be read\n",
            models_dir);
    return -1;
  }

  string lang, s;
  list<lang_FCM> lang_fcm;
  list<lang_k> lang_k;

  uint k = 0, b = 10;  // default values
  int show_text = 0, show_accuracy = 0;
  int option, option_index = 0;
  static struct option long_options[] = {
      {"buffer", required_argument, 0, 'b'},
      {"help", no_argument, 0, 'h'},
      {"context_size", required_argument, 0, 'k'},
      {"print", no_argument, &show_text, 1},
      {"accuracy", no_argument, &show_accuracy, 1},
      {0, 0, 0, 0}};

  while ((option = getopt_long(argc, argv, "bkh", long_options,
                               &option_index)) != -1) {
    switch (option) {
      case 0:
        break;
      case 'b':
        b = atoi(argv[optind]);
        break;
      case 'h':
        printf("%s", help_text.c_str());
        exit(0);
      case 'k':
        k = atoi(argv[optind]);
        break;
      default:
        abort();
    }
  }

  while ((entry = readdir(dp))) {
    if (entry->d_name[0] != '.') {
      FILE *fptr;
      s = entry->d_name;
      string lang = s.substr(0, s.find("."));
      char filename[100];
      sprintf(filename, "%s/%s", ((string)models_dir).c_str(), s.c_str());

      if ((fptr = fopen(filename, "r")) == NULL) {
        printf("ERR: File \"%s\" not found\n", filename);
        exit(2);
      }

      if (k == 0) {
        FCM *fcm1 = new FCM(1);
        FCM *fcm2 = new FCM(2);
        FCM *fcm3 = new FCM(5);
        fcm1->train(fptr, a);
        fcm2->train(fptr, a);
        fcm3->train(fptr, a);
        lang_fcm.push_back({fcm1, fcm2, fcm3, lang});
        fclose(fptr);
      } else {
        FCM *fcm = new FCM(k);
        fcm->train(fptr, a);
        lang_k.push_back({fcm, lang});
        fclose(fptr);
      }
    }
  }

  vector<lang_location> locations, real_locations;

  if (show_accuracy) {
    char first_line[1000];
    fgets(first_line, 1000, fptr_t);
    rewind(fptr_t);

    int pos, pos2;
    uint loc;
    string s, token, lang;
    s = first_line;

    do {
      pos = s.find(';');
      token = s.substr(0, pos);
      s.erase(0, pos + 1);

      if ((pos2 = token.find(',')) != -1) {
        lang = token.substr(0, pos2);
        loc = atoi(token.substr(pos2 + 1).c_str());
        if (loc < 0) {
          printf("WARN: ignoring language \"%s\" with negative location\n",
                 lang.c_str());
        } else {
          real_locations.push_back({loc, lang});
        }
      }
    } while (pos != -1);

    sort(real_locations.begin(), real_locations.end(), compare_lang_location);

    printf("%sReal locations received:%s\n", bold, reset);
    for (auto v : real_locations) {
      printf("  %s: %d\n", v.lang.c_str(), v.location);
    }
  }

  printf("%sLocations detected:%s\n", bold, reset);
  if (k == 0) {
    locations = locatelang(lang_fcm, fptr_t, a, b, show_accuracy);
  } else {
    locations = locatelang_k(lang_k, fptr_t, a, k, b, show_accuracy);
  }

  printf("\n");
  pprint(fptr_t, locations, real_locations, show_accuracy, show_text);

  closedir(dp);
  fclose(fptr_t);

  return 0;
}

// 1 E 5 R 11 F 0 E 6 F 10 R

//     EEEERRR