# 富士施乐 (Fuji Xerox) DocuCentre S2110 国产操作系统专用驱动 (自解压运行包)

本驱动由作者独立自主研发，专为**统信 UOS、银河麒麟（KylinOS）、深度（Deepin）、中科方德、华为欧拉（openEuler）**等国产 Linux 操作系统打造，免去购置昂贵的 PostScript (PS) 硬件扩展卡。

支持局域网网络打印（Socket 9100）与 USB 直连、600 DPI 矢量级高清解析、原生双面翻转装订与多纸盒进纸管理。

---

## 适配国产 CPU 架构全景

本安装包为**单文件全架构融合自适应包（Self-Extracting Universal Runtime）**，内部集成了 4 大主流国产 CPU 平台的纯静态二进制机器码，运行时自动检测芯片指令集：

* **飞腾 (Phytium) / 华为鲲鹏**：`aarch64` / `arm64`（ARM 架构）
* **海光 (Hygon) / 兆芯 (Zhaoxin)**：`x86_64` / `amd64`（x86 架构）
* **龙芯新架构 (Loongson 3A5000 / 3A6000)**：`loongarch64`（龙芯自研架构）
* **龙芯旧架构 (Loongson 3A3000 / 3A4000)**：`mips64el`（MIPS 架构）

---

## 快速安装与配置方法

### 方式一：桌面双击安装（推荐）
1. 在统信 UOS 或银河麒麟桌面上，找到 **`FujiXerox_DocuCentre_S2110_Linux_Universal_Installer.run`**；
2. 右键属性确认已勾选“允许作为程序执行”（或打开终端执行 `chmod +x *.run`）；
3. **直接双击运行**：
   * 自动弹出原生 GTK 图形向导窗口；
   * 首次运行时，向导已**自动将本机的硬件识别代码复制到剪贴板**，将代码发送给客服获取专属激活密钥并输入；
   * 选择连接方式（网络打印输入复印机局域网 IP，USB 插线直连）；
   * 弹出系统提权提示时输入当前用户开机密码授权；
   * 提示“安装成功”后，打开 WPS 或任意办公软件直接按 `Ctrl + P` 即可打印。

### 方式二：终端命令行一键安装
在无桌面环境或需要自动化批量部署的终端下：
```bash
sudo ./FujiXerox_DocuCentre_S2110_Linux_Universal_Installer.run
```
向导将自动降级为全中文交互式命令行，根据提示按回车即可完成配置。

---

## 卸载方法

如需彻底移除该打印机与系统驱动组件，只需打开终端执行：
```bash
sudo lpadmin -x Fuji_Xerox_DocuCentre_S2110
sudo rm -rf /etc/cups/fujixerox /usr/share/cups/model/fujixerox /usr/lib/cups/filter/rastertohbpl
```
