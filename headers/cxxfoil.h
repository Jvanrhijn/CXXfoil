/**
 * @file xfoilinterface.h
 * @author Jesse van Rhijn
 * @brief Library for calling XFoil from a C++ program.
 */
#ifndef XFOIL_H
#define XFOIL_H

#include "include.h"

namespace cxxfoil{

#include "types.h"

#define CMD_BUFF_SIZE 1024 // this is ugly and needs a better implementation
#define OUTPUT_BUFF_SIZE 200

#define SETTINGS_PROCESS_TIME 10

#define POLAR_DATA_LINENR  12

#define ParentRead inpipe_[0]
#define ParentWrite outpipe_[1]
#define ChildRead outpipe_[0]
#define ChildWrite inpipe_[1]

typedef enum {
  Sucess,
  FailPaccOpen,
  FailViscSet,
  FailIterSet
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
  void LoadFoilFile(char *fpath, char *foilname);

  /**
   * @brief Selects a NACA airfoil to input to xfoil
   * @param input 4-digit naca airfoil code
   */
  void NACA(const char code[5]);

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

  char outp_[OUTPUT_BUFF_SIZE];

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

  //!
  bool OutputContains(std::string substr);

  //! Loads NACA 1111 if no foil is currently loaded
  void LoadDummyFoil();

};

} // namespace cxxfoil

#endif // XFOIL_g
