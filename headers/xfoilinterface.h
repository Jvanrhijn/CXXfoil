/* Copyright (C) Jesse van Rhijn - All Rights Reserved
 *  * Unauthorized copying of this file, via any medium is strictly prohibited
 *  * Proprietary and confidential
 *  * Written by Jesse van Rhijn <jesse.v.rhijn@gmail.com>, November 2017
 */

/**
 * @file xfoilinterface.h
 * @author Jesse van Rhijn
 * @brief Library for calling XFoil from a C++ program.
 */
#ifndef XFOIL_H
#define XFOIL_H

#include "include.h"
#include "types.h"

#define PACC_FAIL (-1) //! Fail to open pacc file
#define VISC_FAIL (-2) //! Fail to set viscosity
#define ITER_FAIL (-3) //! Fail to set maximum iterations

#define INPUT_LOG

#define CMD_BUFF_SIZE 1024 // this is ugly and needs a better implementation
#define OUTPUT_BUFF_SIZE 200

#define SETTINGS_PROCESS_TIME 10

#define POLAR_DATA_LINENR  12

#define ParentRead inpipe[0]
#define ParentWrite outpipe[1]
#define ChildRead outpipe[0]
#define ChildWrite inpipe[1]

/**
 * @brief Class that interfaces with XFoil
 */
class XfoilInterface {

    private:
        FRIEND_TEST(XfoilTest, OutputTest);

        struct state {
            bool visc;
            bool G;
            bool foilloaded;
            bool pacc;
            unsigned int Reynolds;
            double Ncrit;
            unsigned int iter;
            std::string paccfile;
            std::string foilname;
        };

        //! struct containing xfoil settings current values
        state xfoilstate;

        //! Filestream to write into xfoil
        FILE* in;

        //! Filestream to get xfoil output
        FILE* out;

        //! pid of xfoil child process
        pid_t proc;

        //! input pipe file descriptors
        int inpipe[2];

        //! output pipe file descriptors
        int outpipe[2];

        //! mutex for locking output buffer
        std::mutex mutex1;

        //! buffer for xfoil last char output
        char outbuff;

        char outp[OUTPUT_BUFF_SIZE];

        //! xfoil log file
        std::ofstream log;

#ifdef INPUT_LOG
        std::ofstream inplog;
#endif

        //! thread for reading xfoil output
        std::thread reading;

        //! bool to stop output logging thread
        volatile bool logoutput;

        //! line number to read from polar file
        int linenr;

        //! Writes a single newline to xfoil
        void newline();

        //! Enable polar accumulation
        bool enablePACC(std::string paccfile);

        //! Disable polar accumulation
        void disablePACC();
        
        //! sets Ncrit
        void setNcrit(double Ncrit);

        //! Read value from location in polar file
        double readFromPolar(int linenr, size_t start, size_t end);

        //! Read line from polar file
        std::vector<double> readLineFromPolar(int linenr);

        //! let current thread sleep for milliseconds
        void wait_ms(unsigned int milliseconds);

        //! enter command into xfoil and flush buffer
        void command(const char* cmd, ...);

        //! Read xfoil output
        void readOutput();

        //! Checks whether Xfoil is currently waiting for input
        bool waitingForInput();

        //!
        bool outputContains(std::string substr);

        //! Loads NACA 1111 if no foil is currently loaded
        void loadDummyFoil();

    public:
        //! Constructor for XfoilInterface class
        explicit XfoilInterface(bool plot, double Ncrit = 9);

        //! Destructor for XfoilInterface class
        ~XfoilInterface();

        /**
         * @brief Starts xfoil interface
         * @return Whether XFoil was initialized succesfully
         */
        bool start();

        //! Configures xfoil with constructor parameters
        int configure();

        //! Quits xfoil
        bool quit();

        //! sets number of iterations
        bool setIter(unsigned int iterations);

        /**
         * @brief Loads airfoil coordinates from file
         * @param fpath File to load coordinates from
         * @param foilname Airfoil name
         */
        void loadFoilFile(char fpath[], char foilname[]);

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
        std::vector<double> angleOfAttack(double angle);

        /**
         * @brief Starts Xfoil analysis of sequence of angles
         * @param anglest starting angle of sequence
         * @param anglee ending angle of sequence
         * @param angleinc angle increment
         * @param liftcoeffs array to store results in
         * @returns polar file data structure
         */
        polar angleOfAttack(double anglest, double anglee, double angleinc);
        /**
         * @brief Starts Xfoil analysis of single lift coefficient
         * @param liftcoeff Lift coefficient to analyze
         * @returns vector containing calculation results, same order as in xfoil polar files
         */
        std::vector<double> liftCoefficient(double liftcoeff);

        /**
         * @brief Starts Xfoil analysis of sequence of lift coefficients
         * @param clstart starting lift coefficient
         * @param clend ending lift coefficient
         * @param clinc lift coefficient increment
         * @param angles array to store results in
         * @returns Polar file data structure
         */
        polar liftCoefficient(double clstart, double clend, double clinc);

        //! Enables viscous mode
        bool setViscosity(unsigned int Reynolds);

};

#endif
