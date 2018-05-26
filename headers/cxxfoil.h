/**
 * @file xfoilinterface.h
 * @author Jesse van Rhijn
 * @brief Library for calling XFoil from a C++ program.
 */
#ifndef __CXXFOIL_H
#define __CXXFOIL_H

#include "include.h"

namespace cxxfoil{

#include "types.h"

constexpr int kOutputBufferSize = 200;
constexpr int kCommandBufferSize = 1024;
constexpr int kSettingsProcessTime = 10;
constexpr int kPolarLineNr = 12;

constexpr int alpha = 0;
constexpr int CL = 1;
constexpr int CD = 2;
constexpr int CDp = 3;
constexpr int CM = 4;
constexpr int Top_Xtr = 5;
constexpr int Bot_Xtr = 6;
constexpr int Top_Itr = 7;
constexpr int Bot_Itr = 8;

typedef enum {
  Success,
  FailPaccOpen,
  FailViscSet,
  FailIterSet,
  FailNaca,
  FailDatFile,
} XfoilError;

/**
 * @brief Class that interfaces with XFoil
 */
class Xfoil {

 public:
  //! Constructor for Xfoil class
  explicit Xfoil(const std::string &path);

  //! Destructor for Xfoil class
  ~Xfoil();

  //! Configures xfoil with constructor parameters
  XfoilError Configure();

  //! sets number of iterations
  XfoilError SetIterations(unsigned int iterations);

  /**
   * @brief Loads airfoil coordinates from file
   * @param fpath File to load coordinates from
   * @param foilname Airfoil name
   */
  XfoilError LoadFoilFile(const std::string& fpath, const std::string& foilname);

  /**
   * @brief Selects a NACA airfoil to input to xfoil
   * @param input 4-digit naca airfoil code
   */
  XfoilError NACA(const char code[5]);

  /**
   * @brief Starts Xfoil analysis of single angle of attack
   * @param angle angle of attack to analyze
   * @returns vector containing calculation results, same order as in xfoil polar files
   */
  std::vector<double> AngleOfAttack(double angle);

  /**
   * @brief Starts Xfoil analysis of sequence of angles
   * @param angle_start starting angle of sequence
   * @param angle_end ending angle of sequence
   * @param angle_increment angle increment
   * @param liftcoeffs array to store results in
   * @returns polar file data structure
  * */
  polar AngleOfAttack(double angle_start, double angle_end, double angle_increment);

  /**
 * @brief Starts Xfoil analysis of single lift coefficient
   * @param lift_coefficient Lift coefficient to analyze
   * @returns vector containing calculation results, same order as in xfoil polar files
   */
  std::vector<double> LiftCoefficient(double lift_coefficient);

  /**
   * @brief Starts Xfoil analysis of sequence of lift coefficients
   * @param cl_start starting lift coefficient
   * @param cl_end ending lift coefficient
   * @param cl_increment lift coefficient increment
   * @param angles array to store results in
   * @returns Polar file data structure
   */
  polar LiftCoefficient(double cl_start, double cl_end, double cl_increment);

  //! Enables viscous mode
  XfoilError SetViscosity(unsigned int Reynolds);

  /**
   * @brief Calculate pressure distribution for given angle of attack
   * @param angle The angle of attack for which to calculate the pressure distribution
   * @returns Vector of tuples (x, Cp)
   */
   std::vector<std::tuple<double, double>> PressureDistribution(double value, const std::string &type="aoa");

 private:

  struct state {
    bool viscous;
    bool G;
    bool foil_loaded;
    bool pacc;
    unsigned int Reynolds;
    double Ncrit;
    unsigned int iter;
    std::string pacc_file;
    std::string foil_name;
  };

  //! struct containing xfoil settings current values
  state xfoil_state_;

  //! Filestream to write into xfoil
  FILE *input_;

  //! Filestream to get xfoil output
  FILE *output_;

  //! pid of xfoil child process
  pid_t process_;

  //! input pipe file descriptors
  int inpipe_[2];

  //! output pipe file descriptors
  int outpipe_[2];

  //! mutex for locking output buffer
  std::mutex mutex_output_;

  //! buffer for xfoil last char output
  char output_buffer_;

  char outp_[kOutputBufferSize];

  //! xfoil log file
  std::ofstream log_;

  std::ofstream input_log_;

  //! thread for reading xfoil output
  std::thread reading_;

  //! bool to stop output logging thread
  volatile bool log_output_;

  //! line number to read from polar file
  int line_number_;

  /**
   * @brief Starts xfoil interface
   * @return Whether XFoil was initialized succesfully
   */
  bool Start(const std::string &path);

  //! Quits xfoil
  bool Quit();

  //! Writes a single newline to xfoil
  void Newline();

  //! Enable polar accumulation
  bool EnablePACC(std::string paccfile);

  //! Disable polar accumulation
  void DisablePACC();

  //! sets Ncrit
  void SetNcrit(double Ncrit);

  //! Read value from location in polar file
  double ReadFromPolar(int linenr, size_t start, size_t end);

  //! Read line from polar file
  std::vector<double> ReadLineFromPolar(int linenr);

  //! let current thread sleep for milliseconds
  void wait_ms(unsigned int milliseconds);

  //! enter command into xfoil and flush buffer
  void Command(const char *cmd, ...);

  //! Read xfoil output
  void ReadOutput();

  //! Checks whether Xfoil is currently waiting for input
  bool WaitingForInput();

  //! Read x, Cp values from file fname
  std::vector<std::tuple<double, double>> ReadPressureFile(const std::string &fname);

  //!
  bool OutputContains(std::string substr);

  //! Loads NACA 1111 if no foil is currently loaded
  void LoadDummyFoil();

};

} // namespace cxxfoil

#endif // __CXXFOIL_H
