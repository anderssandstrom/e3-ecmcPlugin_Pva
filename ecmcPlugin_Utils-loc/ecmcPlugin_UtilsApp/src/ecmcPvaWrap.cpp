/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPvaWrap.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN


#include "ecmcPvaWrap.h"
#include "ecmcPvRegFunc.h"

//  void* getPvGetObj() {
//    return (void*) new pvget<double>();
//  }

//  void* getPvPutObj() {
//    return (void*) new pvput<double>();
//  }

void* getPvRegObj() {
  return (void*) new pvreg<double>();
}

void reset(double handle) {
  try{
    pvVector.at(handle-1)->reset();
  }    
  catch(std::exception &e){
    std::cerr << "Error: " << e.what() << "\n";
    return;
  }
  return;
}

double getError(double handle) {
  try{
    return pvVector.at(handle-1)->getError();
  }    
  catch(std::exception &e){
    std::cerr << "Error: " << e.what() << "\n";
    return 0.0;
  }
  return 0.0;
}

// Normal plc functions
double getData(double handle) {
  try{
    return pvVector.at(handle-1)->get();
  }    
  catch(std::exception &e){
    std::cerr << "Error: " << e.what() << "\n";
    return 0.0;
  }
  return 0.0;
}

// Normal plc functions
double putData(double handle, double value) {
  try{
    pvVector.at((int)handle-1)->put(value);
    return 0.0;
  }    
  catch(std::exception &e){
    std::cerr << "Error: " << e.what() << "\n";
    return ECMC_PV_PUT_ERROR;
  }
  return ECMC_PV_PUT_ERROR;
}