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
        ("option", po::value<std::string>(), "set option type (CallPx, PutPx, BinaryCallPx, BinaryPutPx)")
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
    std::string option = vm["option"].as<std::string>();

    double px = 0.0;
    if (option == "CallPx") {
        px = BSM::CallPx(K, T, r, D, sigma, t, St);
    } else if (option == "PutPx") {
        px = BSM::PutPx(K, T, r, D, sigma, t, St);
    } else if (option == "BinaryCallPx") {
        px = BSM::BinaryCallPx(K, T, r, D, sigma, t, St);
    } else if (option == "BinaryPutPx") {
        px = BSM::BinaryPutPx(K, T, r, D, sigma, t, St);
    } else {
        std::cerr << "Invalid option type. Must be one of: CallPx, PutPx, BinaryCallPx, BinaryPutPx" << std::endl;
        return 1;
    }

    std::cout << px << std::endl;
    return 0;
}
