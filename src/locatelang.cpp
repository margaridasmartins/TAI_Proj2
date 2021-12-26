#include <dirent.h>

#include "fcm.hpp"
#include "lang.hpp"

int main(int argc, char *argv[]) {
  string help_text =
      "Usage:\n"
      "  ./locatelang models_dir filename_t context_size alpha \n"
      "Required:\n"
      "  models_dir       The name of the directory with the language files\n"
      "  filename_t       The name of the file with the text under analysis\n"
      "order of the model\n"
      "  alpha          The value for the smoothing parameter\n"
      "Options:\n"
      "  -b           The value of buffer size for switching to another language\n"
      "  -k              Only use one context size "
      "Example:\n"
      "  ./locatelang ?? ?? 2 0.5\n";

  if (argc < 4) {
    printf("ERR: Incorrect number of arguments\n\n%s", help_text.c_str());
    exit(1);
  }

  uint k = 0;
  float a;
  char models_dir[100];
  char filename_t[100];
  sprintf(models_dir, "../%s", argv[1]);
  sprintf(filename_t, "../tests/%s", argv[2]);
  a = atof(argv[3]);

  

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

  string lang;
  string s;
  list<lang_FCM> lang_fcm;
  list<lang_k> lang_k;

  uint b = 10;  // default buffer
  int option, option_index = 0;
  static struct option long_options[] = {
      {"buffer", required_argument, 0, 'b'},
      {"help", no_argument, 0, 'h'},
      {"context_size", required_argument, 0, 'k'},
      {0, 0, 0, 0}};

  while ((option = getopt_long(argc, argv, "bhk", long_options,
                               &option_index)) != -1) {
    switch (option) {
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

  vector<lang_location> locations;

  if (k == 0) {
    locations = locatelang(lang_fcm, fptr_t, a, b);
  } else {
    locations = locatelang_k(lang_k, fptr_t, a, k, b);
  }
  
  closedir(dp);
  fclose(fptr_t);

  return 0;
}
