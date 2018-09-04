#include "cxxfoil.h"

namespace cxxfoil {

Xfoil::Xfoil(const std::string &path, bool log) {
  xfoil_state_.G = false;
  xfoil_state_.pacc_file = std::tmpnam(nullptr);
  xfoil_state_.Ncrit = 9;
  xfoil_state_.pacc = false;
  xfoil_state_.viscous = false;
  line_number_ = kPolarLineNr;
  xfoil_state_.iter = 20;
  read_output_ = true;
  if (log) {
    log_.open("xfoil.log");
    log_output_ = true;
    input_log_.open("input.log");
  }
  Start(path);
  do {
    wait_ms(kSettingsProcessTime);
  } while (!WaitingForInput());
  Configure();
}

Xfoil::~Xfoil() {
  Quit();
}

bool Xfoil::Start(const std::string &path) {
  if (!(pipe(inpipe_) || pipe(outpipe_))) {
    process_ = fork();
    if (process_==0) { /* child process_ */
      dup2(outpipe_[0], STDIN_FILENO);
      dup2(inpipe_[1], STDOUT_FILENO);
      execl(path.c_str(), "xfoil", NULL);
      exit(1);
    } else if (process_==-1) { // fork failure
      return false;
    } else { /* parent process_ */
      input_ = fdopen(outpipe_[1], "w");
      output_ = fdopen(inpipe_[0], "r");
      if (!input_ || !output_)
        return false;
      close(outpipe_[0]);
      close(inpipe_[1]);
      reading_ = std::thread(&Xfoil::ReadOutput, this);
      return true;
    }
  }
  return false;
}

XfoilError Xfoil::Configure() {
  if (!xfoil_state_.G) {
    Command("plop\n");
    Command("G\n");
    Newline();
  }
  SetNcrit(xfoil_state_.Ncrit);
  if (!EnablePACC(xfoil_state_.pacc_file))
    return FailPaccOpen;
  if (!SetViscosity(xfoil_state_.Reynolds))
    return FailViscSet;
  if (!SetIterations(xfoil_state_.iter))
    return FailIterSet;
  Newline();
  return Success;
}

bool Xfoil::Quit() {
  Newline();
  Command("Quit\n");
  if (log_output_) {
    log_.close();
    log_output_ = false;
  }
  read_output_ = false;
  reading_.detach();
  int stopped = kill(process_, SIGTERM);
  int status;
  waitpid(process_, &status, 0);
  remove(xfoil_state_.pacc_file.c_str());
  fclose(input_);
  fclose(output_);
  if (log_output_)
    input_log_.close();
  std::remove(":00.bl");
  return stopped == 0;
}

void Xfoil::SetNcrit(double Ncrit) {
  LoadDummyFoil();
  Command("oper\n");
  Command("vpar\n");
  Command("N %f\n", Ncrit);
  xfoil_state_.Ncrit = Ncrit;
  Newline();
  Newline();
}

XfoilError Xfoil::LoadFoilFile(const std::string& fpath, const std::string& foilname) {
  Command("load %s\n", fpath.c_str());
  wait_ms(kSettingsProcessTime);
  if (OutputContains("LOAD NOT COMPLETED") && WaitingForInput())
    return FailDatFile;
  Command("%s\n", foilname.c_str());
  xfoil_state_.foil_name = foilname;
  return Success;
}

XfoilError Xfoil::NACA(const char code[5]) {
  Command("naca\n");
  Command("%s\n", code);
  wait_ms(kSettingsProcessTime);
  if (OutputContains("not implemented") || !WaitingForInput()) {
    Newline();
    return FailNaca;
  }
  xfoil_state_.foil_loaded = true;
  xfoil_state_.foil_name = code;
  return Success;
}

XfoilError Xfoil::SetViscosity(unsigned int Reynolds) {
  XfoilError success = FailViscSet;
  LoadDummyFoil();
  if (xfoil_state_.pacc) {
    DisablePACC();
  }
  if (Reynolds != 0 && !xfoil_state_.viscous) {
    Command("oper\n");
    Command("v\n");
    Command("%d\n", Reynolds);
    Newline();
    wait_ms(kSettingsProcessTime);
    if (OutputContains("Re = ")) {
      xfoil_state_.viscous = true;
      success = Success;
    }
  } else if (Reynolds != 0 && xfoil_state_.viscous) {
    Command("r\n");
    Command("%d\n", Reynolds);
    Newline();
    wait_ms(kSettingsProcessTime);
    if (OutputContains("OPERv"))
      success = Success;
  } else if (Reynolds == 0 && xfoil_state_.viscous) {
    Command("oper\n");
    Command("v\n");
    xfoil_state_.viscous = false;
    success = Success;
  } else {
    success = FailViscSet;
  }
  xfoil_state_.Reynolds = Reynolds;
  if (!xfoil_state_.pacc) {
    EnablePACC(xfoil_state_.pacc_file);
  }
  return success;
}

std::vector<double> Xfoil::AngleOfAttack(double angle) {
  std::vector<double> result;
  Command("oper\n");
  std::thread execute([&](float alpha) { Command("a %f\n", alpha); }, angle);
  execute.join();
  do {
    wait_ms(10);
  } while (!WaitingForInput());
  result = ReadLineFromPolar(line_number_);
  line_number_++;
  if (OutputContains("VISCAL:  Convergence failed")) {
    throw ConvergenceException();
  }
  Newline();
  return result;
}

polar Xfoil::AngleOfAttack(double angle_start, double angle_end, double angle_increment) {
  size_t len = 1 + (size_t) ceil((angle_end - angle_start)/angle_increment);
  std::thread execute([&](float st, float e, float inc) {
    Command("oper\n");
    Command("aseq\n");
    Command("%f\n", st);
    Command("%f\n", e);
    Command("%f\n", inc);
  }, angle_start, angle_end, angle_increment);
  polar result(len);
  execute.join();
  do {
    wait_ms(10);
  } while (!WaitingForInput());
  for (int i = 0; i < len; i++) {
    result.contents[i] = ReadLineFromPolar(line_number_);
    line_number_++;
  }
  if (OutputContains("VISCAL:  Convergence failed")) {
    throw ConvergenceException();
  }
  Newline();
  return result;
}

std::vector<double> Xfoil::LiftCoefficient(double lift_coefficient) {
  std::vector<double> result;
  std::thread execute([&](float cl) {
    Command("oper\n");
    Command("cl %f\n", cl);
  }, lift_coefficient);
  execute.join();
  do {
    wait_ms(10);
  } while (!WaitingForInput());
  result = ReadLineFromPolar(line_number_);
  line_number_++;
  if (OutputContains("VISCAL:  Convergence failed")) {
    throw ConvergenceException();
  }
  Newline();
  return result;
}

polar Xfoil::LiftCoefficient(double cl_start, double cl_end, double cl_increment) {
  size_t len = 1 + (size_t) ceil((cl_end - cl_start)/cl_increment);
  std::thread execute([&](float start, float end, float inc) {
    Command("oper\n");
    Command("cseq\n");
    Command("%f\n", start);
    Command("%f\n", end);
    Command("%f\n", inc);
  }, cl_start, cl_end, cl_increment);
  polar result(len);
  execute.join();
  do {
    wait_ms(10);
  } while (!WaitingForInput());
  for (size_t i = 0; i < len; i++) {
    result.contents[i] = ReadLineFromPolar(line_number_);
    line_number_++;
  }
  if (OutputContains("VISCAL:  Convergence failed")) {
    throw ConvergenceException();
  }
  Newline();
  return result;
}

bool Xfoil::EnablePACC(std::string paccfile) {
  Command("oper\n");
  Command("pacc\n");
  Command("%s\n", paccfile.c_str());
  wait_ms(kSettingsProcessTime);
  if (OutputContains("different from old")) { // deals with the case where old save file values differ from new
    Command("n\n");
    Newline();
    Command("pacc\n");
    remove(xfoil_state_.pacc_file.c_str());
    xfoil_state_.pacc_file = std::tmpnam(nullptr);
    Command("pacc\n");
    Command("%s\n", xfoil_state_.pacc_file.c_str());
    line_number_ = kPolarLineNr;
  }
  Newline();
  wait_ms(kSettingsProcessTime);
  if (OutputContains("Polar accumulation enabled")) {
    Newline();
    xfoil_state_.pacc = true;
    return true;
  }
  Newline();
  return false;
}

void Xfoil::DisablePACC() {
  Command("oper\n");
  Command("pacc\n");
  Newline();
  xfoil_state_.pacc = false;
}

XfoilError Xfoil::SetIterations(unsigned int iterations) {
  Command("oper\n");
  Command("iter\n");
  Command("%d\n", iterations);
  wait_ms(kSettingsProcessTime);
  Newline();
  if (OutputContains("iteration")) {
    xfoil_state_.iter = iterations;
    return Success;
  }
  return FailIterSet;
}

std::vector<std::tuple<double, double>> Xfoil::PressureDistribution(double value, const std::string &type) {
  const std::string fname = std::tmpnam(nullptr);
  Newline();
  if (type == "aoa")
    AngleOfAttack(value);
  else if (type == "cl")
    LiftCoefficient(value);
  else
    throw std::runtime_error("Unsupported calculation type in xfoil::PressureDistribution (must be 'aoa' or 'cl')");
  Command("oper\n");
  Command("cpwr %s\n", fname.c_str());
  do {
    wait_ms(10);
  } while (!WaitingForInput());
  Newline();
  return ReadPressureFile(fname);
}


/*
 * LOW LEVEL CODE
 */
std::vector<std::tuple<double, double>> Xfoil::ReadPressureFile(const std::string &fname) {
  std::ifstream cp_file;
  cp_file.open(fname.c_str());
  if (!cp_file)
    throw std::runtime_error("Could not open cp_file");
  cp_file.ignore(256, '\n');
  std::string linebuf;
  std::vector<std::tuple<double, double>> result;
  while (getline(cp_file, linebuf)) {
    double x = std::stod(linebuf.substr(5, 7));
    double cp = std::stod(linebuf.substr(16, 7));
    if (linebuf.substr(15, 1) == std::string(1, '-'))
      cp *= -1;
    result.emplace_back(std::make_tuple(x, cp));
  }
  return result;
}

double Xfoil::ReadFromPolar(int linenr, size_t start, size_t end) {
  int i = 0;
  std::ifstream polar_file;
  std::string linebuf;
  std::string valuestr;
  polar_file.open(xfoil_state_.pacc_file.c_str());
  if (!polar_file)
    throw std::runtime_error("Could not open polar file");
  while (getline(polar_file, linebuf)) {
    if (i==linenr) {
      valuestr = linebuf.substr(start, end);
      break;
    }
    i++;
  }
  double value = strtod(valuestr.c_str(), nullptr);
  polar_file.close();
  return value;
}

std::vector<double> Xfoil::ReadLineFromPolar(int linenr) {
  int i = 0;
  std::ifstream polar_file;
  std::string linebuf;
  polar_file.open(xfoil_state_.pacc_file.c_str());
  if (!polar_file)
    throw std::runtime_error("Could not open polar file");
  std::vector<double> line;
  while (getline(polar_file, linebuf)) {
    if (i==linenr) {
      std::string alpha = linebuf.substr(2, 8);
      std::string CL = linebuf.substr(10, 8);
      std::string CD = linebuf.substr(20, 7);
      std::string CDp = linebuf.substr(29, 8);
      std::string CM = linebuf.substr(39, 8);
      line = {strtod(alpha.c_str(), nullptr), strtod(CL.c_str(), nullptr),
              strtod(CD.c_str(), nullptr), strtod(CDp.c_str(), nullptr),
              strtod(CM.c_str(), nullptr)};
    }
    i++;
  }
  return line;
}

void Xfoil::wait_ms(unsigned int milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Xfoil::ReadOutput() {
  outp_[kOutputBufferSize- 1] = '\0';
  while (read_output_) {
    output_buffer_ = (char) fgetc(output_);
    mutex_output_.lock();
    for (int i = 1; i < kOutputBufferSize - 1; i++) { // shift output buffer
      outp_[i - 1] = outp_[i];
    }
    outp_[kOutputBufferSize - 2] = output_buffer_;
    mutex_output_.unlock();
  }
}

bool Xfoil::WaitingForInput() {
  mutex_output_.lock();
  std::string output_buffer(outp_);
  mutex_output_.unlock();
  std::string last_four = output_buffer.substr(kOutputBufferSize - 5);
  return last_four=="c>  ";
}

void Xfoil::Command(const char *cmd, ...) {
  char buf[kCommandBufferSize];
  va_list vl;
  va_start(vl, cmd);
  vsnprintf(buf, sizeof(buf), cmd, vl);
  va_end(vl);
  if (log_output_)
    input_log_ << buf;
  fprintf(input_, buf);
  fflush(input_);
}

void Xfoil::Newline() {
  Command("\n");
}

void Xfoil::LoadDummyFoil() {
  if (!xfoil_state_.foil_loaded) {
    NACA("1111");
    xfoil_state_.foil_loaded = true;
  }
}

bool Xfoil::OutputContains(std::string substr) {
  mutex_output_.lock();
  std::string output_string(outp_);
  mutex_output_.unlock();
  return (output_string.find(substr)!=(std::string::npos));
}

} // namespace cxxfoil
