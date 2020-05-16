/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPva.cpp
*
*  Created on: May 12, 2020
*      Author: anderssandstrom
*
\*************************************************************************/

#include "exprtk.hpp"
#include "pva/client.h"
#include "ecmcPv.h"
#include "ecmcPvDefs.h"

vector<ecmcPv*> pvVector;

// class for exprtk handle=pv_reg(<pvName>, <providerName = "pva"/"ca">) command
template <typename T>
struct pvreg : public exprtk::igeneric_function<T>
{
public:

  typedef typename exprtk::igeneric_function<T> igfun_t;
  typedef typename igfun_t::parameter_list_t    parameter_list_t;
  typedef typename igfun_t::generic_type        generic_type;
  typedef typename generic_type::string_view    string_t;

  using exprtk::igeneric_function<T>::operator();

  pvreg()
  : exprtk::igeneric_function<T>("SS")
  { 
    printf("pvreg constructs 1\n"); 
  }

  inline T operator()(parameter_list_t parameters)
  {
    if (getEcmcEpicsIOCState()!=ECMC_IOC_STARTED_STATE) {    
      return ECMC_PV_IOC_NOT_STARTED;
    }

    string_t pvName(parameters[0]);
    string_t providerName(parameters[1]);
    try{
      // Create object and append to "global" vector
      ecmcPv* pv = new ecmcPv(&pvName[0],&providerName[0],pvVector.size()+1);
      pvVector.push_back(pv);
    }    
    catch(std::exception &e){
      std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_REG_ASYNC  "(): " << e.what() << "\n";
      return T(ECMC_PV_REG_ERROR);
    }
    // return handle to object (1 higher than index to avoid 0)
    return pvVector.size();
  }
};
