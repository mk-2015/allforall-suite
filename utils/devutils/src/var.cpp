#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/math/constants/constants.hpp>

using BigFloat = boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<120>
>;

static void usage()
{
    std::cout <<
        "var - Mathematical Constant Utility\n\n"
        "Usage:\n"
        "  var [CONSTANT] [OPTIONS]\n\n"
        "Constants:\n"
        "  E                   Euler's number\n"
        "  PI                  Pi\n"
        "  PI2                 Pi squared\n"
        "  E2                  Euler's number squared\n"
        "  SQRT2               Square root of 2\n"
        "  PYTHAGORAS          Alias for SQRT2\n"
        "  GR                  Golden ratio\n"
        "  GOLDEN_RATIO        Alias for GR\n"
        "  EM                  Euler-Mascheroni constant\n"
        "  EULER_MASCHERONI    Alias for EM\n"
        "  AP                  Apery's constant\n"
        "  APERY               Alias for AP\n\n"
        "Options:\n"
        "  -h       Display this help\n"
        "  -d       Output integer part only\n"
        "  -f       Output fractional digits only\n"
        "  -p N     Set precision (default: 5, maximum: 100)\n";
}

static BigFloat get_constant(const std::string& name)
{
    using namespace boost::math::constants;

    if (name == "PI")
        return pi<BigFloat>();

    if (name == "E")
        return e<BigFloat>();

    if (name == "PI2")
        return pi<BigFloat>() * pi<BigFloat>();

    if (name == "E2")
        return e<BigFloat>() * e<BigFloat>();

    if (name == "SQRT2" || name == "PYTHAGORAS")
        return root_two<BigFloat>();

    if (name == "GR" || name == "GOLDEN_RATIO")
        return phi<BigFloat>();

    if (name == "EM" || name == "EULER_MASCHERONI")
        return euler<BigFloat>();

    if (name == "AP" || name == "APERY")
        return zeta_three<BigFloat>();

    throw std::runtime_error("Unknown constant: " + name);
}

int main(int argc, char* argv[])
{
    std::string constant;

    int precision = 5;
    bool integer_only = false;
    bool fraction_only = false;

    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "-h") == 0)
        {
            usage();
            return 0;
        }
        else if (std::strcmp(argv[i], "-d") == 0)
        {
            integer_only = true;
        }
        else if (std::strcmp(argv[i], "-f") == 0)
        {
            fraction_only = true;
        }
        else if (std::strcmp(argv[i], "-p") == 0)
        {
            if (++i >= argc)
            {
                std::cerr << "var: -p requires a precision value\n";
                return 1;
            }

            char* end = nullptr;
            long value = std::strtol(argv[i], &end, 10);

            if (*argv[i] == '\0' || *end != '\0' ||
                value < 0 || value > 100)
            {
                std::cerr << "var: precision must be between 0 and 100\n";
                return 1;
            }

            precision = static_cast<int>(value);
        }
        else if (argv[i][0] == '-')
        {
            std::cerr << "var: unknown option: " << argv[i] << '\n';
            return 1;
        }
        else if (constant.empty())
        {
            constant = argv[i];
        }
        else
        {
            std::cerr << "var: unexpected argument: " << argv[i] << '\n';
            return 1;
        }
    }

    if (integer_only && fraction_only)
    {
        std::cerr << "var: -d and -f cannot be used together\n";
        return 1;
    }

    if (constant.empty())
    {
        std::cerr << "var: missing constant\n";
        std::cerr << "Try 'var -h' for help.\n";
        return 1;
    }

    try
    {
        BigFloat value = get_constant(constant);

        std::ostringstream output;

        output << std::fixed
               << std::setprecision(precision)
               << value;

        std::string result = output.str();

        if (integer_only)
        {
            std::cout << result.substr(0, result.find('.')) << '\n';
        }
        else if (fraction_only)
        {
            std::size_t dot = result.find('.');

            if (dot != std::string::npos)
                std::cout << result.substr(dot + 1);

            std::cout << '\n';
        }
        else
        {
            std::cout << result << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "var: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
