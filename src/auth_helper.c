/*
 * Fuji Xerox DocuCentre S2110 macOS GUI Wizard & Activation Helper
 * Unified Universal Algorithm Edition (Compatible with Universal Master Keygen)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <CommonCrypto/CommonHMAC.h>

/* 统一全品牌母私钥 K_BOX (volatile 阻断编译期常量折叠)
 * 对应明文: MACOS_COMMERCIAL_PRINTER_DRIVER_UNIVERSAL_SECRET_2026_@#! (长度 57 字节)
 */
static volatile const unsigned char K_BOX[] = {
    0x1f, 0x1b, 0x1d, 0x21, 0x2d, 0x31, 0x1d, 0x21, 0x1f, 0x1f,
    0x17, 0x2e, 0x1d, 0x23, 0x1b, 0x20, 0x31, 0x2c, 0x2e, 0x23,
    0x22, 0x28, 0x17, 0x2e, 0x31, 0x18, 0x2e, 0x23, 0x2a, 0x17,
    0x2e, 0x31, 0x27, 0x22, 0x23, 0x2a, 0x17, 0x2e, 0x2d, 0x1b,
    0x20, 0x31, 0x2d, 0x17, 0x1d, 0x2e, 0x17, 0x28, 0x31, 0xce,
    0xcc, 0xce, 0xca, 0x31, 0x1c, 0xbd, 0xbb
};
#define K_LEN 57

__attribute__((noinline)) static void restore_secret(unsigned char *out) {
    for (size_t i = 0; i < K_LEN; i++) {
        out[i] = ((K_BOX[i] - 0x37) ^ 0xA5) & 0xFF;
    }
    out[K_LEN] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 4) return 1;

    const char *action = argv[1];
    const char *hw_uuid = argv[2];
    const char *input_val = argv[3];

    unsigned char secret[K_LEN + 1];
    restore_secret(secret);

    if (strcmp(action, "verify") == 0) {
        long long step = 7 * 86400; // 7天滑动时钟窗口
        long long cur_window = time(NULL) / step;

        int passed = 0;
        for (long long w = cur_window - 1; w <= cur_window + 1; w++) {
            char msg[256];
            snprintf(msg, sizeof(msg), "ACTIVATION:%s:%lld", hw_uuid, w);

            unsigned char digest[CC_SHA256_DIGEST_LENGTH];
            CCHmac(kCCHmacAlgSHA256, secret, K_LEN, msg, strlen(msg), digest);

            char hex[16];
            for (int i = 0; i < 6; i++) {
                sprintf(&hex[i * 2], "%02X", digest[i]);
            }

            char expected[32];
            snprintf(expected, sizeof(expected), "%.4s-%.4s-%.4s", hex, hex + 4, hex + 8);

            if (strcasecmp(input_val, expected) == 0) {
                passed = 1;
                break;
            }
        }

        if (passed) {
            char perm_msg[256];
            snprintf(perm_msg, sizeof(perm_msg), "PERM_AUTH:%s", hw_uuid);

            unsigned char perm_digest[CC_SHA256_DIGEST_LENGTH];
            CCHmac(kCCHmacAlgSHA256, secret, K_LEN, perm_msg, strlen(perm_msg), perm_digest);

            char perm_token[64];
            for (int i = 0; i < 12; i++) {
                sprintf(&perm_token[i * 2], "%02X", perm_digest[i]);
            }
            printf("%s\n", perm_token);
            memset(secret, 0, sizeof(secret));
            return 0;
        }

        memset(secret, 0, sizeof(secret));
        return 1;
    } else if (strcmp(action, "check") == 0) {
        char perm_msg[256];
        snprintf(perm_msg, sizeof(perm_msg), "PERM_AUTH:%s", hw_uuid);

        unsigned char perm_digest[CC_SHA256_DIGEST_LENGTH];
        CCHmac(kCCHmacAlgSHA256, secret, K_LEN, perm_msg, strlen(perm_msg), perm_digest);

        char expected_token[64];
        for (int i = 0; i < 12; i++) {
            sprintf(&expected_token[i * 2], "%02X", perm_digest[i]);
        }

        int ok = (strcasecmp(input_val, expected_token) == 0) ? 0 : 1;
        memset(secret, 0, sizeof(secret));
        return ok;
    }

    memset(secret, 0, sizeof(secret));
    return 1;
}
