#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <arm_neon.h>
#include <chrono>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <memory>

const char dataset_path[] = "/root/data/sift/sift_base_1mx256d_f16_scale.bin";
const char query_path[] = "/root/data/sift/sift_query_10kx256d_f16_scale.bin";

const int dim = 128;
const int data_num = 1000000;
const int query_num = 10000;
const int compute_ratio = 100;

bool ReadFile(const std::string &filePath, size_t &fileSize, void *buffer, size_t bufferSize)
{
    struct stat sBuf;
    int fileStatus = stat(filePath.data(), &sBuf);
    if (fileStatus == -1)
    {
        // ERROR_LOG("failed to get file %s", filePath.c_str());
        return false;
    }
    if (S_ISREG(sBuf.st_mode) == 0)
    {
        // ERROR_LOG("%s is not a file, please enter a file", filePath.c_str());
        return false;
    }

    std::ifstream file;
    file.open(filePath, std::ios::binary);
    if (!file.is_open())
    {
        // ERROR_LOG("Open file failed. path = %s", filePath.c_str());
        return false;
    }

    std::filebuf *buf = file.rdbuf();
    size_t size = buf->pubseekoff(0, std::ios::end, std::ios::in);
    if (size == 0)
    {
        // ERROR_LOG("file size is 0");
        file.close();
        return false;
    }
    if (size > bufferSize)
    {
        // ERROR_LOG("file size is larger than buffer size");
        file.close();
        return false;
    }
    buf->pubseekpos(0, std::ios::in);
    buf->sgetn(static_cast<char *>(buffer), size);
    fileSize = size;
    file.close();
    return true;
}

static float compare(const void *a, const void *b, int dim)
{
    // using neon fp16 simd compute L2 distance
    float32x4_t sum_vec = vdupq_n_f32(0);
    unsigned i = 0;
    for (; i + 4 <= dim; i += 4)
    {
        float32x4_t a_vec = vcvt_f32_f16(vld1_f16((float16_t const *)a + i));
        float32x4_t b_vec = vcvt_f32_f16(vld1_f16((float16_t const *)b + i));
        float32x4_t diff_vec = vsubq_f32(a_vec, b_vec);
        sum_vec = vfmaq_f32(sum_vec, diff_vec, diff_vec);
    }
    float32_t sum = vaddvq_f32(sum_vec);
    for (; i < dim; i++)
    {
        float16_t diff = ((float16_t *)a)[i] - ((float16_t *)b)[i];
        sum += diff * diff;
    }

    return sum;
}

// static float compare(const void *a, const void *b, int dim)
// {
//     // using neon fp16 simd compute L2 distance
//     float16_t *va = (float16_t *)a;
//     float16_t *vb = (float16_t *)b;

//     float16x8_t sum = vdupq_n_f16(0.0f);
//     int i = 0;
//     for (; i < dim; i += 8)
//     {
//         float16x8_t v1 = vld1q_f16(va + i);
//         float16x8_t v2 = vld1q_f16(vb + i);
//         float16x8_t diff = vsubq_f16(v1, v2);
//         sum = vfmaq_f16(sum, diff, diff);
//     }
//     float32x4_t a1 = vcvt_f32_f16(vget_low_f16(sum));
//     float32x4_t a2 = vcvt_f32_f16(vget_high_f16(sum));
//     float32_t dist = vaddvq_f32(vaddq_f32(a1, a2));

//     for (; i < dim; i++)
//     {
//         float16_t diff = ((float16_t *)a)[i] - ((float16_t *)b)[i];
//         dist += diff * diff;
//     }

//     return dist;
// }

int main()
{
    int dataTypeSize = sizeof(float16_t);
    float16_t *base = new float16_t[data_num * dim];
    float16_t *query = new float16_t[query_num * dim];
    size_t fileSize = 0;
    ReadFile(dataset_path, fileSize, base, data_num * dim * dataTypeSize);
    ReadFile(query_path, fileSize, query, query_num * dim * dataTypeSize);

    // // timer
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<float> dists(query_num * data_num);
    for (int i = 0; i < query_num / compute_ratio; i++)
    {
        for (int j = 0; j < data_num / compute_ratio; j++)
        {
            dists[i * data_num + j] = compare((void *)query + i * dim * dataTypeSize, (void *)base + j * dim * dataTypeSize, dim);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("Time: %d ms\n", duration_ms.count());
    // time count by second
    auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    printf("Time: %d s\n", duration_s.count());
    delete[] base;
    delete[] query;
    return 0;
}