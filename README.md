# 富士施乐 (Fuji Xerox) DocuCentre S2110 / S2011 macOS 商业驱动 (一机一码版)

本驱动专为 macOS 用户打造，免去购置昂贵的 PostScript 硬件扩展卡，支持网络打印与 USB 直连、600 DPI 矢量级高清解析、原生双面打印与多纸盒控制，并集成**一机一码 + 7 天动态时效 2FA 离线授权验证**。

---

## 适配机型

* **Fuji Xerox DocuCentre S2110**
* **Fuji Xerox DocuCentre S2011**
* **Fuji Xerox DocuCentre S2320**
* **Fuji Xerox DocuCentre S2520**
* 打印语言：HBPL (Host Based Print Language)
* USB 硬件标识：VID 0x0550, PID 0x01BE

---

## 商业安全与一机一码机制

1. **硬件强绑定**：读取 Mac 主板物理序列号（IOKit `IOPlatformUUID`），锁定单台机器；
2. **7 天时效 2FA 离线激活**：买家提供设备识别码，卖家在《离线算号器》生成 7 天内有效激活码（格式：`XXXX-XXXX-XXXX`）；
3. **原生 Mach-O 二进制混淆**：核心授权助手与转译滤镜全二进制编译并彻底剥离符号（`strip -S -x`），内存中动态解码密钥，杜绝文本反编译。

---

## 安装与激活步骤

1. 双击挂载 `FujiXerox_DocuCentre_S2110_Mac_Driver.dmg`；
2. 双击打开 **「富士施乐打印机一键配置向导.app」**；
3. 弹出授权激活窗口，向导已**自动将您的 Mac 硬件代码复制到剪贴板**，粘贴发送给卖家获取激活密钥；
4. 输入激活密钥，验证通过后自动颁发本机永久凭证；
5. 选择连接方式（网络打印输入 IP，USB 插入直连）；
6. 提示配置成功后，即可直接按 `Cmd + P` 正常打印！

---

## 卖家发卡工具

卖家手机或电脑直接离线打开 **`卖家专属离线算号器.html`**，粘贴买家发来的设备识别码，点击“一键生成激活码”即可生成 7 天有效的授权卡密。
