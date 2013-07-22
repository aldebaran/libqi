#ifndef QICLI_HPP_
# define QICLI_HPP_

# include <qimessaging/session.hpp>
# include <boost/program_options.hpp>

# include "sessionhelper.hpp"

struct MainOptions
{
  std::string   address;
};

typedef int (*SubCmd)(int argc, char **argv, const MainOptions &options);

namespace po = boost::program_options;


/* SUBCMDS */
int subCmd_service(int argc, char **argv, const MainOptions &options);
int subCmd_call(int argc, char **argv, const MainOptions &options);
int subCmd_post(int argc, char **argv, const MainOptions &options);
int subCmd_service(int argc, char **argv, const MainOptions &options);
int subCmd_watch(int argc, char **argv, const MainOptions &options);
int subCmd_get(int argc, char **argv, const MainOptions &options);
int subCmd_set(int argc, char **argv, const MainOptions &options);
int subCmd_trace(int argc, char **argv, const MainOptions &options);
int subCmd_top(int argc, char **argv, const MainOptions &options);

/* UTILS */
int readNumericInput();
std::string readAlphaInput();
bool poDefault(const po::command_line_parser &clp, po::variables_map &vm, const po::options_description &desc);
void showHelp(const po::options_description &desc);
std::string getTime();
bool isNumber(const std::string &str);
void printError(const std::string &errorStr);
void printSuccess();
void printServiceMember(const std::string &service, const std::string &member);

/** Given available services list @param allServices, parse list of requested services
 * @param names, that may contain globbing pattern and a '-' prefix to remove
 * matches from list. Each element of @param name
 * can also be a comma-separated list of globs.
 * @return the resolved list of services
*/
std::vector<std::string> parseServiceList(
  const std::vector<std::string>& names,
  const std::vector<std::string>& allServices);
#endif /* !QICLI_HPP_ */
