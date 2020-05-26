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

// // Start monitor thread
// void f_monitor(void *obj) {
//   if(!obj) {
//     printf("%s/%s:%d: Error: Worker thread PV object NULL..\n",
//             __FILE__, __FUNCTION__, __LINE__);
//     return;
//   }

//   ecmcPv * pvObj = (ecmcPv*)obj;
//   pvObj->monitorThread();
// }

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
  // printf("init!!!!\n");
  // PvaClientPtr pvaClient = PvaClient::get("pva ca");
  // printf("init!!!!\n");

  pvaClientChannel_ = pvaClient->createChannel(channelName_,providerName_);
  pvaClientChannel_->setStateChangeRequester(shared_from_this());
  pvaClientChannel_->issueConnect();

  // // Create Mutex to protect valueLatestRead_ (accessed from 3 threads)
   ecmcGetValMutex_ = epicsMutexCreate();
   if(!ecmcGetValMutex_) {
     throw std::runtime_error("Error: Create Mutex failed.");
   }

  // Create worker thread
  std::string threadname = "ecmc.cmd.pv"  + to_string(index_);
  cmdExeThread_ = epicsThreadCreate(threadname.c_str(), 0, 32768, f_cmd_exe, this);
  if( cmdExeThread_ == NULL) {
     throw std::runtime_error("Error: Failed cmd exe worker thread.");
  } 

  // // Create monitor thread
  // threadname = "ecmc.mon.pv" + to_string(index_);
  // monThread_ = epicsThreadCreate(threadname.c_str(), 0, 32768, f_monitor, this);
  // if( monThread_ == NULL) {
  //   throw std::runtime_error("Error: Failed create monitor worker thread.");
  // }
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
  cout << "event " << channelName_ << endl;
  while(monitor->poll()) {
    PvaClientMonitorDataPtr monitorData = monitor->getData();
/*  cout << "monitor " << endl;
    cout << "changed\n";
    monitorData->showChanged(cout);
    cout << "overrun\n";
    monitorData->showOverrun(cout);*/
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
  //cout << "putConnect " << channelName_ << " status " << status << endl;
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
  //cout << "channelStateChange " << channelName_ << " isConnected_ " << (isConnected ? "true" : "false") << endl;
  channelConnected_ = isConnected;
  if(isConnected) {
    if(!pvaClientMonitor_) {
      pvaClientMonitor_ = pvaClientChannel_->createMonitor(request_);
      pvaClientMonitor_->setRequester(shared_from_this());
      pvaClientMonitor_->issueConnect();
    }  
    //if(!pvaClientPut_) {
      pvaClientPut_ = pvaClientChannel_->createPut(request_);      
      pvaClientPut_->setRequester(shared_from_this());
      pvaClientPut_->issueConnect();
    //}
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

// void ecmcPv::getCmd() {

//   if(errorCode_ == ECMC_PV_GET_ERROR || errorCode_ == ECMC_PV_BUSY) {
//     reset(); // reset if try again
//   }

//   if (getEcmcEpicsIOCState()!=ECMC_IOC_STARTED_STATE) {
//     errorCode_ = ECMC_PV_IOC_NOT_STARTED;
//     throw std::runtime_error("Error: ECMC IOC not started.");
//   }

//   if(busyLock_.test_and_set()) {
//     errorCode_ = ECMC_PV_BUSY;
//     throw std::runtime_error("Error: Object busy. Get operation to "+ channelName_ + ") failed." );
//   }  
//   cmd_ =  ECMC_PV_CMD_GET;
//   doCmdEvent_.signal();  
// }

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

// // Send Reg cmd to worker
// void ecmcPv::regCmd() {
//   if(busyLock_.test_and_set()) {
//     errorCode_ = ECMC_PV_BUSY;
//     throw std::runtime_error("Error: Object busy. Reg operation to "+ channelName_ + ") failed." );
//   }  
//   cmd_ =  ECMC_PV_CMD_REG;
//   doCmdEvent_.signal();  
// }

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
  // Retry untill success..
  // while(!connected() ) {
  //   try{
  //     connect();      
  //   }
  //   catch(std::exception &e){
  //     std::cerr << "Error: Connect failed: " << e.what() << "Try to reconnect in " 
  //               << to_string(ECMC_PV_TIME_BETWEEN_RECONNECT) << "s\n";
  //     errorCode_ = ECMC_PV_REG_ERROR;
  //     epicsThreadSleep(ECMC_PV_TIME_BETWEEN_RECONNECT);
  //   }
  // }

  // Now connected
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

//  // Monitor thread
// void ecmcPv::monitorThread() {

//   PVScalarPtr pvScalar = NULL;
//   double retValue = 0;
//   while(true) {
//     if(destructs_) {
//       return; 
//     }
//     if(!connected_) {
//       epicsThreadSleep(ECMC_PV_TIME_BETWEEN_RECONNECT);
//       continue;
//     }
// //    try{
//       if(pvaClientMonitor_->waitEvent(0.1)) {
//         while(true) {
  
//           retValue = 0;        
//           if(destructs_) {
//             return; 
//           }
  
//           switch(type_) {
//             case scalar: 
//               retValue = monitorData_->getDouble();
//               break;
  
//             case structure:
//               pvScalar = monitorData_->getPVStructure()->getSubField<PVScalar>("value.index");      
//               if(pvScalar) {
//                 retValue = pvScalar->getAs<double>();
//               } else {
//                 errorCode_ = ECMC_PV_MON_ERROR;
//               }
//               break;
  
//             default:
//               errorCode_ = ECMC_PV_MON_ERROR;
//               break;
//           }
  
//           epicsMutexLock(ecmcGetValMutex_);
//           valueLatestRead_ = retValue;
//           epicsMutexUnlock(ecmcGetValMutex_);
  
//           //printf("pv: %s, new value = %lf\n",channelName_.c_str(),valueLatestRead_);
//           //cout << "changed\n";
//           //monitorData_->showChanged(cout);
//           //cout << "overrun\n";
//           //monitorData_->showOverrun(cout);
//           pvaClientMonitor_->releaseEvent();
//           if(!pvaClientMonitor_->poll()) break;
//         }
//       }
//     // } 
//     // catch(std::exception &e){
//     //   std::cerr << "Error: IN MONITOR THREAD..";
//     // }
//   }
// }

// int ecmcPv::connect() {

//   pva_ = PvaClient::get(providerName_);
//   pvaClientChannel_ = pva_->createChannel(channelName_,providerName_);
//   pvaClientChannel_->setStateChangeRequester(shared_from_this());

//   pvaClientChannel_->issueConnect();
//   Status status = pvaClientChannel_->waitConnect(1.0);
//   if(!status.isOK()) {
//     cout << "Error: connect failed\n";
//     errorCode_ = ECMC_PV_REG_ERROR;
//     throw std::runtime_error("Error: Failed connect to:" + channelName_);
//   }
//   pvaClientMonitor_ = pva_->channel(channelName_,providerName_,2.0)->monitor("");
//   monitorData_ = pvaClientMonitor_->getData();
//   get_ = pvaClientChannel_->createGet();
//   get_->issueConnect();
//   status = get_->waitConnect();
//   if(!status.isOK()) {
//     cout << "Error: createGet failed\n";
//     errorCode_ = ECMC_PV_REG_ERROR;
//     throw std::runtime_error("Error: Failed create get to:" + channelName_);
//   }
//   getData_ = get_->getData();
//   //printf("Get Data has value: %d, isValueScalar %d\n", getData_->hasValue(),getData_->isValueScalar());
//   pvaClientPut_ = pvaClientChannel_->put();
//   putData_ = pvaClientPut_->getData();

//   //printf("Put Data has value: %d, isScalarArray %d\n", putData_->hasValue(),getData_->isValueScalarArray());
//   //cout << "getData_->getvalue(): " << getData_->getValue()<< "\n";
//   //cout << "getData_->getString(): " << getData_->getString()<< "\n";
//   //cout << "getData_->getPVStructure(): " << getData_->getPVStructure()<< "\n";
//   //cout << "getData_->getStructure(): " << getData_->getStructure()<< "\n";
//   //cout << "getData_->getValue()->getField()->getType(): " << getData_->getValue()->getField()->getType() << "\n";
//   //cout << "getData_->getValue()->getField(): " << getData_->getValue()->getField() << "\n"; 
//   //cout << "getData_->getValue()->getField(): " << getData_->getValue()->getField() << "\n"; 

//   // Ensure that type is scalar or enum
//   type_ = getData_->getValue()->getField()->getType();
//   if(!validateType()) {
//     cout << "Error: Type not supported for PV: " + channelName_ +"\n";
//     errorCode_ = ECMC_PV_TYPE_NOT_SUPPORTED;
//     throw std::runtime_error("Error: Type not supported for PV: " + channelName_);
//   }

//   connected_ = true;
//   return 0;
// }

double ecmcPv::getDouble(PvaClientMonitorDataPtr monData) {
  double retVal = 0;
  PVScalarPtr pvScalar = NULL;
  switch(type_) {
    case scalar:
      // Scalar types are normal AI/AO VAL fields
      //retVal = pva_->channel(channelName_,providerName_)->getDouble();
      retVal = monData->getDouble();
      break;

    case structure:
      // Support enum records (return the index of the field)
      //get_->get();
      //pvScalar = getData_->getPVStructure()->getSubField<PVScalar>("value.index");      
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
        return 0;
      }

      pvScalar = monData->getPVStructure()->getSubField<PVScalar>("value.index");
      if (pvScalar) {
        return 1;
      } else {
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
