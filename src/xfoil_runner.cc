#include <sstream>
#include <iterator>
#include "child_process.h"
#include <iostream>

#include "xfoil_runner.h"
#include "except.h"

namespace cxxfoil {

XfoilRunner::XfoilRunner(std::string path,
                         std::vector<std::string> command_sequence,
                         std::string polar)
  : path_(std::move(path)),
    polar_(std::move(polar)),
    command_sequence_(std::move(command_sequence))
{}

polar XfoilRunner::Dispatch() const {
  const char* const argv[] = {path_.c_str(), (const char*) 0};
  spawn process(argv);

  for (const auto& cmd: command_sequence_) {
    process.child_stdin << cmd;
    process.child_stdin << '\n';
  }

  process.send_eof();
  int child_status = process.wait();

  std::string output;
  while (std::getline(process.child_stdout, output)) {
    if (output.find("VISCAL:  Convergence failed") != std::string::npos) {
      throw XfoilException("Xfoil failed to converge");
    }
    if (output.find("New polar save file OPEN error") != std::string::npos) {
      throw XfoilException("Xfoil failed to open polar save file");
    }
  }

  if (polar_.has_value()) {
    return ParsePolar(polar_.value());
  } else {
    return polar();
  }
}

polar XfoilRunner::ParsePolar(const std::string& path) const {
  polar table;
  std::vector<std::string> header = {"alpha", "CL", "CD", "CDp", "CM", "Top_Xtr", "Bot_Xtr", "Top_Itr", "Bot_Itr"};
  for (const auto &key: header) {
    table[key] = std::vector<double>();
  }
  // number of lines in polar before header
  constexpr size_t kHeader = 12;
  std::string line;
  std::ifstream file;
  file.open(path);
  size_t line_nr = 0;
  while (std::getline(file, line)) {
    // skip the header
    if (line_nr < kHeader) {
      line_nr++;
      continue;
    }
    // split the line by white spaces
    std::istringstream iss(line);
    std::vector<std::string> vec = std::vector<std::string>(std::istream_iterator<std::string>(iss),
                                                            std::istream_iterator<std::string>());
    for (size_t i=0; i<vec.size(); i++) {
      table[header[i]].push_back(std::stod(vec[i]));
    }

  }
  return table;
}

} // namespace cxxfoil
