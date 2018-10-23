#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

class Shell{
private:
  int parallels;
  string prompt, tmp_file_str;
  const char* TMP_PIPE;
  const char* TMP_FILE;
  bool tmp_file_full;
public:
  Shell();
  void setPrompt(string p) {prompt = p; return;}
  void shellMain();
  void shellExec(string com, string arg);
  void parallelExec(string com, string arg);
  string pipeExec(string com, string arg);
  char** toCharArr(string s);
  char** strToCharArr(string s);
};
