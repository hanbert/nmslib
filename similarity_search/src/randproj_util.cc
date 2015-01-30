/**
 * Non-metric Space Library
 *
 * Authors: Bilegsaikhan Naidan (https://github.com/bileg), Leonid Boytsov (http://boytsov.info).
 * With contributions from Lawrence Cayton (http://lcayton.com/) and others.
 *
 * For the complete list of contributors and further details see:
 * https://github.com/searchivarius/NonMetricSpaceLib 
 * 
 * Copyright (c) 2014
 *
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */

#include <cmath>
#include <random>
#include <vector>

#include "randproj_util.h"
#include "distcomp.h"
#include "logging.h"

namespace similarity {

using namespace std;

template <class dist_t> void initRandProj(size_t nSrcDim, size_t nDstDim,
                                         bool bDoOrth,
                                         vector<vector<dist_t>>& projMatr) {
  // Static is thread-safe in C++-11
  static  std::random_device          rd;
  static  std::mt19937                engine(rd());
  static  std::normal_distribution<>  normGen(0.0f, 1.0f);

  // 1. Create normally distributed vectors
  projMatr.resize(nDstDim);
  for (size_t i = 0; i < nDstDim; ++i) {
    projMatr[i].resize(nSrcDim);
    for (size_t j = 0; j < nSrcDim; ++j)
      projMatr[i][j] = normGen(engine);
  }
  /* 
   * 2. If bDoOrth == true, normalize the basis using the numerically stable
        variant of the Gram–Schmidt process (see Wikipedia for details 
        http://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process#Algorithm).
   *    Otherwise, just divide each vector by its norm. 
   */
  for (size_t i = 0; i < nDstDim; ++i) {
    if (bDoOrth) {
      // Normalize the outcome (in particular, to ensure the above mentioned invariant is true)
      dist_t normCoeff = sqrt(ScalarProductSIMD(&projMatr[i][0], &projMatr[i][0], nSrcDim));
      for (size_t n = 0; n < nSrcDim; ++n) projMatr[i][n] /= normCoeff;

      for (size_t k = i + 1; k < nDstDim; ++k) {
        dist_t coeff = ScalarProductSIMD(&projMatr[i][0], &projMatr[k][0], nSrcDim);
      /* 
       * Invariant the all previously processed vectors have been normalized.
       * Therefore, we we subtract the projection to a previous vector u,
       * we don't divide elements by the norm of the vector u
       */
        for (size_t n = 0; n < nSrcDim; ++n) projMatr[k][n] -= coeff * projMatr[i][n];
      }
    }
  }
}

template void initRandProj<float>(size_t nSrcDim, size_t nDstDim,
                                  bool bDoOrth,
                                  vector<vector<float>>& projMatr);
template void initRandProj<double>(size_t nSrcDim, size_t nDstDim,
                                  bool bDoOrth,
                                  vector<vector<double>>& projMatr);

template <class dist_t> void compProj(const vector<vector<dist_t>>& projMatr, 
                                      const dist_t* pSrcVect, size_t nSrcDim,
                                      dist_t* pDstVect, size_t nDstDim) {
  if (projMatr.empty()) LOG(LIB_FATAL) << "Bug: empty projection matrix";
  if (projMatr.size() != nDstDim) 
    LOG(LIB_FATAL) << "Bug: the # of rows in the projection matrix (" << projMatr.size() << ")"
               << " isn't equal to the number of vector elements in the target space "
               << "(" << nDstDim << ")";

  for (size_t i = 0; i < nDstDim; ++i) {
    if (projMatr[i].size() != nSrcDim) {
      LOG(LIB_FATAL) << "Bug: row index " << i << " the number of columns "
                 << "(" << projMatr[i].size() << ")"
                 << " isn't equal to the number of vector elements in the source space "
                 << "(" << nSrcDim << ")";
    }
    pDstVect[i] = ScalarProductSIMD(&projMatr[i][0], pSrcVect, nSrcDim);
  }
}

template void compProj<float>(const vector<vector<float>>& projMatr,
                              const float* pSrcVect, size_t nSrcDim,
                              float* pDstVect, size_t nDstQty);
template void compProj<double>(const vector<vector<double>>& projMatr,
                              const double* pSrcVect, size_t nSrcDim,
                              double* pDstVect, size_t nDstQty);
}
