#!/bin/bash
# ==============================================================================
# 富士施乐 (Fuji Xerox) DocuCentre S2110 终端一键安装脚本 (macOS)
# ==============================================================================

set -e

DIR="$(cd "$(dirname "$0")" && pwd)"
PPD_SRC="${DIR}/富士施乐打印机一键配置向导.app/Contents/SharedSupport/ppd/fujixerox-docucentre-s2110.ppd"
FILTER_SRC="${DIR}/富士施乐打印机一键配置向导.app/Contents/SharedSupport/filters/rastertohbpl"

echo "=================================================================="
echo "    富士施乐 (Fuji Xerox) DocuCentre S2110 macOS 一键安装程序"
echo "=================================================================="
echo ""

# 请求 root 提权
if [ "$EUID" -ne 0 ]; then
    echo ">> 正在请求管理员权限，请输入当前 Mac 开机密码 (输入时不显示字符):"
    sudo "$0" "$@"
    exit 0
fi

echo ">> [1/4] 部署系统级打印过滤器与设备描述 PPD..."
mkdir -p /Library/Printers/FujiXerox/Filters
mkdir -p /Library/Printers/FujiXerox/Custom
mkdir -p /Library/Printers/PPDs/Contents/Resources

cp -f "$FILTER_SRC" /Library/Printers/FujiXerox/Filters/rastertohbpl
chmod 755 /Library/Printers/FujiXerox/Filters/rastertohbpl
chown root:wheel /Library/Printers/FujiXerox/Filters/rastertohbpl

cp -f "$PPD_SRC" /Library/Printers/PPDs/Contents/Resources/fujixerox-docucentre-s2110.ppd
chmod 644 /Library/Printers/PPDs/Contents/Resources/fujixerox-docucentre-s2110.ppd
chown root:wheel /Library/Printers/PPDs/Contents/Resources/fujixerox-docucentre-s2110.ppd

echo ">> [2/4] 配置打印机连接方式:"
echo "   1) 网络连接 (TCP/IP Socket 9100) - 推荐"
echo "   2) USB 直连"
read -p "请选择连接类型 [默认为 1]: " CONN_OPT
CONN_OPT=${CONN_OPT:-1}

PRINTER_URI=""
if [ "$CONN_OPT" = "2" ]; then
    DETECTED_USB=$(lpinfo -v 2>/dev/null | grep -i -E "direct usb://|0550|s2110|docucentre" | head -n 1 | awk '{print $2}')
    if [ -n "$DETECTED_USB" ]; then
        PRINTER_URI="$DETECTED_USB"
    else
        PRINTER_URI="usb://Fuji%20Xerox/DocuCentre%20S2110?serial=S2110"
    fi
    echo ">> 使用 USB 连接: $PRINTER_URI"
else
    while true; do
        read -p "请输入打印机局域网 IPv4 地址 (例如 192.168.1.200): " PRINTER_IP
        PRINTER_IP=$(echo "$PRINTER_IP" | tr -d '[:space:]')
        if echo "$PRINTER_IP" | grep -Eq '^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$'; then
            PRINTER_URI="socket://${PRINTER_IP}:9100"
            break
        else
            echo "   [错误] 输入的 IP 格式不正确，请重新输入！"
        fi
    done
fi

echo ">> [3/4] 注册打印机队列到 macOS CUPS 打印系统..."
QUEUE_NAME="Fuji_Xerox_DocuCentre_S2110"
PPD_TARGET="/Library/Printers/PPDs/Contents/Resources/fujixerox-docucentre-s2110.ppd"

lpadmin -x "$QUEUE_NAME" 2>/dev/null || true
lpadmin -p "$QUEUE_NAME" \
        -E \
        -v "$PRINTER_URI" \
        -P "$PPD_TARGET" \
        -D "Fuji Xerox DocuCentre S2110" \
        -L "办公室" \
        -o printer-is-shared=false

cupsenable "$QUEUE_NAME" 2>/dev/null || true
cupsaccept "$QUEUE_NAME" 2>/dev/null || true

echo ">> [4/4] 驱动安装与配置完成！"
echo ""
echo "=================================================================="
echo "  恭喜！打印机已配置成功。"
echo "  打印机名称: $QUEUE_NAME"
echo "  设备地址:   $PRINTER_URI"
echo "  支持特性:   600 DPI 矢量级高清解析、双面打印、多纸盒"
echo "=================================================================="
read -p "按回车键退出..."
