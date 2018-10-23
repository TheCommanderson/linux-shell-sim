#include "Shell.h"

int main(int argc, char* argv[]){
  Shell* sh = new Shell;
  int c;
  while((c = getopt(argc, argv, "t?")) != -1){
    switch(c){
      case 't':
        sh->setPrompt("");
        break;
      case '?':
        cout << "Please only use arg -t\n";
        exit(1);
        break;
    }
  }
  sh->shellMain();
  delete sh;
  return 0;
}
