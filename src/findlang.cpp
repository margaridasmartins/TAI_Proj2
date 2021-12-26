#include "lang.hpp"
#include <bits/stdc++.h>

int main(int argc, char *argv[]) {
  string help_text =
      "Usage:\n"
      "  ./lang models_dir filename_t context_size alpha \n"
      "Required:\n"
      "  models_dir     The name of the directory with the language models"
      "  filename_t       The name of the file with the text under analysis\n"
      "  context_size   The size of the context which translates into the order of the model\n"
      "  alpha          The value for the smoothing parameter\n"
      "Example:\n"
      "  ./lang ?? 2 0.5\n";

  if (argc < 4) {
    printf("ERR: Incorrect number of arguments\n\n%s", help_text.c_str());
    exit(1);
  }

  uint k;
  float a;
  char models_dir[100];
  char filename_t[100];
  sprintf(models_dir, "%s", argv[1]);
  sprintf(filename_t, "%s", argv[2]);
  k = atoi(argv[3]);
  a = atof(argv[4]);

  FILE *fptr_t;
  if ((fptr_t = fopen(filename_t, "r")) == NULL) {
    printf("ERR: File \"%s\" not found\n", filename_t);
    exit(2);
  }

  struct dirent *entry;
  DIR *dp;

  dp = opendir(models_dir);
  if (dp == NULL) {
    perror("opendir: Path does not exist or could not be read.");
    return -1;
  }

  vector<pair<double, string>> langs;
  string s;

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

      FCM *fcm = new FCM(k);

      fcm->train(fptr, a);

      printf("Trained %s model with \t %f entropy\n", lang.c_str(), fcm->get_entropy(a));

      double nbits = get_numbits(fcm, fptr_t, k, a);

      langs.push_back({nbits, lang});

      rewind(fptr_t);
      fclose(fptr);
    }
  }
  fclose(fptr_t);
  closedir(dp);

  sort(langs.begin(), langs.end());

  printf("Top 5 nearest languages:\n");
  for (auto it = langs.begin(); it != langs.begin() + 5; ++it) {
    printf("%20s: \t %f avg bits\n", it->second.c_str(), it->first);
  }

  return 0;
}