def split_file(file_path, chunk_size=10 * 1024 * 1024):
    """将文件按块分割为多个小文件，每块大小为chunk_size字节"""
    with open(file_path, 'rb') as f:
        index = 0
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            # 生成分割文件名
            chunk_file_name = f'{file_path}_part_{index}'
            with open(chunk_file_name, 'wb') as chunk_file:
                chunk_file.write(chunk)
            print(f'Created: {chunk_file_name}')
            index += 1
    print(f'File split into {index} parts.')

split_file('sift_base_1mx256d_f16_scale.bin')
