/*******************************************************************************
* Copyright 2015-2018 Intel Corporation
* All Rights Reserved.
*
* If this  software was obtained  under the  Intel Simplified  Software License,
* the following terms apply:
*
* The source code,  information  and material  ("Material") contained  herein is
* owned by Intel Corporation or its  suppliers or licensors,  and  title to such
* Material remains with Intel  Corporation or its  suppliers or  licensors.  The
* Material  contains  proprietary  information  of  Intel or  its suppliers  and
* licensors.  The Material is protected by  worldwide copyright  laws and treaty
* provisions.  No part  of  the  Material   may  be  used,  copied,  reproduced,
* modified, published,  uploaded, posted, transmitted,  distributed or disclosed
* in any way without Intel's prior express written permission.  No license under
* any patent,  copyright or other  intellectual property rights  in the Material
* is granted to  or  conferred  upon  you,  either   expressly,  by implication,
* inducement,  estoppel  or  otherwise.  Any  license   under such  intellectual
* property rights must be express and approved by Intel in writing.
*
* Unless otherwise agreed by Intel in writing,  you may not remove or alter this
* notice or  any  other  notice   embedded  in  Materials  by  Intel  or Intel's
* suppliers or licensors in any way.
*
*
* If this  software  was obtained  under the  Apache License,  Version  2.0 (the
* "License"), the following terms apply:
*
* You may  not use this  file except  in compliance  with  the License.  You may
* obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
*
* Unless  required  by   applicable  law  or  agreed  to  in  writing,  software
* distributed under the License  is distributed  on an  "AS IS"  BASIS,  WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
* See the   License  for the   specific  language   governing   permissions  and
* limitations under the License.
*******************************************************************************/

/* 
// 
//  Purpose:
//     Cryptography Primitive.
//     PRNG Functions
// 
//  Contents:
//        ippsPRNGenRDRAND()
//
*/

#include "owndefs.h"
#include "owncp.h"
#include "pcpbn.h"
#include "pcptool.h"
#include "pcpprng_genhw.h"

/*F*
// Name: ippsPRNGenRDRAND
//
// Purpose: Generates a pseudorandom bit sequence
//          based on RDRAND instruction of the specified nBits length.
//
// Returns:                   Reason:
//    ippStsNullPtrErr           NULL == pRand
//
//    ippStsLengthErr            1 > nBits
//
//    ippStsNotSupportedModeErr  unsupported rdrand instruction
//
//    ippStsErr                  random bit sequence can't be generated
//
//    ippStsNoErr                no error
//
// Parameters:
//    pRand    pointer to the buffer
//    nBits    number of bits be requested
//    pCtx     pointer to the context
*F*/
IPPFUN(IppStatus, ippsPRNGenRDRAND,(Ipp32u* pRand, int nBits, void* pCtx))
{
   /* test PRNG buffer */
   IPP_BAD_PTR1_RET(pRand);

   /* test sizes */
   IPP_BADARG_RET(nBits< 1, ippStsLengthErr);

   UNREFERENCED_PARAMETER(pCtx);

   #if ((_IPP>=_IPP_G9) || (_IPP32E>=_IPP32E_E9))
   if( IsFeatureEnabled(ippCPUID_RDRAND) ) {
      cpSize rndSize = BITS2WORD32_SIZE(nBits);
      Ipp32u rndMask = MAKEMASK32(nBits);

      if(cpRandHW_buffer(pRand, rndSize)) {
         pRand[rndSize-1] &= rndMask;
         return ippStsNoErr;
      }
      else
         return ippStsErr;
   }
   /* unsupported rdrand instruction */
   else
   #endif
      IPP_ERROR_RET(ippStsNotSupportedModeErr);
}
