#include "charmHpccg.h"
#include "generate_matrix.hpp"
#include "read_HPC_row.hpp"

#include <string>
#include <iostream>

using namespace std;

/*readonly*/ int numChares;
/*readonly*/ CProxy_charmMain mainProxy;

charmMain::charmMain(CkArgMsg* msg) {
  CkPrintf("charm HPCCG startup\n");
  fflush(stdout);
  
  mainProxy = thisProxy;

  int argc = msg->argc;
  char** argv = msg->argv;

  if(argc != 3 && argc!=5) {
    cerr << "Usage:" << endl
         << "Mode 1: " << argv[0] << " numChares nx ny nz" << endl
         << "     where nx, ny and nz are the local sub-block dimensions, or" << endl
         << "Mode 2: " << argv[0] << " numChares HPC_data_file " << endl
         << "     where HPC_data_file is a globally accessible file containing matrix data." << endl;
    CkExit();
  }

  numChares = atoi(argv[1]);
  CProxy_charmHpccg array = CProxy_charmHpccg::ckNew(numChares);

  if (argc == 5) {
    int nx = atoi(argv[2]);
    int ny = atoi(argv[3]);
    int nz = atoi(argv[4]);
    array.generateMatrix(nx, ny, nz);
  } else {
    array.readMatrix(argv[2]);
  }
  
  delete msg;
}

void charmMain::matrixReady() {
  CkExit();
}

void charmHpccg::generateMatrix(int nx, int ny, int nz) {
  generate_matrix(nx, ny, nz, &A, &x, &b, &xexact, numChares, thisIndex);
  contribute(CkCallback(CkReductionTarget(charmMain, matrixReady), mainProxy));
}

void charmHpccg::readMatrix(std::string fileName) {
  read_HPC_row(fileName.c_str(), &A, &x, &b, &xexact, numChares, thisIndex);
  contribute(CkCallback(CkReductionTarget(charmMain, matrixReady), mainProxy));
}

#include "charmHpccg.def.h"
