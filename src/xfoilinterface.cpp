/* Copyright (C) Jesse van Rhijn - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Jesse van Rhijn <jesse.v.rhijn@gmail.com>, November 2017
 */

#include "xfoilinterface.h"


// contains class XfoilInterface, which interacts with xfoil via the command line
// the interface is instantiated in an empty state, optionally with parameters that set options in xfoil
// methods contained by the class should:
// * start xfoil from command line
// * load a file containing airfoil coordinates
// * set all different options in xfoil TODO
// * get pressure coeff, Cd, Cm, etc TODO

XfoilInterface::XfoilInterface(bool plot, double Ncrit) {
    xfoilstate.G = plot;
    xfoilstate.paccfile = "polarfile";
    xfoilstate.Ncrit = Ncrit;
    xfoilstate.pacc = false;
    linenr = POLAR_DATA_LINENR;
    xfoilstate.iter = 20;
    log.open("xfoil.log");
    logoutput = true;
#ifdef INPUT_LOG
    inplog.open("input.log");
#endif
}

XfoilInterface::~XfoilInterface() = default;

bool XfoilInterface::start() {
    if (!(pipe(inpipe) || pipe(outpipe))) {
        proc = fork();
        if (proc == 0) { /* child process */
            dup2(ChildRead, STDIN_FILENO);
            dup2(ChildWrite, STDOUT_FILENO);
            execl("/bin/xfoil", "xfoil", NULL);
            exit(1);
        } else if (proc == -1) { // fork failure
            return false;
        } else { /* parent process */
            in = fdopen(ParentWrite, "w");
            out = fdopen(ParentRead, "r");
            close(ChildRead);
            close(ChildWrite);
            reading = std::thread(&XfoilInterface::readOutput, this);
            return true;
        }
    }
}

int XfoilInterface::configure() {
    if (!xfoilstate.G) {
        command("plop\n");
        command("G\n");
        newline();
    }
    setNcrit(xfoilstate.Ncrit);
    if (!enablePACC(xfoilstate.paccfile))
        return PACC_FAIL;
    if (!setViscosity(xfoilstate.Reynolds))
        return VISC_FAIL;
    if (!setIter(xfoilstate.iter))
        return ITER_FAIL;
    newline();
    return 0;
}

bool XfoilInterface::quit() {
    newline();
    command("quit\n");
    logoutput = false;
    reading.detach();
    int stopped  = kill(proc, SIGTERM);
    int status;
    waitpid(proc, &status, 0);
    log.close();
    remove(xfoilstate.paccfile.c_str());
#ifdef INPUT_LOG
    inplog.close();
#endif
    if (stopped == 0) {
        return true;
    } else if (stopped == -1) {
        return false;
    }
}

void XfoilInterface::setNcrit(double Ncrit) {
    loadDummyFoil();
    command("oper\n");
    command("vpar\n");
    command("N %f\n", Ncrit);
    xfoilstate.Ncrit = Ncrit;
    newline(); newline();
}

void XfoilInterface::loadFoilFile(char fpath[], char foilname[]) {
    command("load %s\n", fpath);
    command("%s\n", foilname);
    xfoilstate.foilname = foilname;
}

void XfoilInterface::NACA(const char code[5]) {
    command("naca\n");
    command("%s\n", code);
    xfoilstate.foilloaded = true;
    xfoilstate.foilname = code;
}

bool XfoilInterface::setViscosity(unsigned int Reynolds) {
    bool success = false;
    loadDummyFoil();
    if (xfoilstate.pacc) {
        disablePACC();
    }
    if (Reynolds != 0) {
        command("oper\n");
        command("v\n");
        command("%d\n", Reynolds);
        newline();
        wait_ms(SETTINGS_PROCESS_TIME);
        if (outputContains("Re = ")) {
            xfoilstate.visc = true;
            success = true;
        }
    } else if (Reynolds == 0 && xfoilstate.visc) { // TODO resolve warning here
        command("oper\n");
        command("v\n");
        xfoilstate.visc = false;
        success = true;
    } else {
        success = true;
    }
    xfoilstate.Reynolds = Reynolds;
    if (!xfoilstate.pacc) {
        enablePACC(xfoilstate.paccfile);
    }
    return success;
}

std::vector<double> XfoilInterface::angleOfAttack(double angle) {
    std::vector<double> result;
    command("oper\n");
    std::thread execute([&](float alpha) {command("a %f\n", alpha); }, angle);
    execute.join();
    do {
        wait_ms(10);
    } while (!waitingForInput());
    result = readLineFromPolar(linenr);
    linenr++;
    if (outputContains("VISCAL:  Convergence failed")) {
        throw ConvergenceException();
    }
    return result;
}

polar XfoilInterface::angleOfAttack(double anglest, double anglee, double angleinc) {
    size_t len = 1 + (size_t) ceil((anglee - anglest) / angleinc);
    std::thread execute([&](float st, float e, float inc) {command("oper\n"); command("aseq\n"); command("%f\n", st); command("%f\n", e); command("%f\n", inc); }, anglest, anglee, angleinc);
    polar result(len);
    execute.join();
    do {
        wait_ms(10);
    } while(!waitingForInput());
    for (int i = 0; i < len; i++) {
        result.contents[i] = readLineFromPolar(linenr);
        linenr++;
    }
    if (outputContains("VISCAL:  Convergence failed")) {
        throw ConvergenceException();
    }
    newline();
    return result;
}

std::vector<double> XfoilInterface::liftCoefficient(double liftcoeff) {
    std::vector<double> result;
    std::thread execute([&](float cl) {command("oper\n"); command("cl %f\n", cl); }, liftcoeff);
    execute.join();
    do {
        wait_ms(10);
    } while (!waitingForInput());
    result = readLineFromPolar(linenr);
    linenr++;
    if (outputContains("VISCAL:  Convergence failed")) {
        throw ConvergenceException();
    }
    newline();
    return result;
}

polar XfoilInterface::liftCoefficient(double clstart, double clend, double clinc) {
    size_t len = 1 + (size_t) ceil((clend - clstart) / clinc);
    std::thread execute([&](float start, float end, float inc) {command("oper\n"); command("cseq\n"); command("%f\n", start); command("%f\n", end); command("%f\n", inc); }, clstart, clend, clinc);
    polar result(len);
    execute.join();
    do {
        wait_ms(10);
    } while (!waitingForInput());
    for (size_t i=0; i<len; i++) {
        result.contents[i] = readLineFromPolar(linenr);
        linenr++;
    }
    if (outputContains("VISCAL:  Convergence failed")) {
        throw ConvergenceException();
    }
    newline();
    return result;
}

bool XfoilInterface::enablePACC(std::string paccfile) {
    command("oper\n");
    command("pacc\n");
    command("%s\n", paccfile.c_str());
    wait_ms(SETTINGS_PROCESS_TIME);
    if (outputContains("different from old")) { // deals with the case where old save file values differ from new
        command("n\n");
        newline();
        command("pacc\n");
        remove(xfoilstate.paccfile.c_str());
        xfoilstate.paccfile += "0";
        command("pacc\n");
        command("%s\n", xfoilstate.paccfile.c_str());
        linenr = POLAR_DATA_LINENR;
    }
    newline();
    wait_ms(SETTINGS_PROCESS_TIME);
    if (outputContains("Polar accumulation enabled")) {
        newline();
        xfoilstate.pacc = true;
        return true;
    }
    newline();
    return false;
}

void XfoilInterface::disablePACC() {
    command("oper\n");
    command("pacc\n");
    newline();
    xfoilstate.pacc = false;
}

bool XfoilInterface::setIter(unsigned int iterations) {
    command("oper\n");
    command("iter\n");
    command("%d\n", iterations);
    wait_ms(SETTINGS_PROCESS_TIME);
    newline();
    if (outputContains("iteration")) {
        xfoilstate.iter = iterations;
        return true;
    }
    return false;
}

/*
 * LOW LEVEL CODE
 */
double XfoilInterface::readFromPolar(int linenr, size_t start, size_t end) {
    int i = 0; std::ifstream polar; std::string linebuf; std::string valuestr;
    polar.open(xfoilstate.paccfile.c_str());
    while(getline(polar, linebuf)) {
        if (i == linenr) {
            valuestr = linebuf.substr(start, end);
            break;
        }
        i++;
    }
    double value = strtod(valuestr.c_str(), nullptr);
    polar.close();
    return value;
}

std::vector<double> XfoilInterface::readLineFromPolar(int linenr) {
    int i = 0; std::ifstream polar; std::string linebuf;
    polar.open(xfoilstate.paccfile.c_str());
    std::vector<double> line;
    while(getline(polar, linebuf)) {
        if (i == linenr) {
            std::string alpha = linebuf.substr(2, 8);
            std::string CL = linebuf.substr(10, 8);
            std::string CD = linebuf.substr(20, 7);
            std::string CDp = linebuf.substr(29, 8);
            std::string CM = linebuf.substr(39, 8);
            line = {strtod(alpha.c_str(), nullptr), strtod(CL.c_str(), nullptr), strtod(CD.c_str(), nullptr), strtod(CDp.c_str(),
                                                                                                                     nullptr), strtod(CM.c_str(),
                                                                                                                                      nullptr)};
        }
        i++;
    }
    return line;
}

void XfoilInterface::wait_ms(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void XfoilInterface::readOutput() {
    outp[OUTPUT_BUFF_SIZE-1] = '\0';
    while (logoutput) {
        outbuff = (char) fgetc(out);
        log << outbuff;
        mutex1.lock();
        for (int i = 1; i<OUTPUT_BUFF_SIZE-1; i++) { // shift output buffer
            outp[i-1] = outp[i];
        }
        outp[OUTPUT_BUFF_SIZE-2] = outbuff;
        mutex1.unlock();
    }
}

bool XfoilInterface::waitingForInput() {
    mutex1.lock();
    std::string output_buffer(outp);
    std::string last4 = output_buffer.substr(OUTPUT_BUFF_SIZE-5);
    mutex1.unlock();
    return last4 == "c>  ";
}

void XfoilInterface::command(const char* cmd, ...) {
    char buf[CMD_BUFF_SIZE];
    va_list vl;
    va_start(vl, cmd);
    vsnprintf(buf, sizeof(buf), cmd, vl);
    va_end(vl);
    inplog << buf;
    fprintf(in, buf);
    fflush(in);

}

void XfoilInterface::newline() {
    command("\n");
}

void XfoilInterface::loadDummyFoil() {
    if (!xfoilstate.foilloaded) {
        NACA("1111");
        xfoilstate.foilloaded = true;
    }
}

bool XfoilInterface::outputContains(std::string substr) {
    mutex1.lock();
    std::string outpstr(outp);
    mutex1.unlock();
    return (outpstr.find(substr) != (std::string::npos));
}