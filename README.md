# Option Pricer
This project is an implementation of the Black-Scholes-Merton model for pricing various types of options. It supports pricing for standard call and put options, as well as binary call and put options.

## Building the Project
To build the project, simply run the make command in the root directory of the project:

```bash
make
```
This will compile the source code and generate the OptionPricer executable.

## Running the Program
You can run the OptionPricer program with command line arguments to specify the parameters for the option pricing model. Here's an example:

```bash
./OptionPricer --K=100 --T=2023.9 --r=0.05 --D=0.02 --sigma=0.25 --t=2023.6 --St=100 --option=CallPx
```
In this example:

- K is the strike price of the option.
- T is the expiration time of the option, expressed as a year fraction (e.g., 2023.9).
- r is the risk-free interest rate.
- D is the dividend rate.
- sigma is the implied volatility.
- t is the pricing time, expressed as a year fraction.
- St is the price of the underlying asset at time t.
option is the type of option to price. It can be one of CallPx, PutPx, BinaryCallPx, or BinaryPutPx.
The program will calculate the price of the specified option using the provided parameters and print the result to the console.