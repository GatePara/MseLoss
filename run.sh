/usr/local/Ascend/ascend-toolkit/set_env.sh
export DDK_PATH=/usr/local/Ascend/ascend-toolkit/latest
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

function main {


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

main