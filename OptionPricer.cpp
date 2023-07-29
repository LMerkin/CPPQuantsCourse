#include "BSM.h"
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("K", po::value<double>(), "set K")
        ("T", po::value<double>(), "set T")
        ("r", po::value<double>(), "set r")
        ("D", po::value<double>(), "set D")
        ("sigma", po::value<double>(), "set sigma")
        ("t", po::value<double>(), "set t")
        ("St", po::value<double>(), "set St")
        ("option", po::value<int>(), "set option type (1: Call, 2: Put, 3: BinaryCall, 4: BinaryPut)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    double K     = vm["K"].as<double>();
    double T     = vm["T"].as<double>();
    double r     = vm["r"].as<double>();
    double D     = vm["D"].as<double>();
    double sigma = vm["sigma"].as<double>();
    double t     = vm["t"].as<double>();
    double St    = vm["St"].as<double>();
    BSM::OptionType optionType = static_cast<BSM::OptionType>(vm["option"].as<int>());

    if(optionType < BSM::OptionType::Call || optionType > BSM::OptionType::BinaryPut) {
        std::cerr << "Invalid option type. Must be one of: 1 (Call), 2 (Put), 3 (BinaryCall), 4 (BinaryPut)" << std::endl;
        return 1;
    }

    double px = BSM::PriceOption(optionType, K, T, r, D, sigma, t, St);
    std::cout << px << std::endl;
    return 0;
}
