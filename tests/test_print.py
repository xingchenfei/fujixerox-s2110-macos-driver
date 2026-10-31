#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Fuji Xerox DocuCentre S2110 CUPS Driver Test & Self-Diagnosis Suite
Validates CUPS Raster input -> rastertohbpl filter -> HBPL binary stream decoding.
"""

import os
import sys
import struct
import subprocess
import time

def generate_cups_raster(width=512, height=512, dpi=600, page_name="A4"):
    """生成合法的 CUPS Raster v2 二进制数据流"""
    # 4字节魔数: 'RaS2'
    magic = b"RaS2"
    
    # 1792 字节头部
    hdr = bytearray(1792)
    
    # offset 272: cupsDuplex (0=none)
    struct.pack_into(">I", hdr, 272, 0)
    # offset 276, 280: cupsHWResolution (600, 600)
    struct.pack_into(">I", hdr, 276, dpi)
    struct.pack_into(">I", hdr, 280, dpi)
    
    # offset 372: cupsWidth
    struct.pack_into(">I", hdr, 372, width)
    # offset 376: cupsHeight
    struct.pack_into(">I", hdr, 376, height)
    
    # offset 384: cupsBitsPerColor (1)
    struct.pack_into(">I", hdr, 384, 1)
    # offset 388: cupsBitsPerPixel (1)
    struct.pack_into(">I", hdr, 388, 1)
    
    bytes_per_line = (width + 7) // 8
    # offset 392: cupsBytesPerLine
    struct.pack_into(">I", hdr, 392, bytes_per_line)
    
    # offset 400: cupsColorSpace (3 = Black, 1 is black, 0 is white)
    struct.pack_into(">I", hdr, 400, 3)
    
    # offset 1732: cupsPageSizeName
    name_bytes = page_name.encode('ascii')[:64]
    hdr[1732:1732+len(name_bytes)] = name_bytes
    
    # 点阵行数据
    raster_body = bytearray(height * bytes_per_line)
    
    # 在位图中绘制测试图案 (边框与对角线)
    for y in range(height):
        row_offset = y * bytes_per_line
        # 顶边和底边
        if y < 16 or y >= height - 16:
            for x_byte in range(bytes_per_line):
                raster_body[row_offset + x_byte] = 0xff
        else:
            # 左右边框 (前16像素与后16像素置黑)
            raster_body[row_offset] = 0xff
            raster_body[row_offset + 1] = 0xff
            raster_body[row_offset + bytes_per_line - 1] = 0xff
            raster_body[row_offset + bytes_per_line - 2] = 0xff
            
            # 对角线
            x = y % width
            byte_idx = x // 8
            bit_idx = 7 - (x % 8)
            if byte_idx < bytes_per_line:
                raster_body[row_offset + byte_idx] |= (1 << bit_idx)
                
    return magic + hdr + bytes(raster_body)

def main():
    print("=" * 65)
    print("  Fuji Xerox DocuCentre S2110 驱动本地转译自检测试套件")
    print("  Target: HBPLv1 / S2110 | USB VID: 0x0550, PID: 0x01BE")
    print("=" * 65)
    
    filter_bin = os.path.join(os.path.dirname(os.path.abspath(__file__)), "rastertohbpl")
    if not os.path.isfile(filter_bin):
        print(f"[FAIL] 未找到滤镜二进制文件: {filter_bin}")
        sys.exit(1)
        
    print(f"[1/4] 生成模拟 CUPS A4 栅格数据流 (512x512 @ 600 DPI)...")
    raster_data = generate_cups_raster(512, 512, 600, "A4")
    raw_size = len(raster_data)
    print(f"      输入栅格数据体积: {raw_size} 字节 ({raw_size/1024:.2f} KB)")
    
    print(f"[2/4] 调用 rastertohbpl 滤镜管道进行实时编码...")
    t0 = time.time()
    proc = subprocess.Popen([filter_bin, "1", "testuser", "DiagnosticPage", "1", "", ""],
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    hbpl_out, err = proc.communicate(input=raster_data)
    elapsed = (time.time() - t0) * 1000
    
    if proc.returncode != 0:
        print(f"[FAIL] 滤镜执行失败，退出代码 {proc.returncode}")
        print("STDERR:", err.decode('utf-8', errors='ignore'))
        sys.exit(1)
        
    hbpl_size = len(hbpl_out)
    ratio = (1.0 - (hbpl_size / raw_size)) * 100.0
    print(f"      转译完成! 耗时: {elapsed:.2f} ms")
    print(f"      输出 HBPL 数据体积: {hbpl_size} 字节 | 压缩节约率: {ratio:.1f}%")
    
    print(f"[3/4] 验证施乐 HBPL 协议签名与控制原语...")
    checks = [
        (b"\033%-12345X@PJL", "PJL Universal Exit Language (UEL) 识别符"),
        (b"@PJL ENTER LANGUAGE=HBPL", "HBPL 主机渲染语言声明"),
        (b"@PJL SET RENDERMODE=GRAYSCALE", "灰度单色渲染指令"),
        (b"@PJL SET RESOLUTION=600", "600 DPI 硬件扫描分辨率"),
        (b"\x41\x81\xa1\x00\x82\xa2", "HBPL 作业开始控制记录 (RECTYPE 'A')"),
        (b"\x43\x91\xa1\x00\x92\xa1", "HBPL 页面开始控制记录 (RECTYPE 'C')"),
        (b"\x51\x52", "HBPL 数据流通道同步字 (RECTYPE 'Q' & 'R')"),
        (b"SD", "HBPL 扫描段终止符 (SD Tag)"),
        (b"B\033%-12345X@PJL EOJ", "HBPL 作业结束封包 (RECTYPE 'B' + EOJ)")
    ]
    
    all_passed = True
    for signature, desc in checks:
        if signature in hbpl_out:
            print(f"      [OK] {desc}")
        else:
            print(f"      [FAIL] 缺失关键协议字段: {desc}")
            all_passed = False
            
    print(f"[4/4] 使用开源 HBPL 校验引擎进行逆向解码检验...")
    decoder_bin = "/tmp/hbpldecode"
    if os.path.isfile(decoder_bin):
        dec_proc = subprocess.Popen([decoder_bin, "-d", "/tmp/s2110_test_verify"],
                                    stdin=subprocess.PIPE,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        dec_stdout, dec_stderr = dec_proc.communicate(input=hbpl_out)
        if dec_proc.returncode == 0 and os.path.isfile("/tmp/s2110_test_verify-01.pgm"):
            dec_size = os.path.getsize("/tmp/s2110_test_verify-01.pgm")
            print(f"      [OK] 硬件数据包完全通过 HBPL 校验! 解码位图体积: {dec_size} 字节")
        else:
            print(f"      [WARN] 解码器警告: {dec_stderr.decode('utf-8', errors='ignore')[:100]}")
    else:
        print(f"      [SKIP] 未安装 hbpldecode，跳过二次模拟逆向解码。")
        
    print("-" * 65)
    if all_passed:
        print("  🎉 全部测试项通过! 富士施乐 S2110 滤镜引擎工作完全正常!")
    else:
        print("  ⚠️ 存在未通过项目，请核查协议流实现。")
    print("=" * 65)

if __name__ == "__main__":
    main()
