CURRENT_DIR=$(
    cd $(dirname ${BASH_SOURCE:-$0})
    pwd
)
cd $CURRENT_DIR


function main {

    # 4. 编译可执行文件
    cd $CURRENT_DIR; rm -rf build; mkdir -p build
    g++ src/baseline.cpp -o ./build/baseline -march=native -std=c++11 
    if [ $? -ne 0 ]; then
        echo "ERROR: make failed!"
        return 1
    fi
    echo "[INFO]: make success!"

    # 5. 运行可执行文件
    cd $CURRENT_DIR/build

    echo "[INFO]: execute baseline!"
    ./baseline

}

main