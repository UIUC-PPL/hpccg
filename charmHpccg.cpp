#include "charmHpccg.h"
#include "generate_matrix.hpp"
#include "read_HPC_row.hpp"
#include "make_local_matrix.hpp"

#include <string>
#include <iostream>

using namespace std;

/*readonly*/ int numChares;
/*readonly*/ CProxy_charmMain mainProxy;
/*readonly*/ CProxy_CompletionDetector detector;

charmMain::charmMain(CkArgMsg* msg) {
  __sdag_init();
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

  detector = CProxy_CompletionDetector::ckNew();

  numChares = atoi(argv[1]);
  array = CProxy_charmHpccg::ckNew(numChares);

  if (argc == 5) {
    int nx = atoi(argv[2]);
    int ny = atoi(argv[3]);
    int nz = atoi(argv[4]);
    array.generateMatrix(nx, ny, nz);
  } else {
    array.readMatrix(argv[2]);
  }
  
  delete msg;

  thisProxy.start();
}

void charmHpccg::generateMatrix(int nx, int ny, int nz) {
  generate_matrix(nx, ny, nz, &A, &x, &b, &xexact, numChares, thisIndex);
  contribute(CkCallback(CkReductionTarget(charmMain, matrixReady), mainProxy));
}

void charmHpccg::readMatrix(std::string fileName) {
  read_HPC_row(fileName.c_str(), &A, &x, &b, &xexact, numChares, thisIndex);
  contribute(CkCallback(CkReductionTarget(charmMain, matrixReady), mainProxy));
}

void charmHpccg::findExternals() {
  map<int, int> externals;
  identify_externals(A, externals);

  for (map<int, int>::iterator iter = externals.begin(); iter != externals.end(); ++iter) {
    int row = iter->first;
    int chunksize = A->total_nrow / numChares;
    int remainder = A->total_nrow % numChares;
    int cutoffRow = (chunksize + 1) * remainder;

    int chare;
    int remoteRow;
    if (row < cutoffRow) {
      chare     = row / (chunksize + 1);
      remoteRow = row % (chunksize + 1);
    } else {
      int rowsPastCutoff = row - cutoffRow;
      chare = remainder + rowsPastCutoff / chunksize;
      remoteRow = rowsPastCutoff % chunksize;
    }

    xToReceive[chare].values.push_back(remoteRow);
  }

  int offset = 0;
  for(map<int, RemoteX>::iterator iter = xToReceive.begin();
      iter != xToReceive.end(); ++iter) {
    iter->second.offset = offset;
    offset += iter->second.values.size();
    thisProxy[iter->first].needXElements(thisIndex, iter->second.values);
  }

  detector.ckLocalBranch()->produce(xToReceive.size());
  detector.ckLocalBranch()->done();
}

void charmHpccg::needXElements(int requester, vector<int> rows) {
  xToSend[requester].values = rows;
  detector.ckLocalBranch()->consume();
}

#include "charmHpccg.def.h"
