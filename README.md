# 样例1：单对向量的L2距离计算

原理：通过aclopExecuteV2接口调用NPU自带的MseLoos算子，当入参x，y都是shape为(1,d)的Tensor时，属性reduction = "sum"，x和y的MseLoss就是他们的L2距离。

使用ATC生成OM算子，并编译运行算子验证程序。
```bash
bash atc_run.sh
```

直接编译运行算子验证程序。
```bash
bash run.sh
```

注：经测试，若x与y的shape不同，返回结果为nan。当x和y为相同shape的(n,d)的Tensor时，若reduction = "sum" 或 "mean"，输出结果必为一个(1,1)的Tensor，因此MseLoos算子只能计算单对向量的L2距离。