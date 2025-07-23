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
FF FF FF FF 0F 0F 02 70 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 02 71 00 00 00 00 00 00 00 00 00 00 00 00 00 00 EE EE EE EE 0F 0F 00 00 B5 93 32 40 46 08 B4 3F E4 52 8F 3F 77 94 84 3F C4 BA 84 3F B9 57 8A 3F 49 56 8D 3F 57 13 8B 3F E6 56 84 3F F4 64 8E 3F 67 D5 8D 3F 71 BA 8B 3F D8 03 8B 3F 32 E7 83 3F ED B8 8A 3F 76 4E 88 3F 14 8F 86 3F F3 1B 86 3F 99 97 8A 3F 4F 25 88 3F 39 2D 85 3F 49 86 86 3F D3 D2 46 3F B4 00 B2 3F A5 E7 97 3F 26 76 3F 40 AB 2F FC 3F DA 95 01 40 D2 EA DC 3F 83 FC B8 3F 9C 4D 92 40 46 61 3B 40 D1 9E C4 3F 9F 17 26 3F 05 CA 67 3F 64 8C 52 40 DF 77 1D 3F 02 FB 9E 3F 5C 58 AA 3F 45 4A B6 40 78 15 8F 40 7E 36 5C 40 E8 A5 8F 40 64 28 90 40 82 E3 CE 40 AE 07 C6 3F 11 33 40 3F 58 E2 82 3F 12 D4 4F 3F 85 8E 89 3F C7 77 89 3F F6 A9 89 3F D3 A3 89 3F 02 B7 89 3F 96 87 89 3F 29 84 89 3F 62 D9 89 3F 23 87 89 3F 71 84 89 3F 0B C3 89 3F 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF FF 0F 0F 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 77 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 77 02 77 02 76 02 76 02 76 02 76 02 77 02 76 02 76 02 76 02 76 02 76 02 77 02 77 02 77 02 76 02 77 02 77 02 77 02 77 02 77 00 00 00 00 00 00 00 00 00 00 00 00 00 00 EE EE EE EE 0F 0F 00 00 82 CE 1B 40 EB 76 9A 3F EF 3E 8F 3F DB 59 85 3F EA 4A 8C 3F BE 0B 91 3F 8B 59 86 3F AE E0 90 3F 94 E8 05 40 B4 02 47 3F 0E AE 8B 3F C8 F5 AA 3F 82 06 68 3F D6 09 99 3F E7 27 6A 3F 09 E0 09 40 5B C1 89 3F 90 10 81 3F 03 BB F7 3F 67 A6 A7 3F A9 5E B2 3F 19 0B 81 3F CB 8D A7 3F 57 16 C7 3F C8 DB 54 3F FB 1F 91 3F 72 33 4C 40 F4 5E 86 3F 5A 0C 79 3F 0F DC 59 40 FA D2 B1 3F 42 C0 57 3E 6D E1 2D 3F 35 CA CC 3F 5E B0 35 40 49 C1 5C 3F 0B F1 C3 3F A2 50 84 40 8C C6 64 3F 2D 02 35 3F 4C C9 B6 3F B0 11 EB 3F C7 4D B5 3F 80 71 4B 3F 57 07 7E 3F 1F 23 53 40 E6 8A 8D 3F D4 70 9F 3F 0C 21 8E 3F 02 3A 2D 3F 31 54 AF 3F E6 5E A6 3F 1D D7 62 3F 3F FF 34 40 98 97 B2 3F EB 2D 17 3F 4D 5E A7 3F CE 12 B0 3F F5 A6 DA 3F 0C 1A 3D 3F 6D 01 7A 3F 00 00 00 00 00 00 00 00 FF FF FF FF 0F 0F 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 02 76 00 00 00 00 00 00 00 00 00 00 00 00 EE EE EE EE 0F 0F 00 00 39 2F 61 40 68 6A 7F 3F CA 59 83 3F DE 6E 8B 3F 8B 2E 8C 3F B7 94 80 3F 99 44 88 3F 1F 4A 94 3F 74 0B 92 3F 1A 04 8D 3F 9D 59 7C 3F 5A A2 A4 3F FF 26 89 3F 6E 02 8D 3F AD B6 8B 3F 5B AD 2A 40 68 D7 94 3F 42 D7 8F 3F A4 0D 82 3F 66 2F 0D 40 30 2B 76 3F 5B 21 85 3F 4D 8E 48 3F 19 98 29 3F 96 A8 8A 3F F9 6F 7F 3F 62 3B 2D 40 BC F3 DC 3F 2B DF 90 3F 77 5D 75 3F 49 59 43 3F 13 5C 66 3F 97 19 84 3F 26 FB 8D 3F 41 D4 85 3F B7 79 80 3F 91 14 46 40 49 F0 BE 3F A8 C0 8F 3F 0A 97 6C 3F EE 15 39 3F 7E 17 95 3F 49 C5 65 3F 72 DB BC 3F B1 4E 7C 3F 79 C4 6E 3F 0D 93 65 40 9A ED D1 3F 96 22 85 3F B8 55 A1 3F E6 20 9D 3F 02 A4 8F 3F 06 0F 51 3F B0 E6 82 3F 3D DE 80 3F B3 D4 5E 3F 1E B6 43 3F 8D 08 65 3F 04 0D 7C 3F A3 31 83 3F 15 0F CE 3F 00 00 00 00 00 00 00 00 FF FF FF FF 0F 0F 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 02 77 00 00 00 00 00 00 00 00 00 00 00 00 EE EE EE EE 0F 0F 00 00 78 94 3F 40 F1 6D 92 3F BF 8A 8A 3F C2 2C 8F 3F C2 B0 89 3F 0E 53 8F 3F 87 F2 89 3F 1D CE 89 3F F7 08 8A 3F F5 D1 89 3F 73 B0 88 3F FA 4A 89 3F 39 AE CD 3F EC 73 A6 3F 58 42 A2 3F C8 3B 8C 3F 08 F4 8E 3F AB 5B 57 40 89 D4 B3 3F 5B B2 8B 3F 95 DF 8D 3F 1A FC 8E 3F AF BB 89 3F F2 66 7E 40 9D 4D C8 3F 90 08 97 3F BA 64 99 3F 42 9B 8F 3F F1 8F 89 3F 35 0F D8 3F 28 60 4B 3F 2A 46 8D 3F 22 FD 96 3F E1 B2 89 3F E4 85 89 3F F3 7C 89 3F 21 55 BA 3E B7 86 7A 3F 8A 57 8F 3F 72 96 8D 3F 21 97 89 3F 8C 6D 97 3F 41 09 80 3F C5 37 9A 3F 04 12 95 3F C0 74 89 3F 1F 56 65 40 EB FF CC 3F 75 B6 96 3F 7D A8 86 3F 86 34 8A 3F A2 D8 12 40 BC 3E FB 3F 86 9E A4 3F B7 3D 90 3F F3 80 8B 3F E0 4B 8A 3F 4F C0 E9 3E AA A0 AA 3F F6 BD 8B 3F EF F1 89 3F 00 00 00 00 00 00 00 00 
'''  # 替换为你的十六进制串

segments = uartDataProcess(hex_input)
plot_data(segments)
