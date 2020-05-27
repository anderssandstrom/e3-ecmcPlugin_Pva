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

pvreg<double>*  pvRegObj;
int maxPvs = ECMC_MAX_PVS_DEFAULT;

// Pre allocate objects at construct to minimize time jitter in runtime
int initPvs() {
  try{
    for(int i = 0; i < maxPvs; ++i ) {
      ecmcPvPtr pv = ecmcPv::create("DummyName","DummyProvider","value",i+1);
      pvVector.push_back(pv);
    }
  }
  catch(std::exception &e){
    std::cerr << "Error:  init: " << e.what() << "\n";
    return ECMC_PV_INIT_ERROR;
  }
  return 0;
}

void* getPvRegObj() {
  pvRegObj = new pvreg<double>();
  return (void*) pvRegObj;
}

int getError(int handle) {
  try{
    return pvVector.at(handle-1)->getError();
  }    
  catch(std::exception &e){
    std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_GET_ERR "(): " << e.what() << "\n";
    return 0.0;
  }
  return 0.0;
}  

// Normal plc functions
int exePutDataCmd(int handle, double value) {
  try{
    pvVector.at(handle-1)->putCmd(value);
    return 0;
  }    
  catch(std::exception &e){
    std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_PUT_ASYN "(): " << e.what() << "\n";
    return ECMC_PV_PUT_ERROR;
  }
  return ECMC_PV_PUT_ERROR;
}

double getLastValue(int handle) {
  try{
    return pvVector.at(handle-1)->getLastReadValue();
  }    
  catch(std::exception &e){
    std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_GET_VALUE "(): "<< e.what() << "\n";
    return 0;
  }
  return 0;
}

int getBusy(int handle) {
  try{
    return pvVector.at(handle-1)->busy();
  }    
  catch(std::exception &e){
    std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_GET_BUSY "(): "<< e.what() << "\n";
    return 0;
  }
  return 0;
}

int getConnected(int handle) {
  try{
    return pvVector.at(handle-1)->connected();
  }    
  catch(std::exception &e){
    std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_GET_BUSY "(): "<< e.what() << "\n";
    return 0;
  }
  return 0;
}

void cleanup() {
 try{
    pvVector.clear();
    delete pvRegObj;
  }    
  catch(std::exception &e){
    std::cerr << "Error: Destruct(): "<< e.what() << "\n";
    return;
  }
}
