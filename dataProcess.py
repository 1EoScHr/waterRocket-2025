def parse_hex_data(hex_str):
    # 1. 转换为列表（去掉空格）
    hex_list = hex_str.strip().split()
    hex_list = [int(x, 16) for x in hex_list]

    # 2. 找到所有分隔符的起始位置
    split_indices = []
    for i in range(len(hex_list) - 3):
        if hex_list[i:i+4] == [0xFF, 0xFF, 0xFF, 0xFF]:
            split_indices.append(i)

    # 3. 截取每段数据，跳过分隔符
    segments = []
    for i in range(len(split_indices)):
        start = split_indices[i] + 4  # 跳过4个FF
        end = split_indices[i+1] if i+1 < len(split_indices) else len(hex_list)
        segment = hex_list[start:end]

        # 保证段长为256字节（128个u16）
        if len(segment) >= 256:
            segment = segment[:256]
            # 组装成u16（高字节在前）
            u16_list = []
            for j in range(0, len(segment), 2):
                val = (segment[j] << 8) | segment[j+1]
                u16_list.append(val)
            segments.append(u16_list)

    return segments


# 示例数据（你可以替换成从文件读取等方式）
hex_input = '''
FF FF FF FF 0F 0F 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 68 02 68 02 69 02 68 02 69 02 69 02 69 02 69 02 69 02 6A 02 6A 02 6B 02 6D 02 70 02 71 02 71 02 70 02 70 02 72 02 73 02 73 02 72 02 72 02 72 02 73 02 76 02 7A 02 7C 02 79 02 74 02 70 02 6E 02 6C 02 6B 02 6B 02 6A 02 6A 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 6A 02 6B 02 6C 02 6F 02 70 02 70 02 71 02 74 02 76 02 78 02 7B 02 7C 02 79 02 74 02 70 02 6E 02 6C 02 6B 02 6B 02 6B 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 6A 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 69 02 6A 02 6A 02 6A 02 69 02 69 02 69 02 69 02 69 02 69 00 00 00 00 00 00 00 00 00 00 00 00 
'''

# 解析并输出结果
segments = parse_hex_data(hex_input)
for idx, seg in enumerate(segments):
    print(f"Segment {idx+1} ({len(seg)} values):")
    print(seg)
