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

// Start worker threads for each object
void f_cmd_exe(void *obj) {
  if(!obj) {
    printf("%s/%s:%d: Error: Worker thread PV object NULL..\n",
            __FILE__, __FUNCTION__, __LINE__);
    return;
  }

  ecmcPv * pvObj = (ecmcPv*)obj;
  pvObj->exeCmdThread();
}

ecmcPv::ecmcPv(const std::string &channelName,
               const std::string &providerName,
               const std::string &request, 
               int index):
      channelName_(channelName),
      providerName_(providerName),
      request_(request),
      channelConnected_(false),
      monitorConnected_(false),
      putConnected_(false),
      isStarted_(false),
      typeValidated_(false),
      destructs_(false),
      index_(index),
      errorCode_(0), 
      valueLatestRead_(0),
      valueToWrite_(0),      
      type_(scalar),
      cmd_(ECMC_PV_CMD_NONE)
{
  busyLock_.test_and_set();
}

 void ecmcPv::init(PvaClientPtr const &pvaClient) {

  // pvaClientChannel_ = pvaClient->createChannel(channelName_,providerName_);
  // pvaClientChannel_->setStateChangeRequester(shared_from_this());
  // pvaClientChannel_->issueConnect();

  // // // Create Mutex to protect valueLatestRead_ (accessed from 3 threads)
  //  ecmcGetValMutex_ = epicsMutexCreate();
  //  if(!ecmcGetValMutex_) {
  //    throw std::runtime_error("Error: Create Mutex failed.");
  //  }

  pva_ = pvaClient;
  // Create worker thread
  std::string threadname = "ecmc.cmd.pv"  + to_string(index_);
  cmdExeThread_ = epicsThreadCreate(threadname.c_str(), 0, 32768, f_cmd_exe, this);
  if( cmdExeThread_ == NULL) {
     throw std::runtime_error("Error: Failed cmd exe worker thread.");
  } 
}

ecmcPv::ecmcPv() {
}

ecmcPvPtr ecmcPv::create(PvaClientPtr const & pvaClient,
                         const std::string  & channelName, 
                         const std::string  & providerName,
                         const std::string  & request,
                         int index)
{
  ecmcPvPtr client(ecmcPvPtr(new ecmcPv(channelName, providerName, request, index)));
  client->init(pvaClient);
  return client;
}

void ecmcPv::monitorConnect(epics::pvData::Status const & status,
    PvaClientMonitorPtr const & monitor, epics::pvData::StructureConstPtr const & structure)
{
  cout << "monitorConnect " << channelName_ << " status " << status << endl;
  if(!status.isOK()) return;
  monitorConnected_ = true;
  if(isStarted_) return;
  isStarted_ = true;
  pvaClientMonitor_->start();
}

void ecmcPv::event(PvaClientMonitorPtr const & monitor)
{
// cout << "event " << channelName_ << endl;
  while(monitor->poll()) {
    PvaClientMonitorDataPtr monitorData = monitor->getData();
//     cout << "monitor " << endl;
//     cout << "changed\n";
//     monitorData->showChanged(cout);
//     cout << "overrun\n";
//     monitorData->showOverrun(cout);
    if(!typeValidated_) {
      typeValidated_ = validateType(monitorData);
      if(!typeValidated_) {
        cout << "Type not supported";
        errorCode_ = ECMC_PV_TYPE_NOT_SUPPORTED;
        return;
      }
    }   
    monitor->releaseEvent();
    epicsMutexLock(ecmcGetValMutex_);
    valueLatestRead_ = getDouble(monitorData);
    epicsMutexUnlock(ecmcGetValMutex_);    
  }
}

void ecmcPv::channelPutConnect (const epics::pvData::Status &status, PvaClientPutPtr const &clientPut)
{
 // cout << "putConnect " << channelName_ << " status " << status << endl;
  if(!status.isOK()) return;
  putConnected_ = true;
  busyLock_.clear();  
}

void ecmcPv::putDone(const epics::pvData::Status & status,
                       PvaClientPutPtr const & clientPut) {
  // put cmd done.. allow new
  busyLock_.clear();
  if(!status.isOK()){
    errorCode_ = ECMC_PV_PUT_ERROR;   
  }  
}

void ecmcPv::channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
{
// cout << "channelStateChange " << channelName_ << " isConnected_ " << (isConnected ? "true" : "false") << endl;
  channelConnected_ = isConnected;
  if(isConnected) {
    if(!pvaClientMonitor_) {
      pvaClientMonitor_ = pvaClientChannel_->createMonitor(request_);
      pvaClientMonitor_->setRequester(shared_from_this());
      pvaClientMonitor_->issueConnect();
    }      
    pvaClientPut_ = pvaClientChannel_->createPut(request_);      
    pvaClientPut_->setRequester(shared_from_this());
    pvaClientPut_->issueConnect();
    typeValidated_ = false;  //Could change after reconnect?!
  }
}

PvaClientMonitorPtr ecmcPv::getPvaClientMonitor() {
  return pvaClientMonitor_;
}

void ecmcPv::stop()
{
  if(isStarted_) {
    isStarted_ = false;
    pvaClientMonitor_->stop();
  }
}

void ecmcPv::start(const string &request)
{
  if(!channelConnected_ || !monitorConnected_)
  {
    cout << "notconnected\n";
  }
  isStarted_ = true;
  pvaClientMonitor_->start(request);
}

ecmcPv::~ecmcPv() {
    destructs_ = 1;
    doCmdEvent_.signal();
    epicsThreadMustJoin(cmdExeThread_);
}

std::string ecmcPv::getChannelName(){
  return channelName_;
}

std::string ecmcPv::getProviderName() {
  return providerName_;
}

int ecmcPv::getError() {
  return errorCode_;
}

int ecmcPv::reset() {
  errorCode_ = 0;
  return 0;
}

double ecmcPv::getLastReadValue() {
  
  if (!connected()) {
    errorCode_ = ECMC_PV_NOT_CONNECTED;
    throw std::runtime_error("Error: Not connected.");
  }

  epicsMutexLock(ecmcGetValMutex_);
  double retVal=valueLatestRead_;
  epicsMutexUnlock(ecmcGetValMutex_);
  return retVal;
}

void ecmcPv::putCmd(double value) {

  reset(); // reset if try again
  
  if (!connected()) {
    errorCode_ = ECMC_PV_NOT_CONNECTED;
    throw std::runtime_error("Error: Not connected.");
  }

  if(busyLock_.test_and_set()) {
    errorCode_ = ECMC_PV_BUSY;
    throw std::runtime_error("Error: Object busy. Put operation to "+ channelName_ + ") failed." );
  }  
  
  cmd_ =  ECMC_PV_CMD_PUT;
  valueToWrite_ = value;
  
  //Execute cmd
  doCmdEvent_.signal();

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

bool ecmcPv::connected() {
  return channelConnected_ && monitorConnected_  && isStarted_ && putConnected_ && typeValidated_;
}

 void ecmcPv::exeCmdThread() {
  
  pvaClientChannel_ = pva_->createChannel(channelName_,providerName_);
  pvaClientChannel_->setStateChangeRequester(shared_from_this());
  pvaClientChannel_->issueConnect();

  // Create Mutex to protect valueLatestRead_ (accessed from 3 threads)
  ecmcGetValMutex_ = epicsMutexCreate();
  if(!ecmcGetValMutex_) {
    throw std::runtime_error("Error: Create Mutex failed.");
  }

  while(true) {
    busyLock_.clear();
    doCmdEvent_.wait();
    reset();
    if(destructs_) {
      return; 
    }

    switch(cmd_) {
      case ECMC_PV_CMD_PUT:
        try{
          if(connected()) {
            putDouble(valueToWrite_);            
          }
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

double ecmcPv::getDouble(PvaClientMonitorDataPtr monData) {
  double retVal = 0;
  PVScalarPtr pvScalar = NULL;
  switch(type_) {
    case scalar:
      // Scalar types are normal AI/AO VAL fields
      retVal = monData->getDouble();
      break;

    case structure:
      // Support enum records (return the index of the field)
      pvScalar = monData->getPVStructure()->getSubField<PVScalar>("value.index");      
      if(pvScalar) {
        retVal = pvScalar->getAs<double>();        
      } else {
        errorCode_ = ECMC_PV_GET_ERROR;
        return 0;
      }
      break;

    case scalarArray:
      // not supported yet
      errorCode_ = ECMC_PV_GET_ERROR;
      return 0;      
      break;

    default:
      errorCode_ = ECMC_PV_GET_ERROR;
      return 0;
      break;

  }
  
  errorCode_ = ECMC_PV_GET_ERROR;
  return retVal;
}

void ecmcPv::putDouble(double value) {

  PVScalarPtr pvScalar = NULL;
  switch(type_) {
    case scalar:
      pvaClientPut_->getData()->putDouble(value);
      //pvaClientPut_->put();
      pvaClientPut_->issuePut();
      break;

    case structure:
      // Support enum BI/BO records
      pvScalar = pvaClientPut_->getData()->getPVStructure()->getSubField<PVScalar>("value.index");
      if(pvScalar) {
        pvScalar->putFrom<double>(value);
        //pvaClientPut_->put();
        pvaClientPut_->issuePut();
      } else {
        errorCode_ = ECMC_PV_GET_ERROR;
        return;
      }
      break;

    case scalarArray:
      errorCode_ = ECMC_PV_GET_ERROR; 
      return;
      break;

    default:
      errorCode_ = ECMC_PV_GET_ERROR;
      return;
      break;
  }

  return;
}

int ecmcPv::validateType(PvaClientMonitorDataPtr monData) {

  if(!monData->hasValue()) {
    return 0;
  }
  PVScalarPtr pvScalar = NULL;  // Need to redo

  // Assign type
  type_ = monData->getValue()->getField()->getType();

  switch(type_) {
    case scalar:
      if(monData->isValueScalar()) {
        return 1;
      }
      else {
        return 0;
      }      
      break;
    case structure:
      // Support enum BI/BO records enum type (index, choices)
      if(!(monData->getValue()->getField()->getID()=="enum_t")) {
        cout << "NOT ENUM_T\n";
        return 0;
      }

      pvScalar = monData->getPVStructure()->getSubField<PVScalar>("value.index");
      if (pvScalar) {
        return 1;
      } else {
        cout << "NOT PVSCALAR\n";
        return 0;
      }       

      break;
    case scalarArray:
      return 0;
      break;

    default:
      return 0;
      break;

  }
  return 0;
}

// Static Avoid issues with std:to_string()
std::string ecmcPv::to_string(int value) {
  std::ostringstream os;
  os << value;
  return os.str();
}
