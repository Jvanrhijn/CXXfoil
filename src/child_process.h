//
// Example of communication with a subprocess via stdin/stdout
// Author: Konstantin Tretyakov
// License: MIT
//
#ifndef _CXXFOIL_SRC_SPAWN
#define _CXXFOIL_SRC_SPAWN

#include <ext/stdio_filebuf.h> // NB: Specific /o libstdc++
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <exception>
#include <cstdlib>

namespace cxxfoil {

// Wrapping pipe in a class makes sure they are closed when we leave scope
class cpipe {
private:
    int fd[2];
public:
    inline int read_fd() const { return fd[0]; }
    inline int write_fd() const { return fd[1]; }
    cpipe() { if (pipe(fd)) throw std::runtime_error("Failed to create pipe"); }
    void close() { ::close(fd[0]); ::close(fd[1]); }
    ~cpipe() { close(); }
};


//
// Usage:
//   spawn s(argv)
//   s.stdin << ...
//   s.stdout >> ...
//   s.send_eol()
//   s.wait()
//
class spawn {
private:
    cpipe write_pipe;
    cpipe read_pipe;
public:

    std::ostream child_stdin;
    std::istream child_stdout;

    std::unique_ptr<__gnu_cxx::stdio_filebuf<char> > write_buf = NULL;
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char> > read_buf = NULL;
    int child_pid = -1;

    spawn(const char* const argv[], bool with_path = false, const char* const envp[] = 0)
      : child_stdin(NULL), child_stdout(NULL)
    {
        child_pid = fork();
        if (child_pid == -1) {
          throw std::runtime_error("Failed to start child process");
        }
        if (child_pid == 0) {   // In child process
            dup2(write_pipe.read_fd(), STDIN_FILENO);
            dup2(read_pipe.write_fd(), STDOUT_FILENO);
            write_pipe.close(); read_pipe.close();
            int result;

            if (with_path) {
                if (envp != 0) {
                  result = execve(argv[0],
                                   const_cast<char* const*>(argv),
                                   const_cast<char* const*>(envp));
                }
                else {
                  result = execvp(argv[0], const_cast<char* const*>(argv));
                }
            } else {
                if (envp != 0) {
                  result = execve(argv[0],
                                  const_cast<char* const*>(argv),
                                  const_cast<char* const*>(envp));
                } else {
                  result = system(argv[0]);
                }
            }
            if (result == -1) {
               // Note: no point writing to stdout here, it has been redirected
               std::cerr << "Error: Failed to launch program" << std::endl;
               exit(1);
            }
        }
        else {
            close(write_pipe.read_fd());
            close(read_pipe.write_fd());
            write_buf = std::unique_ptr<__gnu_cxx::stdio_filebuf<char> >(
                  new __gnu_cxx::stdio_filebuf<char>(write_pipe.write_fd(), std::ios::out)
                );
            read_buf = std::unique_ptr<__gnu_cxx::stdio_filebuf<char> >(
                  new __gnu_cxx::stdio_filebuf<char>(read_pipe.read_fd(), std::ios::in)
                );
            child_stdin.rdbuf(write_buf.get());
            child_stdout.rdbuf(read_buf.get());
        }
    }

    void send_eof() {
      write_buf->close();
    }

    int wait() {
        int status;
        waitpid(child_pid, &status, 0);
        return status;
    }
};

} // namespace cxxfoil

#endif // _CXXFOIL_SRC_SPAWN
