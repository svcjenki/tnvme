/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "illegalCreateOrder_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/queues.h"

namespace GrpQueues {


IllegalCreateOrder_r10b::IllegalCreateOrder_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Create IOQ's out of order.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue Create IOSQ cmd before an IOCQ exists, thus specify an "
        "unallocated IOCQ to expect status code = \"Completion Q Invalid\".");
}


IllegalCreateOrder_r10b::~IllegalCreateOrder_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalCreateOrder_r10b::
IllegalCreateOrder_r10b(const IllegalCreateOrder_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalCreateOrder_r10b &
IllegalCreateOrder_r10b::operator=(const IllegalCreateOrder_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
IllegalCreateOrder_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) None.
     *  \endverbatim
     */
    const uint32_t NumEntriesIOQ = 2;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx();

    // Create Admin Q Objects for test lifetime
    SharedACQPtr acq = SharedACQPtr(new ACQ(mFd));
    acq->Init(5);
    SharedASQPtr asq = SharedASQPtr(new ASQ(mFd));
    asq->Init(5);

    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx();

    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    gCtrlrConfig->SetIOSQES(iosqes);
    SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(mFd));
    LOG_NRM("Associate IOSQ #%d to unallocated IOCQ #%d", IOQ_ID, IOQ_ID);
    iosq->Init(IOQ_ID, NumEntriesIOQ, IOQ_ID, 0);
    SharedCreateIOSQPtr createIOSQCmd = SharedCreateIOSQPtr(new CreateIOSQ());
    createIOSQCmd->Init(iosq);

    IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
    createIOSQCmd, "Invalid.CQId", true, CESTAT_CQ_INVALID);

}

}   // namespace
