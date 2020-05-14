/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcFFTWrap.h
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_FFT_WRAP_H_
#define ECMC_FFT_WRAP_H_

#include "ecmcPvDefs.h"

# ifdef __cplusplus
extern "C" {
# endif  // ifdef __cplusplus

  // void* getPvGetObj();
  // void* getPvPutObj();
  void*  getPvRegObj();
  double getData( double handle);
  double putData( double handle, double data);
  void   reset(   double handle);
  double getError(double handle);

# ifdef __cplusplus
}
# endif  // ifdef __cplusplus

#endif  /* ECMC_FFT_WRAP_H_ */
