/*
 * Fuji Xerox DocuCentre S2110 macOS GUI Wizard & Activation Helper
 * Compiled as Native Mach-O to protect Secret Key & 2FA Logic from plaintext reading
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <CommonCrypto/CommonHMAC.h>

/* volatile 阻止编译器常量折叠 (Defeat Constant Folding) */
static volatile const unsigned char K_BOX[] = {
    0x34, 0x17, 0x2e, 0x21, 0x34, 0x31, 0x2d, 0xce, 0xcb, 0xcb,
    0xcc, 0x31, 0x1f, 0x1b, 0x1d, 0x21, 0x2d, 0x31, 0x2d, 0x17,
    0x1d, 0x2e, 0x17, 0x28, 0x31, 0x2d, 0x1b, 0x20, 0x28, 0x31,
    0xce, 0xcc, 0xce, 0xca, 0x31, 0x1c, 0xbd, 0xbb
};
#define K_LEN 38

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
        /* 7天动态时效滑动窗口 (当前窗口及前后各 1 个窗口，防时钟轻微漂移) */
        long long step = 7 * 86400;
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
            /* 验证成功，推导该 Mac 的永久授权凭证 */
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
        /* 本地永久证书快速静默校验 */
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
