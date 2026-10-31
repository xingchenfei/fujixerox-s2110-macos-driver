#!/bin/bash
# ==============================================================================
# 富士施乐 (Fuji Xerox) DocuCentre S2110 终端卸载脚本 (macOS)
# ==============================================================================

if [ "$EUID" -ne 0 ]; then
    echo ">> 正在请求管理员权限，请输入当前 Mac 开机密码:"
    sudo "$0" "$@"
    exit 0
fi

echo ">> 正在移除富士施乐打印机与驱动文件..."
lpadmin -x Fuji_Xerox_DocuCentre_S2110 2>/dev/null || true
rm -rf /Library/Printers/FujiXerox
rm -f /Library/Printers/PPDs/Contents/Resources/fujixerox-docucentre-s2110.ppd

echo ">> 富士施乐 DocuCentre S2110 驱动已成功卸载！"
read -p "按回车键退出..."
