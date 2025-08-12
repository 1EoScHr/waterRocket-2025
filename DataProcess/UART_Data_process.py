import struct
import matplotlib.pyplot as plt

def uartDataProcess(hex_str):
    # 1. 转换为整数列表
    hex_list = [int(x, 16) for x in hex_str.strip().split()]

    # 2. 查找所有标记（FF FF FF FF 和 EE EE EE EE）
    segments = []
    i = 0
    while i < len(hex_list) - 3:
        header = hex_list[i:i+4]
        if header == [0xFF] * 4:
            data_type = 'height'
            i += 4
        elif header == [0xEE] * 4:
            data_type = 'accel'
            i += 4
        else:
            i += 1
            continue

        # 读取后面 256 字节（保证足够长度）
        if i + 256 <= len(hex_list):
            segment_data = hex_list[i:i+256]

            if data_type == 'height':
                u16_list = []
                for j in range(0, 256, 2):
                    # val = (segment_data[j] << 8) | segment_data[j+1] # 芝士手动定义的结构所用的
                    val = segment_data[j] | (segment_data[j+1] << 8) # 芝士用联合体所用的
                    u16_list.append(val)
                segments.append(('height', u16_list))

            elif data_type == 'accel':
                float_list = []
                for j in range(0, 256, 4):
                    bytes_chunk = bytes(segment_data[j:j+4])
                    val = struct.unpack('<f', bytes_chunk)[0]  # 小端 float 解码
                    float_list.append(val)
                segments.append(('accel', float_list))

            i += 256
        else:
            break
        
    print(segments)
    return segments

# 绘图函数
def plot_data(segments):
    for idx, (seg_type, values) in enumerate(segments):
        # 高度：去掉第1个与最后6个
        if seg_type == 'height':
            values = values[1:-6]
            time = [i * 0.1 for i in range(len(values))]
            ylabel = 'Height'
            color = 'blue'
        elif seg_type == 'accel':
        # 加速度：去掉第1个与最后2个
            values = values[1:-2]
            time = [i * 0.2 for i in range(len(values))]
            ylabel = 'Acceleration'
            color = 'red'

        plt.figure(figsize=(10, 4))
        plt.plot(time, values, marker='o', linestyle='-', color=color, label=seg_type.capitalize())
        plt.xlabel('Time (s)')
        plt.ylabel(ylabel)
        plt.title(f'Segment {idx+1}: {seg_type.capitalize()} vs Time')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.show()

# 示例使用
hex_input = '''
FF FF FF FF 0F 0F E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 00 00 00 00 00 00 00 00 00 00 
EE EE EE EE 0F 0F 00 00 01 E2 1B 40 8E D0 68 3F DF 56 82 3F 19 CC 7E 3F DE 05 7E 3F FE 9F 7D 3F 2E 2D 82 3F A2 BF 7E 3F 0E 06 80 3F 8F 11 7E 3F D5 F4 7D 3F 7F D7 7F 3F 09 5D 80 3F C5 0B 7C 3F 09 6E 77 3F 3F 28 7F 3F BB 0B 7C 3F EB 4E 80 3F 06 D7 7C 3F D8 81 7B 3F 87 34 81 3F FB 2F 7F 3F F2 11 80 3F 71 69 7E 3F FC A4 7F 3F B3 23 7F 3F 09 DC 80 3F 58 25 80 3F 18 AA 7E 3F 43 1E 7E 3F 33 2E 80 3F BC 8A 7E 3F 02 4B 7F 3F 84 F3 7F 3F E1 C6 7E 3F CF 8C 7E 3F 58 78 7F 3F 88 57 80 3F 7F B8 7E 3F 08 08 80 3F F1 51 7D 3F E4 13 80 3F 46 F9 7F 3F A2 4B 7E 3F 78 16 7E 3F C8 16 7F 3F 12 A7 7E 3F 87 72 7E 3F F1 B2 7E 3F 39 DB 7C 3F 7B 54 7F 3F 8A 5B 7D 3F E1 27 7E 3F 48 23 7E 3F 2D B2 7D 3F 1B AE 80 3F 42 F8 7F 3F C4 A6 7F 3F F2 FD 7E 3F 48 43 80 3F 5E D0 7D 3F 4F 64 7D 3F 00 00 00 00 
FF FF FF FF 0F 0F E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 E1 02 00 00 00 00 00 00 00 00 00 00 
EE EE EE EE 0F 0F 00 00 4E 8E 06 40 90 55 0F 3F 36 D8 69 3F EA D3 7D 3F F2 F3 7F 3F 2D 8C 80 3F AB F0 80 3F A7 8E 80 3F 21 15 80 3F 03 83 81 3F C7 F9 80 3F B4 FB 80 3F A0 3B 80 3F F3 5D 82 3F 98 37 7F 3F F1 F7 80 3F 3B AF 80 3F F2 A5 80 3F 0D 12 80 3F 4E C7 80 3F CE 09 81 3F 48 66 80 3F AB B0 80 3F 24 21 A0 3F F2 49 9B 3F 86 FC 36 3F A9 5C AC 3F AE 1B 58 3F 9D 98 A2 3F 85 3D 98 3F 22 BF 0E 3F 38 45 B7 3F CE AB 35 3F F5 C4 7F 3F 7E 7B AE 3F AC 12 23 3F 6D 89 D4 3F EB DC 3A 3F 49 0D A8 3F 08 87 29 3F 9B EA 90 3F 51 79 65 3F B2 94 81 3F 07 15 7A 3F 4A E8 80 3F D4 CA 7F 3F 6F 18 80 3F A3 14 80 3F 2C 79 81 3F 90 28 81 3F C1 42 81 3F E0 BE 80 3F 4E 76 80 3F 15 50 80 3F 18 DE 80 3F 6C F0 80 3F EC 40 81 3F 34 56 81 3F 85 7F 7E 3F E9 01 81 3F 9E 2A 7E 3F 4F 64 7D 3F 00 00 00 00 '''

segments = uartDataProcess(hex_input)
plot_data(segments)
