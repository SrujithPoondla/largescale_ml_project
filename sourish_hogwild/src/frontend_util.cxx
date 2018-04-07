#include "frontend_util.h"
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

option*
convert_extended_options(const extended_option* exs)  {
  int nOptions = 0; 
  while(exs[nOptions++].name != NULL);
  struct option* opts =  new option[nOptions];
  for(int i = 0; i < nOptions; i++) {
    opts[i].name    = exs[i].name;
    opts[i].has_arg = exs[i].has_arg;
    opts[i].flag    = exs[i].flag;
    opts[i].val     = exs[i].val;
  }
  return opts;
}

void
print_usage(const extended_option *exs, char* sysname, char *usage_str) {
  string flags = "[";
  int i = 0;
  for(i=0;exs[i].name != NULL;i++) {
    string s = string("--") + string(exs[i].name);
    flags += (s + ((exs[i+1].name != NULL) ? string("|") : string("]")));
  }
  std::cout << "usage: " << sysname << " " << flags << " " << usage_str << std::endl;
   

  for(i=0;exs[i].name != NULL;i++) {
     string s = string("--") + string(exs[i].name);
     std::cout << "\t" << setw(20) << left << s  << "\t: " << exs[i].msg << std::endl;
   }
}
