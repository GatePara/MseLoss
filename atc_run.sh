/usr/local/Ascend/ascend-toolkit/set_env.sh
export DDK_PATH=/usr/local/Ascend/ascend-toolkit/latest
export ASCEND_HOME_DIR=$DDK_PATH/
export NPU_HOST_LIB=$DDK_PATH/runtime/lib64/stub
export ASCEND_SLOG_PRINT_TO_STDOUT=0
export ASCEND_GLOBAL_LOG_LEVEL=0

CURRENT_DIR=$(
    cd $(dirname ${BASH_SOURCE:-$0})
    pwd
)
cd $CURRENT_DIR

# 导出环境变量
DTYPE="float16"
JSON_NAME=MseLoss
arch=$(uname -m)

# 检查当前昇腾芯片的类型
function check_soc_version() {
    SOC_VERSION_CONCAT=`python3 -c '''
import ctypes, os
def get_soc_version():
    max_len = 256
    rtsdll = ctypes.CDLL(f"libruntime.so")
    c_char_t = ctypes.create_string_buffer(b"\xff" * max_len, max_len)
    rtsdll.rtGetSocVersion.restype = ctypes.c_uint64
    rt_error = rtsdll.rtGetSocVersion(c_char_t, ctypes.c_uint32(max_len))
    if rt_error:
        print("rt_error:", rt_error)
        return ""
    soc_full_name = c_char_t.value.decode("utf-8")
    find_str = "Short_SoC_version="
    ascend_home_dir = os.environ.get("ASCEND_HOME_DIR")
    with open(f"{ascend_home_dir}/compiler/data/platform_config/{soc_full_name}.ini", "r") as f:
        for line in f:
            if find_str in line:
                start_index = line.find(find_str)
                result = line[start_index + len(find_str):].strip()
                return "{},{}".format(soc_full_name, result.lower())
    return ""
print(get_soc_version())
    '''`
    SOC_FULL_VERSION=`echo $SOC_VERSION_CONCAT | cut -d ',' -f 1`
    SOC_SHORT_VERSION=`echo $SOC_VERSION_CONCAT | cut -d ',' -f 2`
}

function main {

    # 1. 清楚遗留生成文件和日志文件
    rm -rf $HOME/ascend/log/*
    rm -rf op_models/*.om

    # 2. 编译离线om模型
    cd $CURRENT_DIR

    atc --singleop=scripts/mseloss.json  --output=op_models/ --soc_version=$SOC_FULL_VERSION
    # 3. 生成输入数据和真值数据
    python3 scripts/gen_data.py
    if [ $? -ne 0 ]; then
        echo "ERROR: generate input data failed!"
        return 1
    fi
    echo "[INFO]: generate input data success!"

    # 4. 编译acl可执行文件
    cd $CURRENT_DIR; rm -rf build; mkdir -p build; cd build
    cmake ../src -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE
    if [ $? -ne 0 ]; then
        echo "ERROR: cmake failed!"
        return 1
    fi
    echo "[INFO]: cmake success!"
    make
    if [ $? -ne 0 ]; then
        echo "ERROR: make failed!"
        return 1
    fi
    echo "[INFO]: make success!"

    # 5. 运行可执行文件
    cd $CURRENT_DIR/output

    echo "[INFO]: execute static op!"
    ./execute_op
    
    if [ $? -ne 0 ]; then
        echo "ERROR: acl executable run failed! please check your project!"
        return 1
    fi
    echo "[INFO]: acl executable run success!"

    # 6. 比较真值文件
    cd $CURRENT_DIR
    python3 scripts/verify_result.py output/output_z.bin output/golden.bin
}
check_soc_version
main