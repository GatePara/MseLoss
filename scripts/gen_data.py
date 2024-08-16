#!/usr/bin/python3
# -*- coding:utf-8 -*-
# Copyright 2022-2023 Huawei Technologies Co., Ltd
import numpy as np

def gen_golden_data_simple():
    # input_x = np.random.uniform(-100, 100, [8, 2048]).astype(np.float32)
    # input_y = np.random.uniform(-100, 100, [8, 2048]).astype(np.float32)
    # input_x = np.random.uniform(1, 100, [8, 2048]).astype(np.float16)
    # input_y = np.random.uniform(1, 100, [8, 2048]).astype(np.float16)
    # 测试过 如果两个矩阵shape不同会返回nan
    n1 = 1
    n2 = n1
    dim = 128
    a = np.ones((n1,dim)).astype(np.float16)
    b = np.zeros((n2,dim)).astype(np.float16)
    # b[0][0] = -1
    a.tofile('./input/input_x.bin')
    b.tofile('./input/input_y.bin')
    out = np.multiply((a.astype(np.float32)-b.astype(np.float32)),(a.astype(np.float32)-b.astype(np.float32)))
    out = np.sum(out.astype(np.float16))
    out.tofile('./output/golden.bin')
    # golden = (input_x + input_y).astype(np.float16)

    # input_x.tofile("./input/input_x.bin")
    # input_y.tofile("./input/input_y.bin")
    # golden.tofile("./output/golden.bin")

if __name__ == "__main__":
    gen_golden_data_simple()
