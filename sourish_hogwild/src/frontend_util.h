#include <getopt.h>
// Extends the definition of option in getopt to add in a message.
struct extended_option
{
  const char *name;
  int has_arg,*flag,val;
  const char *msg;
};

// convert extended options to regular options for use in getopt.
option* convert_extended_options(const extended_option* exs);


// printout a usage string that lists all the options.
// extended options are all default options from above.
// sysname is typically argv[0]
// usage str contains the usage of the mandatory no flag options.
void print_usage(const extended_option *exs, char* sysname, char *usage_str);
