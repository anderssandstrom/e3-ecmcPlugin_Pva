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

  void*  getPvRegObj();
  int    exeGetDataCmd(int handle);
  int    exePutDataCmd(int handle, double data);
  double getLastValue(int handle);
  int    getBusy(int handle);
  int    getConnected(int handle);
  void   resetError(int handle);
  int    getError(int handle);
  void   cleanup();

# ifdef __cplusplus
}
# endif  // ifdef __cplusplus

#endif  /* ECMC_FFT_WRAP_H_ */
