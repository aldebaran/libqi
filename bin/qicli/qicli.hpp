#ifndef QICLI_HPP_
# define QICLI_HPP_

# include <qimessaging/session.hpp>
# include <boost/program_options.hpp>

# include "sessionhelper.hpp"

struct MainOptions
{
  std::string   address;
  bool          verbose;
  MainOptions()
    :verbose(false)
  {}
};

typedef int (*SubCmd)(int argc, char **argv, SessionHelper& session, MainOptions const& options);

namespace po = boost::program_options;


/* SUBCMDS */
int subCmd_service(int argc, char **argv, SessionHelper& session, MainOptions const& options);
int subCmd_call(int argc, char **argv, SessionHelper& session, MainOptions const& options);
int subCmd_post(int argc, char **argv, SessionHelper& session, MainOptions const& options);
int subCmd_top(int argc, char **argv, SessionHelper& session, MainOptions const& options);
int subCmd_service(int argc, char **argv, SessionHelper& session, MainOptions const& options);
int subCmd_watch(int argc, char **argv, SessionHelper& session, MainOptions const& options);
int subCmd_get(int argc, char **argv, SessionHelper& session, MainOptions const& options);
int subCmd_set(int argc, char **argv, SessionHelper& session, MainOptions const& options);

/* UTILS */
bool splitName(std::string const& fullName, std::string &beforePoint, std::string &afterPoint);
int readNumericInput();
bool poDefault(po::command_line_parser const& clp, po::variables_map &vm, po::options_description const& desc);
void showHelp(po::options_description const& desc);
std::string getTime();

#endif /* !QICLI_HPP_ */
