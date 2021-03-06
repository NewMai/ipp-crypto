/*******************************************************************************
* Copyright 2016-2018 Intel Corporation
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
//     Intel(R) Integrated Performance Primitives. Cryptography Primitives.
// 
//     Context:
//        ippsGFpECVerifySM2()
//
*/

#include "owndefs.h"
#include "owncp.h"
#include "pcpeccp.h"

/*F*
//    Name: ippsGFpECVerifySM2
//
// Purpose: SM2 Signature Verification.
//
// Returns:                   Reason:
//    ippStsNullPtrErr           NULL == pEC
//                               NULL == pMsgDigest
//                               NULL == pRegPublic
//                               NULL == pSignR
//                               NULL == pSignS
//                               NULL == pResult
//                               NULL == pScratchBuffer
//
//    ippStsContextMatchErr      illegal pECC->idCtx
//                               illegal pMsgDigestDigest->idCtx
//                               illegal pRegPublic->idCtx
//                               illegal pSignR->idCtx
//                               illegal pSignS->idCtx
//
//    ippStsMessageErr           0> MsgDigest
//                               order<= MsgDigest
//
//    ippStsRangeErr             SignR < 0 or SignS < 0
//
//    ippStsOutOfRangeErr        bitsize(pRegPublic) != bitsize(prime)
//
//    ippStsNotSupportedModeErr  1<GFP_EXTDEGREE(pGFE)
//
//    ippStsNoErr                no errors
//
// Parameters:
//    pMsgDigest     pointer to the message representative to being signed
//    pRegPublic     pointer to the regular public key
//    pSignR,pSignS  pointer to the signature
//    pResult        pointer to the result: ippECValid/ippECInvalidSignature
//    pEC            pointer to the ECCP context
//    pScratchBuffer pointer to buffer (2 mul_point operation)
//
*F*/
IPPFUN(IppStatus, ippsGFpECVerifySM2,(const IppsBigNumState* pMsgDigest,
                                      const IppsGFpECPoint* pRegPublic,
                                      const IppsBigNumState* pSignR, const IppsBigNumState* pSignS,
                                      IppECResult* pResult,
                                      IppsGFpECState* pEC,
                                      Ipp8u* pScratchBuffer))
{
   IppsGFpState*  pGF;
   gsModEngine* pGFE;

   /* EC context and buffer */
   IPP_BAD_PTR2_RET(pEC, pScratchBuffer);
   pEC = (IppsGFpECState*)( IPP_ALIGNED_PTR(pEC, ECGFP_ALIGNMENT) );
   IPP_BADARG_RET(!ECP_TEST_ID(pEC), ippStsContextMatchErr);
   IPP_BADARG_RET(!ECP_SUBGROUP(pEC), ippStsContextMatchErr);

   pGF = ECP_GFP(pEC);
   pGFE = GFP_PMA(pGF);
   IPP_BADARG_RET(1<GFP_EXTDEGREE(pGFE), ippStsNotSupportedModeErr);

   /* test message representative */
   IPP_BAD_PTR1_RET(pMsgDigest);
   pMsgDigest = (IppsBigNumState*)( IPP_ALIGNED_PTR(pMsgDigest, ALIGN_VAL) );
   IPP_BADARG_RET(!BN_VALID_ID(pMsgDigest), ippStsContextMatchErr);
   IPP_BADARG_RET(BN_NEGATIVE(pMsgDigest), ippStsMessageErr);

   /* test regular public key */
   IPP_BAD_PTR1_RET(pRegPublic);
   IPP_BADARG_RET( !ECP_POINT_TEST_ID(pRegPublic), ippStsContextMatchErr );
   IPP_BADARG_RET( ECP_POINT_FELEN(pRegPublic)!=GFP_FELEN(pGFE), ippStsOutOfRangeErr);

   /* test signature */
   IPP_BAD_PTR2_RET(pSignR, pSignS);
   pSignR = (IppsBigNumState*)( IPP_ALIGNED_PTR(pSignR, ALIGN_VAL) );
   pSignS = (IppsBigNumState*)( IPP_ALIGNED_PTR(pSignS, ALIGN_VAL) );
   IPP_BADARG_RET(!BN_VALID_ID(pSignR), ippStsContextMatchErr);
   IPP_BADARG_RET(!BN_VALID_ID(pSignS), ippStsContextMatchErr);
   IPP_BADARG_RET(BN_NEGATIVE(pSignR), ippStsRangeErr);
   IPP_BADARG_RET(BN_NEGATIVE(pSignS), ippStsRangeErr);

   /* test result */
   IPP_BAD_PTR1_RET(pResult);

   {
      IppECResult vResult = ippECInvalidSignature;

      gsModEngine* pMontR = ECP_MONT_R(pEC);
      BNU_CHUNK_T* pOrder = MOD_MODULUS(pMontR);
      int orderLen = MOD_LEN(pMontR);

      /* test mesage: msg<order */
      IPP_BADARG_RET((0<=cpCmp_BNU(BN_NUMBER(pMsgDigest), BN_SIZE(pMsgDigest), pOrder, orderLen)), ippStsMessageErr);

      /* test signature value */
      if(!cpEqu_BNU_CHUNK(BN_NUMBER(pSignR), BN_SIZE(pSignR), 0) &&
         !cpEqu_BNU_CHUNK(BN_NUMBER(pSignS), BN_SIZE(pSignS), 0) &&
         0>cpCmp_BNU(BN_NUMBER(pSignR), BN_SIZE(pSignR), pOrder, orderLen) &&
         0>cpCmp_BNU(BN_NUMBER(pSignS), BN_SIZE(pSignS), pOrder, orderLen)) {

         int elmLen = GFP_FELEN(pGFE);
         int ns;

         BNU_CHUNK_T* r = cpGFpGetPool(4, pGFE);
         BNU_CHUNK_T* s = r+orderLen;
         BNU_CHUNK_T* t = s+orderLen;
         BNU_CHUNK_T* f = t+orderLen;

         /* expand signatire's components */
         cpGFpElementCopyPadd(r, orderLen, BN_NUMBER(pSignR), BN_SIZE(pSignR));
         cpGFpElementCopyPadd(s, orderLen, BN_NUMBER(pSignS), BN_SIZE(pSignS));

         /* t = (r+s) mod order */
         cpModAdd_BNU(t, r, s, pOrder, orderLen, f);

         /* P = [s]G +[t]regPublic, t = P.x */
         {
            IppsGFpECPoint P, G;
            cpEcGFpInitPoint(&P, cpEcGFpGetPool(1, pEC),0, pEC);
            cpEcGFpInitPoint(&G, ECP_G(pEC), ECP_AFFINE_POINT|ECP_FINITE_POINT, pEC);

            gfec_BasePointProduct(&P,
                                  s, orderLen, pRegPublic, t, orderLen,
                                  pEC, pScratchBuffer);

            gfec_GetPoint(t, NULL, &P, pEC);
            GFP_METHOD(pGFE)->decode(t, t, pGFE);
            ns = cpMod_BNU(t, elmLen, pOrder, orderLen);

            cpEcGFpReleasePool(1, pEC);
         }

         /* t = (msg+t) mod order */
         cpGFpElementCopyPadd(f, orderLen, BN_NUMBER(pMsgDigest), BN_SIZE(pMsgDigest));
         cpModAdd_BNU(t, t, f, pOrder, orderLen, f);

         if(GFP_EQ(t, r, orderLen))
            vResult = ippECValid;

         cpGFpReleasePool(4, pGFE);
      }

      *pResult = vResult;
      return ippStsNoErr;
   }
}
