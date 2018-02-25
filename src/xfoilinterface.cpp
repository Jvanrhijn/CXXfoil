#include "xfoilinterface.h"

// contains class Xfoil, which interacts with xfoil via the Command line
// the interface is instantiated in an empty state, optionally with parameters that set options in xfoil
// methods contained by the class should:
// * Start xfoil from Command line
// * load a file containing airfoil coordinates
// * set all different options in xfoil TODO
// * get pressure coeff, Cd, Cm, etc TODO

Xfoil::Xfoil() {
  xfoil_state.G = false;
  xfoil_state.pacc_file = std::tmpnam(nullptr);
  xfoil_state.Ncrit = 9;
  xfoil_state.pacc = false;
  line_number = POLAR_DATA_LINENR;
  xfoil_state.iter = 20;
  log.open("xfoil.log");
  log_output = true;
  input_log.open("input.log");
  Start();
  Configure();
}

Xfoil::~Xfoil() {
  Quit();
}

bool Xfoil::Start() {
  if (!(pipe(inpipe) || pipe(outpipe))) {
    process = fork();
    if (process == 0) { /* child process */
      dup2(ChildRead, STDIN_FILENO);
      dup2(ChildWrite, STDOUT_FILENO);
      execl("/bin/xfoil", "xfoil", NULL);
      exit(1);
    } else if (process == -1) { // fork failure
      return false;
    } else { /* parent process */
      input = fdopen(ParentWrite, "w");
      output = fdopen(ParentRead, "r");
      close(ChildRead);
      close(ChildWrite);
      reading = std::thread(&Xfoil::ReadOutput, this);
      return true;
    }
  }
}

int Xfoil::Configure() {
  if (!xfoil_state.G) {
    Command("plop\n");
    Command("G\n");
    Newline();
  }
  SetNcrit(xfoil_state.Ncrit);
  if (!EnablePACC(xfoil_state.pacc_file))
    return PACC_FAIL;
  if (!SetViscosity(xfoil_state.Reynolds))
    return VISC_FAIL;
  if (!SetIterations(xfoil_state.iter))
    return ITER_FAIL;
  Newline();
  return 0;
}

bool Xfoil::Quit() {
  Newline();
  Command("Quit\n");
  log_output = false;
  reading.detach();
  int stopped  = kill(process, SIGTERM);
  int status;
  waitpid(process, &status, 0);
  log.close();
  remove(xfoil_state.pacc_file.c_str());
  input_log.close();
  if (stopped == 0) {
    return true;
  } else if (stopped == -1) {
    return false;
  }
}

void Xfoil::SetNcrit(double Ncrit) {
  LoadDummyFoil();
  Command("oper\n");
  Command("vpar\n");
  Command("N %f\n", Ncrit);
  xfoil_state.Ncrit = Ncrit;
  Newline();
  Newline();
}

void Xfoil::LoadFoilFile(char *fpath, char *foilname) {
  Command("load %s\n", fpath);
  Command("%s\n", foilname);
  xfoil_state.foil_name = foilname;
}

void Xfoil::NACA(const char code[5]) {
  Command("naca\n");
  Command("%s\n", code);
  xfoil_state.foil_loaded = true;
  xfoil_state.foil_name = code;
}

bool Xfoil::SetViscosity(unsigned int Reynolds) {
  bool success = false;
  LoadDummyFoil();
  if (xfoil_state.pacc) {
    DisablePACC();
  }
  if (Reynolds != 0) {
    Command("oper\n");
    Command("v\n");
    Command("%d\n", Reynolds);
    Newline();
    wait_ms(SETTINGS_PROCESS_TIME);
    if (OutputContains("Re = ")) {
      xfoil_state.viscous = true;
      success = true;
    }
  } else if (Reynolds == 0 && xfoil_state.viscous) { // TODO resolve warning here
    Command("oper\n");
    Command("v\n");
    xfoil_state.viscous = false;
    success = true;
  } else {
    success = true;
  }
  xfoil_state.Reynolds = Reynolds;
  if (!xfoil_state.pacc) {
    EnablePACC(xfoil_state.pacc_file);
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
  result = ReadLineFromPolar(line_number);
  line_number++;
  if (OutputContains("VISCAL:  Convergence failed")) {
    throw ConvergenceException();
  }
  return result;
}

polar Xfoil::AngleOfAttack(double angle_start, double angle_end, double angle_increment) {
  size_t len = 1 + (size_t) ceil((angle_end - angle_start) / angle_increment);
  std::thread execute([&](float st, float e, float inc) {
    Command("oper\n");
    Command("aseq\n");
    Command("%f\n", st);
    Command("%f\n", e);
    Command("%f\n", inc); }, angle_start, angle_end, angle_increment);
  polar result(len);
  execute.join();
  do {
      wait_ms(10);
  } while(!WaitingForInput());
  for (int i = 0; i < len; i++) {
    result.contents[i] = ReadLineFromPolar(line_number);
    line_number++;
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
    Command("cl %f\n", cl); }, lift_coefficient);
  execute.join();
  do {
    wait_ms(10);
  } while (!WaitingForInput());
  result = ReadLineFromPolar(line_number);
  line_number++;
  if (OutputContains("VISCAL:  Convergence failed")) {
    throw ConvergenceException();
  }
  Newline();
  return result;
}

polar Xfoil::LiftCoefficient(double cl_start, double cl_end, double cl_increment) {
  size_t len = 1 + (size_t) ceil((cl_end - cl_start) / cl_increment);
  std::thread execute([&](float start, float end, float inc) {
    Command("oper\n");
    Command("cseq\n");
    Command("%f\n", start);
    Command("%f\n", end);
    Command("%f\n", inc); }, cl_start, cl_end, cl_increment);
  polar result(len);
  execute.join();
  do {
    wait_ms(10);
  } while (!WaitingForInput());
  for (size_t i=0; i<len; i++) {
    result.contents[i] = ReadLineFromPolar(line_number);
    line_number++;
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
  wait_ms(SETTINGS_PROCESS_TIME);
  if (OutputContains("different from old")) { // deals with the case where old save file values differ from new
    Command("n\n");
    Newline();
    Command("pacc\n");
    remove(xfoil_state.pacc_file.c_str());
    xfoil_state.pacc_file = std::tmpnam(nullptr);
    Command("pacc\n");
    Command("%s\n", xfoil_state.pacc_file.c_str());
    line_number = POLAR_DATA_LINENR;
  }
  Newline();
  wait_ms(SETTINGS_PROCESS_TIME);
  if (OutputContains("Polar accumulation enabled")) {
    Newline();
    xfoil_state.pacc = true;
    return true;
  }
  Newline();
  return false;
}

void Xfoil::DisablePACC() {
  Command("oper\n");
  Command("pacc\n");
  Newline();
  xfoil_state.pacc = false;
}

bool Xfoil::SetIterations(unsigned int iterations) {
  Command("oper\n");
  Command("iter\n");
  Command("%d\n", iterations);
  wait_ms(SETTINGS_PROCESS_TIME);
  Newline();
  if (OutputContains("iteration")) {
    xfoil_state.iter = iterations;
    return true;
  }
  return false;
}

/*
 * LOW LEVEL CODE
 */
double Xfoil::ReadFromPolar(int linenr, size_t start, size_t end) {
  int i = 0; std::ifstream polar_file; std::string linebuf; std::string valuestr;
  polar_file.open(xfoil_state.pacc_file.c_str());
  while(getline(polar_file, linebuf)) {
    if (i == linenr) {
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
  int i = 0; std::ifstream polar_file; std::string linebuf;
  polar_file.open(xfoil_state.pacc_file.c_str());
  std::vector<double> line;
  while(getline(polar_file, linebuf)) {
    if (i == linenr) {
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
  outp[OUTPUT_BUFF_SIZE-1] = '\0';
  while (log_output) {
    output_buffer = (char) fgetc(output);
    log << output_buffer;
    mutex_output.lock();
    for (int i = 1; i<OUTPUT_BUFF_SIZE-1; i++) { // shift output buffer
        outp[i-1] = outp[i];
    }
    outp[OUTPUT_BUFF_SIZE-2] = output_buffer;
    mutex_output.unlock();
  }
}

bool Xfoil::WaitingForInput() {
  mutex_output.lock();
  std::string output_buffer(outp);
  mutex_output.unlock();
  std::string last_four = output_buffer.substr(OUTPUT_BUFF_SIZE-5);
  return last_four == "c>  ";
}

void Xfoil::Command(const char *cmd, ...) {
  char buf[CMD_BUFF_SIZE];
  va_list vl;
  va_start(vl, cmd);
  vsnprintf(buf, sizeof(buf), cmd, vl);
  va_end(vl);
  input_log << buf;
  fprintf(input, buf);
  fflush(input);
}

void Xfoil::Newline() {
  Command("\n");
}

void Xfoil::LoadDummyFoil() {
  if (!xfoil_state.foil_loaded) {
    NACA("1111");
    xfoil_state.foil_loaded = true;
  }
}

bool Xfoil::OutputContains(std::string substr) {
  mutex_output.lock();
  std::string output_string(outp);
  mutex_output.unlock();
  return (output_string.find(substr) != (std::string::npos));
}
