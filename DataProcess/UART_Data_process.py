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
                    val = (segment_data[j] << 8) | segment_data[j+1]
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
FF FF FF FF 0F 0F 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 87 02 88 02 87 02 87 02 87 02 87 02 87 00 00 00 00 00 00 00 00 00 00 00 00 
EE EE EE EE 0F 0F 00 00 5C ED 90 40 7A 5D 64 3F 3B 8C 85 3F 03 CE 84 3F B9 41 80 3F E0 ED 81 3F 70 EA 7D 3F 41 2C 80 3F 3F E7 6F 3F F2 90 81 3F 95 24 80 3F FA 7F 7D 3F 62 EC 7D 3F 13 58 7D 3F C6 59 7D 3F F5 60 7D 3F 7F E7 7C 3F E1 39 7D 3F AA 79 7D 3F 77 A0 7D 3F 24 FC 7C 3F 8E CB 7D 3F F8 77 7D 3F 1D 5E 7D 3F C8 5D 7D 3F 13 F3 7C 3F AE 43 7D 3F 87 0E 7D 3F CA E7 7C 3F 20 69 7D 3F FC 26 7D 3F 7F 5F 7D 3F 57 72 7D 3F 39 25 7D 3F 26 33 7D 3F 8C 4E 7D 3F 07 A3 7D 3F 83 96 7D 3F CA 80 7D 3F B3 7F 7D 3F 8D 32 7D 3F 5F 7D 7D 3F 4D 1C 7D 3F FE 9E 7D 3F 6A 81 7D 3F B4 67 7D 3F B1 B2 59 40 97 A2 DB 3F 9B 43 CE 3E E7 AE 96 3F B1 34 8D 3F FB 6C 88 3F 0A 41 70 3F A9 01 78 3F F1 B9 67 3F F2 2D 88 3F A8 DE 80 3F F9 12 60 40 0C 55 65 3F 6B 92 9B 3F 82 C2 A3 3F 00 00 00 00 00 00 00 00 
FF FF FF FF 0F 0F 02 A9 02 AA 02 AB 02 AB 02 AD 02 AE 02 B0 02 B2 02 B5 02 B8 02 BB 02 BE 02 C2 02 C7 02 CB 02 CF 02 D4 02 D8 02 DB 02 DF 02 E3 02 E7 02 EA 02 EE 02 F1 02 F4 02 F8 02 FB 02 FD 03 00 03 02 03 05 03 07 03 09 03 0B 03 0D 03 0F 03 10 03 12 03 13 03 15 03 16 03 17 03 18 03 19 03 1A 03 1B 03 1B 03 1C 03 1C 03 1D 03 1D 03 1D 03 1D 03 1D 03 1D 03 1D 03 1C 03 1C 03 1B 03 1A 03 1A 03 19 03 18 03 17 03 16 03 14 03 13 03 11 03 10 03 0E 03 0C 03 0B 03 09 03 08 03 06 03 03 03 01 02 FF 02 FF 02 FD 02 FC 02 FB 02 F9 02 F9 02 F8 02 F8 02 F8 02 F7 02 F7 02 F7 02 F6 02 F6 02 F5 02 F5 02 F4 02 F4 02 F4 02 F3 02 F3 02 F2 02 F2 02 F2 02 F2 02 F1 02 F1 02 F0 02 F0 02 F0 02 EF 02 EF 02 EF 02 EE 02 EE 02 EE 02 ED 02 ED 02 ED 02 EC 02 EC 02 EC 00 00 00 00 00 00 00 00 00 00 00 00 
EE EE EE EE 0F 0F 00 00 A8 21 67 40 D1 02 C4 40 DB 5F C6 40 AC FC A6 40 CF 20 73 40 F7 18 A3 3F 61 15 09 3F 2D 90 5F 3E 60 60 A5 3E 00 F8 B6 3E 01 69 A2 3E B1 A3 90 3E 51 D5 54 3E 3E 83 96 3E C1 27 4E 3E 92 44 49 3E C7 4B D3 3D 7A 14 AF 3D 4D 9B 99 3D C2 74 8B 3D 21 B2 79 3D 88 C1 75 3D CE C6 7C 3D 14 4E 83 3D 9A 4D 91 3D 28 53 A3 3D CE 29 CA 3D AE F3 EE 3D B6 C2 0A 3E 4F 7E 17 3E 22 19 0C 3E 90 1C E0 3D 69 0C A5 3D CC 37 95 3D B6 9D AF 3D 50 65 CF 3D F6 3B 38 3E CD 53 53 3E 43 42 22 40 29 6B 88 40 B0 11 BB 40 9B 68 93 40 93 D7 5C 40 4B 31 B5 3F 7A EC 68 3F 9D 95 0B 3F 61 47 9D 3F 6C F0 BA 3F 53 F5 8D 3F C9 2B 67 3F C5 CF A3 3F CE 75 B0 3F C6 91 99 3F 65 67 62 3F E5 C3 89 3F F8 45 8E 3F 52 F0 6B 3F BE C9 76 3F C3 51 88 3F 1B 79 87 3F 9B 56 85 3F 00 00 00 00 00 00 00 00 
'''

segments = uartDataProcess(hex_input)
plot_data(segments)
