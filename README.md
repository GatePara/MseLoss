# 样例1：单对向量的L2距离计算

原理：通过aclopExecuteV2接口调用NPU自带的MseLoos算子，当入参x，y都是shape为(1,d)的Tensor时，属性reduction = "sum"，x和y的MseLoss就是他们的L2距离。

此版本可计算指定数据集Base与Query之间的成对欧氏距离，数据集须存储为二进制Float16格式，为防止结果溢出数值范围最好缩放到0-1之间，超参数在src/main.cpp中设置。

考虑到计算过程缓慢，可以在src/op_runner.cpp中调节计算比率，compute_ratio为100则对参加计算的base和query数目分别除以100。

使用ATC生成OM算子，并编译运行算子验证程序。
```bash
bash atc_run.sh
```

直接编译运行算子验证程序。
```bash
bash run.sh
```

直接编译运行CPU Baseline程序。
```bash
bash run_baseline.sh
```

注：经测试，若x与y的shape不同，返回结果为nan。当x和y为相同shape的(n,d)的Tensor时，若reduction = "sum" 或 "mean"，输出结果必为一个(1,1)的Tensor，因此MseLoos算子只能计算单对向量的L2距离。