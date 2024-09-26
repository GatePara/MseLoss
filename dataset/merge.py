import os

def merge_files(output_file_path, input_file_prefix):
    """将分割的小文件重新组装为一个文件"""
    # 查找分割出的所有文件块，按顺序进行组合
    chunk_files = sorted([f for f in os.listdir('.') if f.startswith(input_file_prefix) and 'part' in f], 
                         key=lambda x: int(x.split('_')[-1]))
    
    with open(output_file_path, 'wb') as output_file:
        for chunk_file in chunk_files:
            with open(chunk_file, 'rb') as f:
                output_file.write(f.read())
            print(f'Merged: {chunk_file}')
    
    print(f'File reassembled into {output_file_path}')
merge_files('reassembled_sift_base_1mx256d_f16_scale.bin', 'sift_base_1mx256d_f16_scale.bin_part')
