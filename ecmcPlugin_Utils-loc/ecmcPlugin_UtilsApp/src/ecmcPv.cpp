/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPv.cpp
*
*  Created on: May 12, 2020
*      Author: anderssandstrom
*
\*************************************************************************/
#include "ecmcPv.h"
#include <sstream>
#include "epicsThread.h"

// Start worker threads for each object
void f_worker(void *obj) {
  if(!obj) {
    printf("%s/%s:%d: Error: Worker thread PV object NULL..\n",
            __FILE__, __FUNCTION__, __LINE__);
    return;
  }

  ecmcPv * pvObj = (ecmcPv*)obj;
  pvObj->exeCmdThread();
}

ecmcPv::ecmcPv(std::string pvName,std::string providerName, int index) {
  name_            = pvName;
  providerName_    = providerName;
  errorCode_       = 0;
  destructs_       = 0;
  cmd_             = ECMC_PV_CMD_NONE;
  //ATOMIC_FLAG_INIT(busyLock_);
  busyLock_.test_and_set();
  index_           = index;
  valueLatestRead_ = 0;
  valueToWrite_    = 0;

  // Create worker thread
  std::string threadname = "ecmc.plugin.utils.pva_"  + to_string(index_);
  if(epicsThreadCreate(threadname.c_str(), 0, 32768, f_worker, this) == NULL) {
    throw std::runtime_error("Error: Failed create worker thread.");
  }
}

ecmcPv::~ecmcPv() {
  doCmdEvent_.signal();
  destructs_ = 1;
}

int ecmcPv::getError() {
  return errorCode_;
}

int ecmcPv::reset() {
  errorCode_ = 0;
  return 0;
}

void ecmcPv::get() {

  if(errorCode_ == ECMC_PV_GET_ERROR || errorCode_ == ECMC_PV_BUSY) {
    reset(); // reset if try again
  }

  if (getEcmcEpicsIOCState()!=ECMC_IOC_STARTED_STATE) {
    errorCode_ = ECMC_PV_IOC_NOT_STARTED;
    throw std::runtime_error("Error: ECMC IOC not started.");
  }

  if(busyLock_.test_and_set()) {
    errorCode_ = ECMC_PV_BUSY;
    throw std::runtime_error("Error: Object busy. Get operation to "+ name_ + ") failed." );
  }  
  cmd_ =  ECMC_PV_CMD_GET;
  doCmdEvent_.signal();  
}

double ecmcPv::getLastReadValue() {
  return valueLatestRead_;
}


void ecmcPv::put(double value) {

  if(errorCode_ == ECMC_PV_PUT_ERROR || errorCode_ == ECMC_PV_BUSY) {
    reset(); // reset if try again
  }
  
  if (getEcmcEpicsIOCState()!=ECMC_IOC_STARTED_STATE) {
    errorCode_ = ECMC_PV_IOC_NOT_STARTED;
    throw std::runtime_error("Error: ECMC IOC not started.");
  }

  if(busyLock_.test_and_set()) {
    errorCode_ = ECMC_PV_BUSY;
    throw std::runtime_error("Error: Object busy. Put operation to "+ name_ + ") failed." );
  }  
  
  cmd_ =  ECMC_PV_CMD_PUT;
  valueToWrite_ = value;
  // Execute cmd
  doCmdEvent_.signal();

  // try{
  //   putData_->putDouble(value);
  //   put_->put();
  // }
  // catch(std::exception &e){
  //   errorCode_ = ECMC_PV_PUT_ERROR;
  //   std::cerr << "Error: " << e.what() << errorCode_ << ", " << "\n";
  // }
  return;
}

bool ecmcPv::busy() {
  if(busyLock_.test_and_set()){
    return true;
  }
  else
  {
    busyLock_.clear();
    return false;
  }  
}

void ecmcPv::exeCmdThread() {
  std::cerr << "Registering PV for: " << name_ << "\n";
  try{
    // Execute reg code here!
    pva_        = PvaClient::get("pva ca");
    pvaChannel_ = pva_->createChannel(name_,providerName_);
    pvaChannel_->issueConnect();
    Status status = pvaChannel_->waitConnect(1.0);
    if(!status.isOK()) {
      cout << " connect failed\n"; 
      throw std::runtime_error("Error: Failed connect to:" + name_);
    }
    get_ = pvaChannel_->createGet();
    get_->issueConnect();
    status = get_->waitConnect();
    if(!status.isOK()) {
      cout << " createGet failed\n"; return;
      throw std::runtime_error("Error: Failed create get to:" + name_);
    }
    getData_ = get_->getData();
    //double value = getData_->getDouble();
    //cout << "as double " << value << endl;
    //cout << "__exampleDouble__ returning\n";
    put_ = pvaChannel_->put();
    putData_ = put_->getData();
    // monitor_ = pva_->channel(pvName,providerName,2.0)->monitor("value");
    // monitorData_ = monitor_->getData();
    // provider_ = new pvac::ClientProvider(providerName_);
    // channel_  = new pvac::ClientChannel(provider_->connect(name_));
    // std::cout << channel_->name() << " : " << channel_->get()->getData()->getDouble() << "\n";    
  }
  catch(std::exception &e){
    std::cerr << "Error: " << e.what() << "\n";
    errorCode_ = ECMC_PV_PUT_ERROR;
  }

  while(true) {
    busyLock_.clear();
    doCmdEvent_.wait();
    if(destructs_) {
      break; 
    }

    switch(cmd_) {
      case ECMC_PV_CMD_GET:
        //std::cerr << "Executing Get cmd for: " << name_ << "\n";
        try{
          // Not working.. Need to revise..
          valueLatestRead_ = getData_->getDouble();
        }
        catch(std::exception &e){
          errorCode_ = ECMC_PV_GET_ERROR;
        }
        break;
      case ECMC_PV_CMD_PUT:
        //std::cerr << "Executing Put cmd for: " << name_ << "\n";
        try{
          putData_->putDouble(valueToWrite_);
          put_->put();
        }
        catch(std::exception &e){
          errorCode_ = ECMC_PV_PUT_ERROR;
        }
        break;
      default:
        break;
    }    
  } 
}

// Static Avoid issues with std:to_string()
std::string ecmcPv::to_string(int value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

