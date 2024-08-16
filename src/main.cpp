/**
 * @file main.cpp
 *
 * Copyright (C) 2020. Huawei Technologies Co., Ltd. All rights reserved.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <cstdint>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "acl/acl.h"
#include "op_runner.h"

#include "common.h"

bool g_isDevice = false;
int deviceId = 0;
int isDynamic = 0;
int length = 0;

const int a = 1;
const int b = 128;
const int m = a;
const int n = b;

const char dataset_path[] = "/root/data/sift/sift_base_1mx256d_f16.bin";
const char query_path[] = "/root/data/sift/sift_query_10kx256d_f16.bin";

const int dim = b;
const int data_num = 1000000;
const int query_num = 10000;

const aclDataType dataType = ACL_FLOAT16;

OperatorDesc CreateOpDesc()
{
    // define operator
    // std::vector<int64_t> shape { 8, 2048 };
    std::vector<int64_t> inputShape0{a, b};
    std::vector<int64_t> inputShape1{m, n};
    std::vector<int64_t> outputShape{a, m};
    std::string opType = "MseLoss";
    // if (isDynamic) {
    //     shape = {8, length};
    // }
    aclFormat format = ACL_FORMAT_ND;
    OperatorDesc opDesc(opType);
    // printf("inputShape0 size is %ld\n",inputShape0.size());
    opDesc.AddInputTensorDesc(dataType, inputShape0.size(), inputShape0.data(), format);
    opDesc.AddInputTensorDesc(dataType, inputShape1.size(), inputShape1.data(), format);
    opDesc.AddOutputTensorDesc(dataType, outputShape.size(), outputShape.data(), format);
    auto ret = aclopSetAttrString(opDesc.opAttr, "reduction", "sum");
    return opDesc;
}

bool SetInputData(OpRunner &runner)
{
    size_t fileSize = 0;
    // printf("input0 size is %ld\n", runner.GetInputSize(0));
    ReadFile("../input/input_x.bin", fileSize, runner.GetInputBuffer<void>(0), runner.GetInputSize(0));
    ReadFile("../input/input_y.bin", fileSize, runner.GetInputBuffer<void>(1), runner.GetInputSize(1));
    INFO_LOG("Set input success");

    return true;
}

bool ProcessOutputData(OpRunner &runner)
{
    WriteFile("../output/output_z.bin", runner.GetOutputBuffer<void>(0), runner.GetOutputSize(0));
    INFO_LOG("Write output success");
    return true;
}

bool RunOp()
{
    // Create op desc
    OperatorDesc opDesc = CreateOpDesc();

    // Create Runner
    OpRunner opRunner(&opDesc);

    if (!opRunner.SetDataInfo(dim, data_num, query_num, dataType))
    {
        ERROR_LOG("Set Data Info failed");
        return false;
    }

    if (!opRunner.Init())
    {
        ERROR_LOG("Init OpRunner failed");
        return false;
    }

    // Load inputs
    if (!SetInputData(opRunner))
    {
        ERROR_LOG("Set input data failed");
        return false;
    }

    // Run op prepare
    if (!opRunner.RunOpPrepare())
    {
        ERROR_LOG("Run op prepare failed");
        return false;
    }

    // Run op
    if (!opRunner.RunOp())
    {
        ERROR_LOG("Run op failed");
        return false;
    }

    // Destory Stream
    if (!opRunner.DestoryStream())
    {
        ERROR_LOG("Destory Stream failed");
        return false;
    }

    // Process output data
    if (!ProcessOutputData(opRunner))
    {
        ERROR_LOG("Process output data failed");
        return false;
    }

    INFO_LOG("Run op success");
    return true;
}

void DestoryResource()
{
    bool flag = false;
    if (aclrtResetDevice(deviceId) != ACL_SUCCESS)
    {
        ERROR_LOG("Reset device %d failed", deviceId);
        flag = true;
    }
    INFO_LOG("Reset Device success");
    if (aclFinalize() != ACL_SUCCESS)
    {
        ERROR_LOG("Finalize acl failed");
        flag = true;
    }
    if (flag)
    {
        ERROR_LOG("Destory resource failed");
    }
    else
    {
        INFO_LOG("Destory resource success");
    }
}

bool InitResource()
{
    std::string output = "./output";
    if (access(output.c_str(), 0) == -1)
    {
        int ret = mkdir(output.c_str(), 0700);
        if (ret == 0)
        {
            INFO_LOG("Make output directory successfully");
        }
        else
        {
            ERROR_LOG("Make output directory fail");
            return false;
        }
    }

    // acl.json is dump or profiling config file
    if (aclInit("../scripts/acl.json") != ACL_SUCCESS)
    {
        ERROR_LOG("acl init failed");
        return false;
    }

    if (aclrtSetDevice(deviceId) != ACL_SUCCESS)
    {
        ERROR_LOG("Set device failed. deviceId is %d", deviceId);
        (void)aclFinalize();
        return false;
    }
    INFO_LOG("Set device[%d] success", deviceId);

    // runMode is ACL_HOST which represents app is running in host
    // runMode is ACL_DEVICE which represents app is running in device
    aclrtRunMode runMode;
    if (aclrtGetRunMode(&runMode) != ACL_SUCCESS)
    {
        ERROR_LOG("Get run mode failed");
        DestoryResource();
        return false;
    }
    g_isDevice = (runMode == ACL_DEVICE);
    INFO_LOG("Get RunMode[%d] success", runMode);

    // set model path
    if (aclopSetModelDir("../op_models") != ACL_SUCCESS)
    {
        std::cerr << "Load single op model failed" << std::endl;
        (void)aclFinalize();
        return FAILED;
    }
    INFO_LOG("aclopSetModelDir op model success", deviceId);

    return true;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        INFO_LOG("static op will be called");
    }
    else
    {
        ERROR_LOG("wrong input parameter number");
        return -1;
    }

    if (!InitResource())
    {
        ERROR_LOG("Init resource failed");
        return FAILED;
    }
    INFO_LOG("Init resource success");

    if (!RunOp())
    {
        DestoryResource();
        return FAILED;
    }

    DestoryResource();

    return SUCCESS;
}
