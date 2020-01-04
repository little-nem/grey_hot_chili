#include <vector>
#include <iostream>

#include "optionparser.h"
#include "interpolation.h"

struct Arg: public option::Arg
{
    static void printError(const char* msg1, const option::Option& opt, const char* msg2)
    {
        std::cerr << msg1 << std::endl;
        fwrite(opt.name, opt.namelen, 1, stderr);
        std::cerr << msg2 << std::endl;
    }
    static option::ArgStatus Unknown(const option::Option& option, bool msg)
    {
        if (msg) {
            printError("Unknown option '", option, "'\n");
        }
        
        return option::ARG_ILLEGAL;
    }
    static option::ArgStatus Required(const option::Option& option, bool msg)
    {
        if (option.arg != 0) {
            return option::ARG_OK;
        }

        if (msg) {
            printError("Option '", option, "' requires an argument\n");
        }

        return option::ARG_ILLEGAL;
      }
    static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0) {
            return option::ARG_OK;
        }

        if (msg) {
            printError("Option '", option, "' requires a non-empty argument\n");
        }

        return option::ARG_ILLEGAL;
    }
    static option::ArgStatus Numeric(const option::Option& option, bool msg)
    {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10)) {};

        if (endptr != option.arg && *endptr == 0) {
            return option::ARG_OK;
        }

        if (msg) {
            printError("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }
};

enum  optionIndex { UNKNOWN, HELP, N};

const option::Descriptor usage[] = {
    { UNKNOWN, 0,"", "",        Arg::Unknown, "USAGE: temp_name source.png target.png [options]\n\n"
                                              "Options:" },
    { HELP,    0,"h", "help",    Arg::None,    "  \t--help  \tPrint usage and exit." },
    { N, 0,"N","resdirac", Arg::Numeric, "  -N <num>, \t--resdirac=<num>  \tSpecify the number of Diracs used to sample target image" },
    { UNKNOWN, 0,"", "",        Arg::None,
     "\nExamples:\n"
     "  texture_generation source.png target.png\n"
    },
    { 0, 0, 0, 0, 0, 0 } 
};

int main(int argc, char* argv[])
{
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present

    std::string source_image_name, target_image_name;
    int N = 128;
    
    bool source_image_path_argument = (argc > 0);

    if(source_image_path_argument) {
        source_image_name = std::string(argv[0]);
    }
    
    argc -= (argc>0); argv += (argc>0); // skip image name if present

    bool target_image_path_argument = (argc > 0);

    if(target_image_path_argument) {
        target_image_name = std::string(argv[0]);
    }

    option::Stats stats(usage, argc, argv);

    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error()) {
        return 1;
    }

    if (options[HELP] || !(source_image_path_argument && target_image_path_argument))
    {
        int columns = getenv("COLUMNS")? atoi(getenv("COLUMNS")) : 80;
        option::printUsage(fwrite, stdout, usage, columns);
        return 0;
    }

    for (int i = 0; i < parse.optionsCount(); ++i)
    {
        option::Option& opt = buffer[i];
        if(opt.index() == N) {
            N = std::stoi(opt.arg);
        }
    }

    std::cout << "Welcome in the project; trying to load " << source_image_name 
              << "and" << target_image_name << std::endl;

    interpolation(source_image_name, target_image_name, N);

    return 0;
}
