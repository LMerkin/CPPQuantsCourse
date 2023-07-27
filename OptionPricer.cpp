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

    double callPx = BSM::CallPx(K, T, r, D, sigma, t, St);
    std::cout << callPx << std::endl;
    return 0;
}